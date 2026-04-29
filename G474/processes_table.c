/*
 * processes_table.c
 *
 *  Created on: Sep 13, 2023
 *      Author: fil
 */
#include "main.h"
#include "A_os_includes.h"
#include "aventadorII.h"

#ifndef	SAMPLE_PROCESSES_ENABLED

extern	void process_1_pwr_manager(uint32_t process_id);	//This is process2
extern	void process_2(uint32_t process_id);	//This is process2
extern	void process_3(uint32_t process_id);	//This is process3
extern	void process_4(uint32_t process_id);	//This is process4 of the application


#define		AVENTADORII_NAME	"AventadorII-03"
#define		AVENTADORII_VERSION	"V1.33nlnu"

VERSIONING	uint8_t	app_name[32] 		= AVENTADORII_NAME;
VERSIONING	uint8_t	app_version[32] 	= AVENTADORII_VERSION;
uint8_t	aventadorII_name[32] 			= AVENTADORII_NAME;
uint8_t	aventadorII_version[32] 		= AVENTADORII_VERSION;

USRprcs_t	UserProcesses[USR_PROCESS_NUMBER] =
{
		{
				.user_process = process_1_pwr_manager,
				.stack_size = 1024,
		},
		{
				.user_process = process_2,
				.stack_size = 1024,
		},
		{
				.user_process = process_3,
				.stack_size = 1024,
		},
		{
				.user_process = process_4,
				.stack_size = 1024,
		}
};
#endif // #ifndef	SAMPLE_PROCESSES_ENABLED

