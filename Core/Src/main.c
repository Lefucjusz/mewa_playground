/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "tca9548a.h"
#include "ssd1306.h"
#include "logger.h"
#include "cs4270.h"
#include "fatfs.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0003)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s1;
DMA_HandleTypeDef hdma_spi1_tx;

SD_HandleTypeDef hsd1;

MDMA_HandleTypeDef hmdma_mdma_channel40_sdmmc1_end_data_0;
SDRAM_HandleTypeDef hsdram1;

/* USER CODE BEGIN PV */

static FATFS __attribute__((section(".sdram"))) fatfs;

static int16_t __attribute__((section(".sdram"))) audio_buffer[2 * 16384];

static volatile bool first_half = false;
static volatile bool second_half = false;

//static int16_t __attribute__((section(".sdram"))) audio_data[2 * 4800];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_MDMA_Init(void);
static void MX_I2S1_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_I2C1_Init(void);
static void MX_FMC_Init(void);
/* USER CODE BEGIN PFP */
static void SDRAM_Init(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  // TODO setup MPU and caches
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_MDMA_Init();
  MX_I2S1_Init();
  MX_SDMMC1_SD_Init();
  MX_I2C1_Init();
  MX_FMC_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  tca9548a_init(&hi2c1, 0);
  tca9548a_switch_to(1);
  ssd1306_Init();
  logger_init();

  bool codec_ret = cs4270_init(&hi2c1, 0);

  cs4270_set_attenuation(24);

  logger_log("Codec %s", codec_ret ? "OK" : "fail");

  const char *const mount_point = "";
  const char *const file_path = "01. A Gallant Gentleman.wav";

  FRESULT ret;
  FIL f;

  logger_log("Mounting SD card...");

  ret = f_mount(&fatfs, mount_point, 1);
  if (ret) {
	  logger_log("Failed");
	  while(1);
  }

  logger_log("Opening %s...", file_path);
  ret = f_open(&f, file_path, FA_READ);
  if (ret) {
	  logger_log("Failed");
	  while(1);
  }

  logger_log("Reading file...");
  const size_t file_size = f_size(&f);
  logger_log("File size: %uB", file_size);

//  size_t seek_val = file_size * 0.7;
//  if (seek_val & 1) {
//	  seek_val++;
//  }
//
//  f_lseek(&f, seek_val);

  logger_log("Filling buffer...");
  size_t bytes_read;
  ret = f_read(&f, audio_buffer, 2 * 16384 * sizeof(audio_buffer[0]), &bytes_read);
  if (ret) {
  	  logger_log("Failed");
  	  while(1);
  }

  HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)audio_buffer, sizeof(audio_buffer)/sizeof(audio_buffer[0]));

  logger_log("Playing");

  while (1) {
	  if (first_half) {
		  first_half = false;
		  f_read(&f, audio_buffer, 16384 * sizeof(audio_buffer[0]), &bytes_read);
	  }

	  if (second_half) {
		  second_half = false;
		  f_read(&f, &audio_buffer[16384], 16384 * sizeof(audio_buffer[0]), &bytes_read);
	  }

	  if (bytes_read == 0) {
		  break;
	  }
  }

  HAL_I2S_DMAStop(&hi2s1);
  logger_log("Playback done");

  logger_log("Closing %s...", file_path);
  ret = f_close(&f);
  if (ret) {
      logger_log("Failed");
      while(1);
  }

  logger_log("Unmounting SD card...");
  ret = f_mount(0, mount_point, 0);
  if (ret) {
      logger_log("Failed");
      while(1);
  }

  logger_log("Success!");


