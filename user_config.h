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
 * user_config.h
 *
 *  Created on: Sep 1, 2025
 *      Author: fil
 */

#ifndef PROJECT_LIB_MODULES_H_
#define PROJECT_LIB_MODULES_H_

#define		USER_PROCESSES	1

/* Libraries */
//#define	HELIX_ENABLE	1
//#define	LORAWAN_ENABLE	1
//#define	LWIP_ENABLE		1
//#define	WIFI_ENABLE		1

#ifdef LWIP_ENABLE
	#define	NETWORKING_ENABLED		1
	//#define	MQTT_ENABLE	1
#endif // #ifdef LWIP_ENABLE
#ifdef WIFI_ENABLE
	#define	WIFI_ESP01S		1
#endif // #ifdef WIFI_ENABLE

/* Modules */
//#define	HEX_DEC_ENABLE	1
//#define	XMODEM_ENABLE	1
//#define	MODBUS_ENABLE	1
//#define	SOUND_ENABLED	1

/* drivers */

#define USB_DEVICE_ENABLED	1
#ifdef USB_DEVICE_ENABLED
	#define	USB_CDC				1
	//#define	USB_MIDI			1
#endif // #ifdef USB_ENABLED


//#define HAS_LCD			1
#ifdef HAS_LCD
	#define	BOARD_LCD	LCD_IS_7735
	//#define	BOARD_LCD	LCD_IS_9341
#endif //#ifdef HAS_LCD

#ifndef LED_GPIO_Port
#define LED_Pin			LD1_Pin
#define LED_GPIO_Port	LD1_GPIO_Port
#endif

#define	BOARD_NAME			"AventadorII"
#define	MACHINE_NAME		"Av"
#define	MACHINE_VERSION		"B"

#endif /* PROJECT_LIB_MODULES_H_ */
