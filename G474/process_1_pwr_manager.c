/* 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Project : A_os
*/
/*
 * process_1_pwr_manager.c
 *
 *  Created on: Jan 10, 2025
 *      Author: fil
 */

#include "main.h"
#include "A_os_includes.h"
#include "aventadorII.h"
#include "stm32g4xx_hal.h"
#ifndef	SAMPLE_PROCESSES_ENABLED

extern	uint32_t WriteFlashDoubleWord(uint32_t address, uint64_t data);
#define	AUTOBOOT_ON_FLASH	0x5aa512347ee75678

uint8_t		led_cntr=0;

uint8_t 	forced_power = 0 , forced_unpower = 0;
uint8_t		forced_power_counter = 0;

uint64_t	*autoboot_var_ptr;
uint64_t	autoboot_var;

AventadorIISystem_TypeDef	AventadorIISystem;
AventadorIIUsb_TypeDef		AventadorIIUsb;

uint8_t	uart_rx_buffer[UART_RX_BUF_SIZE];
uint8_t	uart_tx_buffer[UART_TX_BUF_SIZE];

UART_Drv_TypeDef Uart_Drv =
{
	.data = uart_rx_buffer,
	.rx_max_len = UART_RX_BUF_SIZE,
	.uart = &huart1,
	.wakeup_id = WAKEUP_FROM_UART1_IRQ,
	.timeout = 10,
	//.flags = UART_USES_DMA_TX | UART_USES_DMA_RX | UART_WAKEUP_ON_RXFULL | UART_WAKEUP_ON_TIMEOUT,
	.flags = UART_WAKEUP_ON_RXFULL | UART_WAKEUP_ON_TIMEOUT,
};

uint32_t	uart_driver_handle;

ADC_Drv_TypeDef	ADC1_Drv =
{
		.adc = &hadc1,
		.adc_buffer = &AventadorIISystem.val_3v3,
		.num_channels = 3,
		.adc_timer = &htim6,
		.flags = ADC_FLAGS_FULL_WAKEUP | ADC_FLAGS_CALIBRATE,
		.wakeup_id = WAKEUP_FROM_ADC1_IRQ,
};
uint32_t		adc1_driver_handle;
ADC_Drv_TypeDef	ADC2_Drv =
{
		.adc = &hadc2,
		.adc_buffer = &AventadorIISystem.val_1v8,
		.num_channels = 3,
		.adc_timer = &htim6,
		.flags = ADC_FLAGS_FULL_WAKEUP | ADC_FLAGS_CALIBRATE,
		.wakeup_id = WAKEUP_FROM_ADC2_IRQ,
};
uint32_t		adc2_driver_handle;

USB_Drv_TypeDef	Usb_channel =
{
		.requested_len = USB_BUF_LEN,
		.data = AventadorIIUsb.usb_rx_buf_rxed,
		.timeout = 10,
		.wakeup_id = WAKEUP_FROM_USB_DEVICE_IRQ,
};
uint32_t	usb_handle;
uint8_t		usbmessage_tx[128];

#define	UNIT			(0.000816F)
#define	MFACTOR_VIN		5.6F
#define	MFACTOR_3V3		2.0F
#define	MFACTOR_5V		2.1F
#define	MFACTOR_DIRECT	1.2F

void calc_ad_values(void)
{
	sprintf((char *)AventadorIISystem.V3v3,"%1.2f",(AventadorIISystem.val_3v3+ADC1_Drv.calibration)*MFACTOR_3V3*UNIT);
	sprintf((char *)AventadorIISystem.vin ,"%1.2f",(AventadorIISystem.val_vin+ADC1_Drv.calibration)*MFACTOR_VIN*UNIT);
	sprintf((char *)AventadorIISystem.V5v ,"%1.2f",(AventadorIISystem.val_5v+ADC1_Drv.calibration)*MFACTOR_5V*UNIT);
	sprintf((char *)AventadorIISystem.V1v8 ,"%1.2f",(AventadorIISystem.val_1v8+ADC2_Drv.calibration)*MFACTOR_DIRECT*UNIT);
	sprintf((char *)AventadorIISystem.V1v2 ,"%1.2f",(AventadorIISystem.val_1v2+ADC2_Drv.calibration)*MFACTOR_DIRECT*UNIT);
	sprintf((char *)AventadorIISystem.V0v8 ,"%1.2f",(AventadorIISystem.val_0v8+ADC2_Drv.calibration)*MFACTOR_DIRECT*UNIT);
	sprintf((char *)usbmessage_tx,"Vin : %s\r\n5V0 : %s\r\n3V3 : %s\r\n1V8 : %s\r\n1V2 : %s\r\n0V8 : %s\r\n",
			(char *)AventadorIISystem.vin,
			(char *)AventadorIISystem.V5v,
			(char *)AventadorIISystem.V3v3,
			(char *)AventadorIISystem.V1v8,
			(char *)AventadorIISystem.V1v2,
			(char *)AventadorIISystem.V0v8
			);
	usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
}