//  bool status = cs4270_init(&hi2c1, 0);
//  logger_log("codec status: %d", status);
//
//  for (size_t i = 0; i < 4800; ++i) {
//	  int16_t val = (int16_t)(32767.0f * sin(2.0f * M_PI * 8000.0f * i / 48000.0f));
//	  audio_data[2 * i] = val;
//	  audio_data[2 * i + 1] = val;
//  }
//
//  HAL_StatusTypeDef st = HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t*)audio_data, sizeof(audio_data)/sizeof(audio_data[0]));
//
//  logger_log("DMA status: %d", st);
//
//  HAL_Delay(1000);
//
//  cs4270_set_volume(100);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	  static int cnt = 10000;
//	  logger_log("Logger test: %d", cnt--);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 23;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 12;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 3584;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00301739;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2S1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S1_Init(void)
{

  /* USER CODE BEGIN I2S1_Init 0 */

  /* USER CODE END I2S1_Init 0 */

  /* USER CODE BEGIN I2S1_Init 1 */

  /* USER CODE END I2S1_Init 1 */
  hi2s1.Instance = SPI1;
  hi2s1.Init.Mode = I2S_MODE_MASTER_FULLDUPLEX;
  hi2s1.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s1.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s1.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  hi2s1.Init.CPOL = I2S_CPOL_LOW;
  hi2s1.Init.FirstBit = I2S_FIRSTBIT_MSB;
  hi2s1.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
  hi2s1.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
  hi2s1.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_DISABLE;
  if (HAL_I2S_Init(&hi2s1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S1_Init 2 */

  /* USER CODE END I2S1_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

}

/**
  * Enable MDMA controller clock
  * Configure MDMA for global transfers
  *   hmdma_mdma_channel40_sdmmc1_end_data_0
  */
static void MX_MDMA_Init(void)
{

  /* MDMA controller clock enable */
  __HAL_RCC_MDMA_CLK_ENABLE();
  /* Local variables */

  /* Configure MDMA channel MDMA_Channel0 */
  /* Configure MDMA request hmdma_mdma_channel40_sdmmc1_end_data_0 on MDMA_Channel0 */
  hmdma_mdma_channel40_sdmmc1_end_data_0.Instance = MDMA_Channel0;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.Request = MDMA_REQUEST_SDMMC1_END_DATA;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.Priority = MDMA_PRIORITY_LOW;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.SourceInc = MDMA_SRC_INC_BYTE;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.DestinationInc = MDMA_DEST_INC_BYTE;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.BufferTransferLength = 1;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.SourceBlockAddressOffset = 0;
  hmdma_mdma_channel40_sdmmc1_end_data_0.Init.DestBlockAddressOffset = 0;
  if (HAL_MDMA_Init(&hmdma_mdma_channel40_sdmmc1_end_data_0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure post request address and data masks */
  if (HAL_MDMA_ConfigPostRequestMask(&hmdma_mdma_channel40_sdmmc1_end_data_0, 0, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /* MDMA interrupt initialization */
  /* MDMA_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(MDMA_IRQn);

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 1;
  SdramTiming.ExitSelfRefreshDelay = 5;
  SdramTiming.SelfRefreshTime = 4;
  SdramTiming.RowCycleDelay = 5;
  SdramTiming.WriteRecoveryTime = 2;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */
  SDRAM_Init();
  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS4270_NRESET_GPIO_Port, CS4270_NRESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TCA9548A_NRESET_GPIO_Port, TCA9548A_NRESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS4270_NRESET_Pin */
  GPIO_InitStruct.Pin = CS4270_NRESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS4270_NRESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DBG_PAD1_Pin */
  GPIO_InitStruct.Pin = DBG_PAD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(DBG_PAD1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TCA9548A_NRESET_Pin */
  GPIO_InitStruct.Pin = TCA9548A_NRESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TCA9548A_NRESET_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

static void SDRAM_Init(void)
{
	FMC_SDRAM_CommandTypeDef cmd = {0};
	HAL_StatusTypeDef status = HAL_OK;

	/* Send clock configuration enable command */
	cmd.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	cmd.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	cmd.AutoRefreshNumber = 1;
	cmd.ModeRegisterDefinition = 0;
	status = HAL_SDRAM_SendCommand(&hsdram1, &cmd, 0xFFFF); // TODO magic number
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
	status = HAL_SDRAM_SendCommand(&hsdram1, &cmd, 0xFFFF); // TODO magic number
	if (status != HAL_OK) {
		Error_Handler();
	}

	/* Send Auto Refresh command 8 times */
	cmd.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	cmd.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	cmd.AutoRefreshNumber = 8;
	cmd.ModeRegisterDefinition = 0;
	status = HAL_SDRAM_SendCommand(&hsdram1, &cmd, 0xFFFF); // TODO magic number
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
	status = HAL_SDRAM_SendCommand(&hsdram1, &cmd, 0xFFFF); // TODO magic number
	if (status != HAL_OK) {
		Error_Handler();
	}

	/* Set the rate refresh counter:
	 * refresh_rate = (FMC_freq * (full_refresh_time / refresh_cycles)) - 20
	 * refresh_rate = (72MHz * (64ms/8192)) - 20 = (72MHz * 7.8125us) - 20 = 542
	 *  */
	status = HAL_SDRAM_ProgramRefreshRate(&hsdram1, 542);
	if (status != HAL_OK) {
		Error_Handler();
	}
}

// TODO this shouldn't be here
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	first_half = true;
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	second_half = true;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
