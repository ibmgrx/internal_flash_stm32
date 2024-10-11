/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define FLASH_USER_START_ADDR   0x08080000   // Alamat awal dari User Flash memory
#define FLASH_USER_STR_ADDR     0x08080020   // Alamat untuk menyimpan string
#define FLASH_USER_INT_ADDR     0x08080040   // Alamat untuk menyimpan integer
#define FLASH_USER_FLOAT_ADDR   0x08080060   // Alamat untuk menyimpan float

#define FLASH_USER_END_ADDR     0x080807FF   // Alamat akhir dari User Flash memory
#define DATA_TO_WRITE           "ASTAGHFIRULLAHAL'ADZIM......."      // Data yang akan ditulis

uint32_t Address = 0, PageError = 0;
FLASH_EraseInitTypeDef EraseInitStruct;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Fungsi untuk menulis integer ke flash
HAL_StatusTypeDef Write_Flash_Integer(uint32_t start_address, int32_t data)
{
    HAL_FLASH_Unlock();
    uint64_t data64 = (uint64_t)data;
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, start_address, data64);
    HAL_FLASH_Lock();
    return status;
}

// Fungsi untuk membaca integer dari flash
int32_t Read_Flash_Integer(uint32_t start_address)
{
    uint64_t data64 = *(__IO uint64_t *)start_address;
    int32_t data32 = (int32_t)(data64 & 0xFFFFFFFF);
    return data32;
}

// Fungsi untuk menulis float ke flash
HAL_StatusTypeDef Write_Flash_Float(uint32_t start_address, float data)
{
    HAL_FLASH_Unlock();
    uint64_t data64 = 0;
    memcpy(&data64, &data, sizeof(float));
    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, start_address, data64);
    HAL_FLASH_Lock();
    return status;
}

// Fungsi untuk membaca float dari flash
float Read_Flash_Float(uint32_t start_address)
{
    uint64_t data64 = *(__IO uint64_t *)start_address;
    float data_float = 0.0f;
    memcpy(&data_float, &data64, sizeof(float));
    return data_float;
}

// Fungsi untuk menulis string ke flash
HAL_StatusTypeDef Write_Flash_String(uint32_t start_address, const char* data)
{
    HAL_FLASH_Unlock();
    uint64_t data64 = 0;
    size_t len = strlen(data);

    // Tulis data string dalam blok 64-bit
    for (size_t i = 0; i < len; i += 8)
    {
        // Copy 8 byte dari string ke data64, atau sisa byte jika kurang dari 8
        memcpy(&data64, &data[i], (len - i) >= 8 ? 8 : (len - i));

        // Tulis blok 64-bit ke flash
        HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, start_address + i, data64);
        if (status != HAL_OK)
        {
            HAL_FLASH_Lock();
            return status;
        }

        data64 = 0; // Reset data64 untuk blok berikutnya
    }

    HAL_FLASH_Lock();
    return HAL_OK;
}

// Fungsi untuk membaca string dari flash
void Read_Flash_String(uint32_t start_address, char* buffer, size_t max_len)
{
    uint64_t data64 = 0;
    size_t i = 0;

    // Membaca data string dari flash dalam blok 64-bit
    while (i < max_len)
    {
        data64 = *(__IO uint64_t *)(start_address + i);

        // Copy data dari flash ke buffer string
        memcpy(&buffer[i], &data64, (max_len - i) >= 8 ? 8 : (max_len - i));
        i += 8;

        // Hentikan jika null-terminator ditemukan
        if (strchr((char*)&data64, '\0') != NULL)
        {
            break;
        }
    }

    // Pastikan buffer memiliki null terminator
    buffer[max_len - 1] = '\0';
}

/**
  * @brief  Fungsi untuk menghapus sektor flash pada STM32.
  * @param  start_address: Alamat awal dari sektor yang ingin dihapus.
  * @retval HAL Status (HAL_OK jika berhasil, HAL_ERROR jika gagal)
  */
uint32_t Erase_Flash_Sector(uint32_t start_address)
{
    uint32_t PageError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;

    // Unlock flash memory sebelum penghapusan
    HAL_FLASH_Unlock();

    // Inisialisasi penghapusan halaman flash
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Page = (start_address - FLASH_BASE) / FLASH_PAGE_SIZE;  // Menghitung halaman
    EraseInitStruct.NbPages = 1;  // Menghapus 1 halaman

    // Melakukan penghapusan halaman
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        // Jika ada kesalahan saat menghapus
        HAL_FLASH_Lock();  // Mengunci flash setelah operasi
        return HAL_ERROR;  // Mengembalikan status error
    }

    // Kunci kembali flash memory setelah operasi penghapusan selesai
    HAL_FLASH_Lock();

    return HAL_OK;  // Mengembalikan status berhasil
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

    // Contoh penggunaan untuk menulis dan membaca kalimat
    const char* my_string = "ASTAGHFIRULLAHAL'ADZIM";
    if (Write_Flash_String(FLASH_USER_STR_ADDR, my_string) == HAL_OK)
    {
        char read_string[50];
        Read_Flash_String(FLASH_USER_STR_ADDR, read_string, sizeof(read_string));
        if (strcmp(read_string, my_string) == 0)
        {
            // Berhasil menulis dan membaca string
        }
    }

    // Contoh penggunaan untuk menulis dan membaca integer
    int32_t my_integer = 12345678;
    if (Write_Flash_Integer(FLASH_USER_INT_ADDR, my_integer) == HAL_OK)
    {
        int32_t read_integer = Read_Flash_Integer(FLASH_USER_INT_ADDR);
        if (read_integer == my_integer)
        {
            // Berhasil menulis dan membaca integer
        }
    }

    // Contoh penggunaan untuk menulis dan membaca float
    float my_float = 3.14159f;
    if (Write_Flash_Float(FLASH_USER_FLOAT_ADDR, my_float) == HAL_OK)
    {
        float read_float = Read_Flash_Float(FLASH_USER_FLOAT_ADDR);
        if (read_float == my_float)
        {
            // Berhasil menulis dan membaca float
        }
    }


//    // Contoh penggunaan fungsi penghapusan sektor
//    if (Erase_Flash_Sector(FLASH_USER_START_ADDR) != HAL_OK)
//    {
//        // Jika terjadi kesalahan dalam penghapusan
//        while (1);
//    }


	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = 0;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 60;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