void do_led(void)
{
	if (( AventadorIISystem.status & SYS_STAT_BOARD_POWER)  ==  SYS_STAT_BOARD_POWER )
	{
		//HAL_GPIO_WritePin(GREEN_LED__GPIO_Port, GREEN_LED__Pin,GPIO_PIN_RESET);
	}
	else
	{
		switch(led_cntr)
		{
		case 70 :
		case 90 :
			HAL_GPIO_WritePin(GREEN_LED__GPIO_Port, GREEN_LED__Pin,GPIO_PIN_RESET);
			break;
		default :
			HAL_GPIO_WritePin(GREEN_LED__GPIO_Port, GREEN_LED__Pin,GPIO_PIN_SET);
			break;
		}
	}
	led_cntr++;
	if ( led_cntr == 100 )
		led_cntr = 0;
}

void compile_banner_and_send(void)
{
	if ( autoboot_var == AUTOBOOT_ON_FLASH )
	{
#ifdef AUTO_POWER_ON
		sprintf((char *)usbmessage_tx,"%s %s Aos %s\r\nAUTOBOOT ON ( forced )\r\n",aventadorII_name,aventadorII_version,A_OS_VERSION);
#else
		sprintf((char *)usbmessage_tx,"%s %s Aos %s\r\nAUTOBOOT ON (J16 open)\r\n",aventadorII_name,aventadorII_version,A_OS_VERSION);
#endif
	}
	else
		sprintf((char *)usbmessage_tx,"%s %s Aos %s\r\nAUTOBOOT OFF (disabled by J16 closed)\r\n",aventadorII_name,aventadorII_version,A_OS_VERSION);
	usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
}

void banner_out(void)
{
	AventadorIISystem.logo_timer ++;
	if ( AventadorIISystem.logo_timer == 200)
		compile_banner_and_send();
	if ( AventadorIISystem.logo_timer >= 201)
		AventadorIISystem.logo_timer = 201;
}

/*
 * on sequence
 * 1 : set 1 on ON_5V_MODULE_Pin
 * 2 : wait one tick
 * 3 : set 0 on POWER_EN__Pin, inverted to module
 * 4 : wait for SYS_RESET_IN_Pin go to high
 * 5 : set 1 on CARRIER_PWR_ENABLE_Pin
 *
 * off sequence
 * 1 : set 1 on POWER_EN__Pin, inverted to module
 * 2 : wait for SYS_RESET_IN_Pin go to low
 * 3 : set 0 on CARRIER_PWR_ENABLE_Pin
 * 4 : wait one tick
 * 5 : set 0 on ON_5V_MODULE_Pin
 *
 */


