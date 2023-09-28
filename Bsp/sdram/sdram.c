/*
 * sdram.c
 *
 *  Created on: Sep 28, 2023
 *      Author: lefucjusz
 */

#include "sdram.h"

void sdram_init(SDRAM_HandleTypeDef *sdram)
{
	FMC_SDRAM_CommandTypeDef cmd = {0};
	HAL_StatusTypeDef status = HAL_OK;
	const uint32_t command_timeout = 0x8000;

	/* Send clock configuration enable command */
	cmd.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	cmd.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	cmd.AutoRefreshNumber = 1;
	cmd.ModeRegisterDefinition = 0;
	status = HAL_SDRAM_SendCommand(sdram, &cmd, command_timeout);
	if (status != HAL_OK) {
		Error_Handler();
	}

	/* Wait for at least 100us */
	HAL_Delay(1);

	/* Send Precharge All command */
	cmd.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	cmd.CommandMode = FMC_SDRAM_CMD_PALL;
	cmd.AutoRefreshNumber = 1;
	cmd.ModeRegisterDefinition = 0;
	status = HAL_SDRAM_SendCommand(sdram, &cmd, command_timeout);
	if (status != HAL_OK) {
		Error_Handler();
	}

	/* Send Auto Refresh command 8 times */
	cmd.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	cmd.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	cmd.AutoRefreshNumber = 8;
	cmd.ModeRegisterDefinition = 0;
	status = HAL_SDRAM_SendCommand(sdram, &cmd, command_timeout);
	if (status != HAL_OK) {
		Error_Handler();
	}

	/* Program the external memory mode register */
	cmd.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	cmd.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
	cmd.AutoRefreshNumber = 1;
	cmd.ModeRegisterDefinition = (SDRAM_MODEREG_BURST_LENGTH_1 |
							 	 SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
								 SDRAM_MODEREG_CAS_LATENCY_2 |
								 SDRAM_MODEREG_OPERATING_MODE_STANDARD |
								 SDRAM_MODEREG_WRITEBURST_MODE_SINGLE);
	status = HAL_SDRAM_SendCommand(sdram, &cmd, command_timeout);
	if (status != HAL_OK) {
		Error_Handler();
	}

	/* Set the rate refresh counter:
	 * refresh_rate = ((FMC_freq / SDClockPeriod) * (refresh_time / rows)) - 20
	 * refresh_rate = ((144MHz / 2) * (64ms / 8192)) - 20 = (72MHz * 7.8125us) - 20 = 542 */
	status = HAL_SDRAM_ProgramRefreshRate(sdram, 542);
	if (status != HAL_OK) {
		Error_Handler();
	}
}

