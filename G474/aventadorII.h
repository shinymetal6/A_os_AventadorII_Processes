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
 * Project : A_os_AventadorII
*/
/*
 * aventadorII.h
 *
 *  Created on: Feb 11, 2025
 *      Author: fil
 */

#ifndef G474_AVENTADORII_H_
#define G474_AVENTADORII_H_

extern	ADC_HandleTypeDef hadc1;
extern	ADC_HandleTypeDef hadc2;
extern	TIM_HandleTypeDef htim6;
extern	UART_HandleTypeDef huart1;

#define	USB_BUF_LEN			256
#define	UART_RX_BUF_SIZE	192
#define	UART_TX_BUF_SIZE	18

#define	ADC1_CHANNEL_3V3		0
#define	ADC1_CHANNEL_VIN		1
#define	ADC1_CHANNEL_5V			2

#define	ADC2_CHANNEL_1V2		0
#define	ADC2_CHANNEL_1V8		1

#define	PRC1_TICK				10

typedef struct
{
	uint8_t 		status;
	uint8_t 		btn_status;
	uint8_t 		board_wake_cntr;
	uint8_t			logo_timer;
	uint8_t			power_state_machine;
	uint8_t			power_state_machine_timeout;
	uint16_t 		val_3v3;
	uint16_t 		val_vin;
	uint16_t 		val_5v;
	uint16_t 		val_1v8;
	uint16_t 		val_1v2;
	uint16_t 		val_0v8;
	char			V3v3[8];
	char			vin[8];
	char			V5v[8];
	char			V1v8[8];
	char			V1v2[8];
	char			V0v8[8];

}AventadorIISystem_TypeDef;
/* status */
#define	SYS_STAT_CARRIER_POWER	0x40
#define	SYS_STAT_BOARD_POWER	0x80

/*power_state_machine */
#define	START						0
#define	UNPOWERED					1
#define	MODULE_POWERED				2
#define	WAIT_MODULE_SHUTDOWN_HIGH	3
#define	WAIT_MODULE_RESET_HIGH		4
#define	CARRIER_POWERED				5
#define	DEBOUNCE_BTN_POWEROFF		6
#define	UNPOWERED_CARRIER			7
#define	POWER_OFF_MODULE			8
#define	NO_VIN						9

#define	POWER_STABILIZE_TIME		10
#define	POWER_TO_EN_TIME			10
#define	WAIT_SHUTDOWNHIGH_TIMEOUT	200
#define	WAIT_RESETHIGH_TIMEOUT		200
#define	WAIT_SHUTDOWNLOW_TIMEOUT	10
#define	POWER_OFF_TIMEOUT			30

#define	BOARD_WAKEBTN_TIME		200
#define	BOARD_WAKEBTN_DEBOUNCE	50
#define	BOARD_POFFBTN_TIME		2000

#define	VL_VIN_MIN				1650

typedef struct
{
	uint8_t 		usb_status;
	uint8_t			usb_rx_buf_rxed[USB_BUF_LEN];
	uint8_t			usb_rx_buf[USB_BUF_LEN];
	uint8_t			usb_rx_buf_len;
	uint8_t			usb_rx_buf_index;
	uint8_t			usb_tx_buf[USB_BUF_LEN];
	uint8_t			usb_tx_buf_len;
	uint8_t 		usb_flags;
	uint8_t 		*xmodem_area;
	uint32_t 		xmodem_len;
	uint8_t 		*xmodem_header_area;
	uint32_t 		xmodem_header_len;
	uint8_t 		*xmodem_data_area;
	uint32_t 		xmodem_data_len;
	uint8_t 		external_flash_status;
}AventadorIIUsb_TypeDef;

//#define	AUTO_POWER_ON	1

#define POWERUP_DELAY_AUTO_POWER_ON		200

extern	uint8_t	aventadorII_name[32];
extern	uint8_t	aventadorII_version[32];

#endif /* G474_AVENTADORII_H_ */