//#define	POWER_SEQ_DEBUG	1
void power_sm(void)
{
	switch(AventadorIISystem.power_state_machine)
	{
	case START :
		HAL_GPIO_WritePin(ON_5V_MODULE_GPIO_Port, ON_5V_MODULE_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(POWER_EN__GPIO_Port, POWER_EN__Pin,GPIO_PIN_SET);	// power en to module
		AventadorIISystem.power_state_machine = UNPOWERED;
		AventadorIISystem.board_wake_cntr = BOARD_WAKEBTN_TIME/PRC1_TICK;
		break;
	case UNPOWERED :
		if (( HAL_GPIO_ReadPin(BRD_WAKE__GPIO_Port,BRD_WAKE__Pin) == 0 ) || (forced_power == 1 ))
		{
			AventadorIISystem.board_wake_cntr--;
			if ( AventadorIISystem.board_wake_cntr == 0 )
			{
				forced_power = 0;
				if ( AventadorIISystem.val_vin > VL_VIN_MIN )
				{
					AventadorIISystem.status |= SYS_STAT_BOARD_POWER;
					HAL_GPIO_WritePin(ON_5V_MODULE_GPIO_Port, ON_5V_MODULE_Pin,GPIO_PIN_SET);
					AventadorIISystem.power_state_machine = MODULE_POWERED;
					AventadorIISystem.power_state_machine_timeout = POWER_TO_EN_TIME;
				}
				else
				{
					sprintf((char *)usbmessage_tx,"VIn not present\r\n");
					usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
					AventadorIISystem.board_wake_cntr = BOARD_WAKEBTN_TIME/PRC1_TICK;
					AventadorIISystem.power_state_machine = NO_VIN;
				}
			}
		}
		break;
	case NO_VIN :
		if ( HAL_GPIO_ReadPin(BRD_WAKE__GPIO_Port,BRD_WAKE__Pin) == 1 )
		{
			AventadorIISystem.board_wake_cntr--;
			if ( AventadorIISystem.board_wake_cntr == 0 )
			{
				AventadorIISystem.power_state_machine = UNPOWERED;
				AventadorIISystem.board_wake_cntr = BOARD_WAKEBTN_TIME/PRC1_TICK;
			}
		}
		else
			AventadorIISystem.board_wake_cntr = BOARD_WAKEBTN_TIME/PRC1_TICK;
		break;
	case MODULE_POWERED :
		AventadorIISystem.power_state_machine_timeout--;
		if ( AventadorIISystem.power_state_machine_timeout == 0 )
		{
			HAL_GPIO_WritePin(POWER_EN__GPIO_Port, POWER_EN__Pin,GPIO_PIN_RESET);	// power en high to module
			AventadorIISystem.power_state_machine = WAIT_MODULE_SHUTDOWN_HIGH;
			AventadorIISystem.power_state_machine_timeout = WAIT_SHUTDOWNHIGH_TIMEOUT;
		}
		break;
	case WAIT_MODULE_SHUTDOWN_HIGH :
#ifdef POWER_SEQ_DEBUG
		HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_SET);
		AventadorIISystem.power_state_machine = WAIT_MODULE_RESET_HIGH;
		calc_ad_values();
#else
		if ( HAL_GPIO_ReadPin(SHUTDOWN_REQUEST__GPIO_Port,SHUTDOWN_REQUEST__Pin) == 1 )
		{
			HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_SET);
			AventadorIISystem.power_state_machine = WAIT_MODULE_RESET_HIGH;
			AventadorIISystem.power_state_machine_timeout = WAIT_RESETHIGH_TIMEOUT;
			AventadorIISystem.status |= SYS_STAT_CARRIER_POWER;
			calc_ad_values();
		}
		else
		{
			AventadorIISystem.power_state_machine_timeout--;
			if ( AventadorIISystem.power_state_machine_timeout == 0 )
			{
				HAL_GPIO_WritePin(RED_LED__GPIO_Port, RED_LED__Pin,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GREEN_LED__GPIO_Port, GREEN_LED__Pin,GPIO_PIN_SET);
				HAL_GPIO_WritePin(POWER_EN__GPIO_Port, POWER_EN__Pin,GPIO_PIN_SET);
				HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_RESET);
				AventadorIISystem.power_state_machine = POWER_OFF_MODULE;
				AventadorIISystem.power_state_machine_timeout = POWER_OFF_TIMEOUT;
				sprintf((char *)usbmessage_tx,"Power OFF due to module error\r\n");
				usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
			}
		}
#endif
		break;
	case WAIT_MODULE_RESET_HIGH :
#ifdef POWER_SEQ_DEBUG
		HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_SET);
		AventadorIISystem.power_state_machine = CARRIER_POWERED;
#else
		if ( HAL_GPIO_ReadPin(SYS_RESET_IN_GPIO_Port,SYS_RESET_IN_Pin) == 1 )
		{
			HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_SET);
			AventadorIISystem.power_state_machine = CARRIER_POWERED;
			AventadorIISystem.status |= SYS_STAT_CARRIER_POWER;
			calc_ad_values();
		}
		else
		{
			AventadorIISystem.power_state_machine_timeout--;
			if ( AventadorIISystem.power_state_machine_timeout == 0 )
			{
				HAL_GPIO_WritePin(RED_LED__GPIO_Port, RED_LED__Pin,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(POWER_EN__GPIO_Port, POWER_EN__Pin,GPIO_PIN_SET);
				HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_RESET);
				AventadorIISystem.power_state_machine = POWER_OFF_MODULE;
				AventadorIISystem.power_state_machine_timeout = POWER_OFF_TIMEOUT;
				sprintf((char *)usbmessage_tx,"Power OFF due to module error\r\n");
				usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
			}
		}
