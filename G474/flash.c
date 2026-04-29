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
 * flash.c
 *
 *  Created on: Dec 4, 2025
 *      Author: fil
 */

#include "main.h"
#include "A_os_includes.h"
#include "aventadorII.h"
#include "stm32g4xx_hal.h"
#ifndef	SAMPLE_PROCESSES_ENABLED

#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_flash.h"
#include "stm32g4xx_hal_flash_ex.h"

uint32_t EraseFlashPage(uint32_t address)
{
	FLASH_EraseInitTypeDef EraseStruct;
    uint32_t page_error = 0;

    // Unlock flash
    if (HAL_FLASH_Unlock() != HAL_OK)
        return HAL_ERROR;

    // Fill erase structure
    EraseStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseStruct.Banks       = FLASH_BANK_1;          // G474 is single-bank for <512K
    EraseStruct.Page        = (address - FLASH_BASE) / FLASH_PAGE_SIZE;
    EraseStruct.NbPages     = 1;

    // Erase
    if (HAL_FLASHEx_Erase(&EraseStruct, &page_error) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }

    HAL_FLASH_Lock();
    return HAL_OK;
}

uint32_t WriteFlashDoubleWord(uint32_t address, uint64_t data)
{
	EraseFlashPage(address);
    if (HAL_FLASH_Unlock() != HAL_OK)
        return HAL_ERROR;

    // Address must be 8-byte aligned
    if (address % 8 != 0)
    {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }

    // Program double-word
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }

    HAL_FLASH_Lock();
    return HAL_OK;
}

#endif // #ifndef	SAMPLE_PROCESSES_ENABLED