#endif
		break;
	case CARRIER_POWERED :
		AventadorIISystem.power_state_machine_timeout = WAIT_SHUTDOWNLOW_TIMEOUT;
		if ( HAL_GPIO_ReadPin(BRD_WAKE__GPIO_Port,BRD_WAKE__Pin) == 0 )
		{
			AventadorIISystem.board_wake_cntr--;
			if ( AventadorIISystem.board_wake_cntr == 0 )
			{
				HAL_GPIO_WritePin(POWER_EN__GPIO_Port, POWER_EN__Pin,GPIO_PIN_SET);	// power en low to module
				HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(ON_5V_MODULE_GPIO_Port, ON_5V_MODULE_Pin,GPIO_PIN_RESET);
				AventadorIISystem.board_wake_cntr = BOARD_WAKEBTN_DEBOUNCE/PRC1_TICK;
				AventadorIISystem.power_state_machine = DEBOUNCE_BTN_POWEROFF;
				AventadorIISystem.status &= ~SYS_STAT_BOARD_POWER;
			}
		}
#ifdef POWER_SEQ_DEBUG
#else
		if ( HAL_GPIO_ReadPin(SHUTDOWN_REQUEST__GPIO_Port,SHUTDOWN_REQUEST__Pin) == 0 )
		{
			HAL_GPIO_WritePin(POWER_EN__GPIO_Port, POWER_EN__Pin,GPIO_PIN_SET);	// power en low to module
			AventadorIISystem.power_state_machine = UNPOWERED_CARRIER;
			AventadorIISystem.status &= ~SYS_STAT_BOARD_POWER;
		}
		if ( forced_unpower == 1 )
		{
			forced_unpower = 0;
			HAL_GPIO_WritePin(POWER_EN__GPIO_Port, POWER_EN__Pin,GPIO_PIN_SET);	// power en low to module
			AventadorIISystem.power_state_machine = UNPOWERED_CARRIER;
			AventadorIISystem.status &= ~SYS_STAT_BOARD_POWER;
		}
#endif
		break;
	case DEBOUNCE_BTN_POWEROFF :
		if ( HAL_GPIO_ReadPin(BRD_WAKE__GPIO_Port,BRD_WAKE__Pin) == 1 )
		{
			AventadorIISystem.board_wake_cntr--;
			if ( AventadorIISystem.board_wake_cntr == 0 )
			{
				HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(ON_5V_MODULE_GPIO_Port, ON_5V_MODULE_Pin,GPIO_PIN_RESET);
				AventadorIISystem.power_state_machine = UNPOWERED_CARRIER;
			}
		}
		else
			AventadorIISystem.board_wake_cntr = BOARD_WAKEBTN_DEBOUNCE/PRC1_TICK;
		break;
	case UNPOWERED_CARRIER :
		if ( AventadorIISystem.power_state_machine_timeout )
		{
			AventadorIISystem.power_state_machine_timeout--;
			if ( AventadorIISystem.power_state_machine_timeout == 0 )
			{
				HAL_GPIO_WritePin(POWER_EN__GPIO_Port, POWER_EN__Pin,GPIO_PIN_SET);
				HAL_GPIO_WritePin(CARRIER_PWR_ENABLE_GPIO_Port, CARRIER_PWR_ENABLE_Pin,GPIO_PIN_RESET);
				AventadorIISystem.power_state_machine = POWER_OFF_MODULE;
				AventadorIISystem.power_state_machine_timeout = POWER_OFF_TIMEOUT;
			}
		}
		break;
	case POWER_OFF_MODULE :
		HAL_GPIO_WritePin(ON_5V_MODULE_GPIO_Port, ON_5V_MODULE_Pin,GPIO_PIN_RESET);
		if ( AventadorIISystem.power_state_machine_timeout )
		{
			AventadorIISystem.power_state_machine_timeout--;
			if ( AventadorIISystem.power_state_machine_timeout == 0 )
			{
				HAL_GPIO_WritePin(RED_LED__GPIO_Port, RED_LED__Pin,GPIO_PIN_SET);
				AventadorIISystem.power_state_machine = UNPOWERED;
				AventadorIISystem.status &= ~SYS_STAT_BOARD_POWER;
				AventadorIISystem.status &= ~SYS_STAT_CARRIER_POWER;
				AventadorIISystem.btn_status = 0;
				AventadorIISystem.board_wake_cntr = BOARD_WAKEBTN_TIME/PRC1_TICK;
			}
		}
		break;
	}
}

uint32_t	adc1_ops = 0 , adc2_ops = 0 , usb_ops = 0 , rt = 0;
uint8_t		usb_rx_buf_cmd[12],usb_rx_buf_strparam[12],usb_rx_buf_str_valueparam[12];
#define FLASH_USER_START_ADDR   0x0801FC00U   // Example: last page of 128KB flash (page 127)

/*
"__! POWER ON"
"__! POWER OFF"
"__! SET AUTOBOOT ON"
"__! SET AUTOBOOT OFF"
"__! GET AUTOBOOT"
"__! GET POWERS"
*/
uint32_t monitor_cmds_parser(void)
{
uint32_t	usb_len;
	if (AventadorIIUsb.usb_rx_buf_rxed[0] != '_' )
		return 0;

	if ((usb_len = usb_get_rx_len(usb_handle) ) < 8 )
		return 0;

	AventadorIIUsb.usb_rx_buf_rxed[usb_len] = 0;
	bzero((char *)usb_rx_buf_cmd , 12);
	bzero((char *)usb_rx_buf_strparam , 12);
	bzero((char *)usb_rx_buf_str_valueparam , 12);
	rt = sscanf((char *)AventadorIIUsb.usb_rx_buf_rxed,"__! %s %s %s",(char *)usb_rx_buf_cmd,(char *)usb_rx_buf_strparam,(char *)usb_rx_buf_str_valueparam);
	if ( rt  )
	{
		if ( rt == 2 )
		{
			if (strcmp((char *)usb_rx_buf_cmd,"GET") == 0 )
			{
				if (strcmp((char *)usb_rx_buf_strparam,"AUTOBOOT") == 0 )
				{
					if ( HAL_GPIO_ReadPin(ENA_AUTOBOOT__GPIO_Port, ENA_AUTOBOOT__Pin) == GPIO_PIN_SET)
					{
						sprintf((char *)usbmessage_tx,"AUTOBOOT disabled by J16 closed\r\n");
					}
					else
					{
						uint64_t 	*autoboot_var_ptr64 = (uint64_t *)autoboot_var_ptr;
						uint64_t	abvar = *autoboot_var_ptr64;
						if ( abvar == AUTOBOOT_ON_FLASH)
							sprintf((char *)usbmessage_tx,"AUTOBOOT enabled both by J16 and SW\r\n");
						else
							sprintf((char *)usbmessage_tx,"AUTOBOOT enabled by J16 but disabled by SW\r\n");
					}
					usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
				}
				if (strcmp((char *)usb_rx_buf_strparam,"POWERS") == 0 )
				{
					bzero((char *)AventadorIIUsb.usb_rx_buf_rxed , usb_get_rx_len(usb_handle));
					compile_banner_and_send();
					calc_ad_values();
					return 3;
				}
			}
			if (strcmp((char *)usb_rx_buf_cmd,"POWER") == 0 )
			{
				if (strcmp((char *)usb_rx_buf_strparam,"ON") == 0 )
				{
					bzero((char *)AventadorIIUsb.usb_rx_buf_rxed , usb_get_rx_len(usb_handle));
					sprintf((char *)usbmessage_tx,"Power ON\r\n");
					usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
					forced_power = 1;
					return 1;
				}
				if (strcmp((char *)usb_rx_buf_strparam,"OFF") == 0 )
				{
					bzero((char *)AventadorIIUsb.usb_rx_buf_rxed , usb_get_rx_len(usb_handle));
					sprintf((char *)usbmessage_tx,"Power OFF\r\n");
					usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
					forced_unpower = 1;
					return 2;
				}
			}
		}
		if ( rt == 3 )
		{
			if (strcmp((char *)usb_rx_buf_cmd,"SET") == 0 )
			{
				if (strcmp((char *)usb_rx_buf_strparam,"AUTOBOOT") == 0 )
				{
					if (strcmp((char *)usb_rx_buf_str_valueparam,"ON") == 0 )
					{
						WriteFlashDoubleWord(FLASH_USER_START_ADDR,AUTOBOOT_ON_FLASH);
						sprintf((char *)usbmessage_tx,"SET AUTOBOOT ON\r\n");
						usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
					}
					if (strcmp((char *)usb_rx_buf_str_valueparam,"OFF") == 0 )
					{
						WriteFlashDoubleWord(FLASH_USER_START_ADDR,0x0);
						sprintf((char *)usbmessage_tx,"SET AUTOBOOT OFF\r\n");
						usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
					}
				}
			}
		}
	}
	return 0;
}

void supervisor_entry_callback(void)
{
	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON,PWR_SLEEPENTRY_WFI);
}

void process_1_pwr_manager(uint32_t process_id)
{
uint32_t	wakeup,flags;
uint32_t	uart_len,usb_len;


#ifdef AUTO_POWER_ON
	autoboot_var = AUTOBOOT_ON_FLASH;
#else
	if ( HAL_GPIO_ReadPin(ENA_AUTOBOOT__GPIO_Port, ENA_AUTOBOOT__Pin) == GPIO_PIN_SET)
	{
		autoboot_var = AUTOBOOT_ON_FLASH;
	}
	else
	{
		autoboot_var_ptr = (uint64_t *)FLASH_USER_START_ADDR;
		autoboot_var = *autoboot_var_ptr;
	}
#endif

	adc1_driver_handle = int_adc_register(&ADC1_Drv);
	adc2_driver_handle = int_adc_register(&ADC2_Drv);
	HAL_GPIO_WritePin(RED_LED__GPIO_Port, RED_LED__Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GREEN_LED__GPIO_Port, GREEN_LED__Pin,GPIO_PIN_SET);

	adc_start(adc1_driver_handle);
	adc_start(adc2_driver_handle);

	usb_handle = usb_device_driver_register(&Usb_channel);

	sprintf((char *)usbmessage_tx,"%s %s\r\n",aventadorII_name,aventadorII_version);
	usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));

	uart_driver_handle = uart_register(&Uart_Drv);
	uart_start_receive(uart_driver_handle);

	create_timer(TIMER_ID_0,PRC1_TICK,TIMERFLAGS_FOREVER | TIMERFLAGS_ENABLED);
	while(1)
	{
		wait_event(EVENT_TIMER | EVENT_USB_DEVICE_IRQ | EVENT_ADC1_IRQ | EVENT_ADC2_IRQ | EVENT_UART1_IRQ);
		get_wakeup_flags(&wakeup,&flags);

		if (( wakeup & WAKEUP_FROM_TIMER) == WAKEUP_FROM_TIMER)
		{
			banner_out();
			do_led();
			power_sm();
			if ( autoboot_var == AUTOBOOT_ON_FLASH )
			{
				if ( forced_power_counter <= POWERUP_DELAY_AUTO_POWER_ON )
				{
					if ( AventadorIISystem.val_vin > VL_VIN_MIN )
					{
						forced_power_counter++;
						if ( forced_power_counter == POWERUP_DELAY_AUTO_POWER_ON )
						{
							sprintf((char *)usbmessage_tx,"Auto Power ON\r\n");
							usb_send(usb_handle,usbmessage_tx,strlen((char *)usbmessage_tx));
							forced_power = 1;
						}
					}
				}
			}
		}
		if (( wakeup & WAKEUP_FROM_ADC1_IRQ) == WAKEUP_FROM_ADC1_IRQ)
		{
			adc1_ops++;
		}
		if (( wakeup & WAKEUP_FROM_ADC2_IRQ) == WAKEUP_FROM_ADC2_IRQ)
		{
			adc2_ops++;
		}

		if (( wakeup & WAKEUP_FROM_USB_DEVICE_IRQ) == WAKEUP_FROM_USB_DEVICE_IRQ)
		{
			if( monitor_cmds_parser() == 0 )
			{
				usb_len = usb_get_rx_len(usb_handle);
				uart_send(uart_driver_handle,AventadorIIUsb.usb_rx_buf_rxed,usb_len);
			}
		}
		if (( wakeup & WAKEUP_FROM_UART1_IRQ) == WAKEUP_FROM_UART1_IRQ)
		{
			/*
			if (( flags & WAKEUP_FLAGS_UART_RX) == WAKEUP_FLAGS_UART_RX)
			{
				uart_len = uart_get_rxlen(usb_handle);
				usb_send(usb_handle,uart_rx_buffer,uart_len);
			}
			*/
		}
	}
}
#endif // #ifndef	SAMPLE_PROCESSES_ENABLED
