#include "main.h"

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
static void MPU_Config(void);
static void Error_Handler(void);
static void UART_Init(void);
static void CAMERA_Init(void);
static void LED_Init(void);
static void WaitCameraFrame(void);
void BSP_LCDEx_PrintfAtLineCenter(uint16_t line, const char *format, ...);

/* For printf  */
UART_HandleTypeDef huart1;

/* Private variables ---------------------------------------------------------*/
static volatile uint8_t new_frame_ready = 0;
static uint32_t camera_timing = 0; /*  For fps computation */

/* Frame buffers */
__attribute__((aligned(16)))
uint16_t camera_frame_buff[CAM_RES_WIDTH * CAM_RES_HEIGHT];

__attribute__((aligned(16)))
uint8_t gray_frame_buff[CAM_RES_WIDTH * CAM_RES_HEIGHT];

int main(void)
{

  /* Configure the MPU attributes */
  MPU_Config();

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  HAL_Init();

  /* Configure the system clock to 400 MHz */
  SystemClock_Config();

  /* Enable CRC HW IP block (needed by Cube.AI) */
  __HAL_RCC_CRC_CLK_ENABLE();

  /* LEDs initialization */
  LED_Init();

  /*  Enable QSPI for external Flash */
  BSP_QSPI_Init();
  BSP_QSPI_EnableMemoryMappedMode();

  /* Init UART (used for printf)*/
  UART_Init();

  /* Configure the Wakeup push-button in GPIO Mode */
  BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_GPIO);

  /* Activate joystick. */
  BSP_JOY_Init(JOY_MODE_GPIO);

  /* Initialize the LCD */
  LCD_Init();
  BSP_LCD_Clear(LCD_COLOR_BLACK);

  /* Initialize the Camera */
  CAMERA_Init();

  for (;;)
  {

    WaitCameraFrame();

    /* Create a camera image */
    Image_t cameraImg = {.width = CAM_RES_WIDTH,
                         .height = CAM_RES_HEIGHT,
                         .pData = camera_frame_buff,
                         .format = PXFMT_RGB565};

    /* Create a grayscale image  */
    Image_t grayImg = {.width = CAM_RES_WIDTH,
                       .height = CAM_RES_HEIGHT,
                       .pData = gray_frame_buff,
                       .format = PXFMT_GRAY8};

    /* Perform color conversion */
    ImgToGrayscale(&cameraImg, &grayImg);

    /*  Resume  camera  acquisition */
    BSP_CAMERA_Resume();

    /*  Display image to LCD buffer with 2x upsampling*/
    /*  (DMA2D doens't support Grayscale input) */
    uint32_t *lcd_buffer = (uint32_t *) get_lcd_frame_write_buff();
    uint8_t *image_buffer = (uint8_t *) grayImg.pData;
    int rowlcd = 0;
    int collcd = 0;
    for (int row = 0; row < CAM_RES_HEIGHT; row++)
    {
      for (int col = 0; col < CAM_RES_WIDTH; col++)
      {
        uint8_t r8 = *image_buffer;
        uint8_t g8 = *image_buffer;
        uint8_t b8 = *image_buffer;
        image_buffer++;
        uint32_t argb_pix = 0xFF000000 | (r8 << 16) | (g8 << 8) | b8;
        lcd_buffer[rowlcd * LCD_RES_WIDTH + collcd] = argb_pix;
        lcd_buffer[rowlcd * LCD_RES_WIDTH + collcd + 1] = argb_pix;
        lcd_buffer[(rowlcd + 1) * LCD_RES_WIDTH + collcd] = argb_pix;
        lcd_buffer[(rowlcd + 1) * LCD_RES_WIDTH + collcd + 1] = argb_pix;
        collcd += 2;
      }
      collcd = 0;
      rowlcd += 2;
    }

    /*  Compute display FPS */
    float fps = 1000.0 / (float) (HAL_GetTick() - camera_timing);
    camera_timing = HAL_GetTick();
    /*  Add additionnal info */
    BSP_LCDEx_PrintfAtLineCenter(2, "%.2f FPS", fps);
    /*  Printf to UART */
    printf("%.2f FPS\r\n", fps);

    /*  Refresh LCD screen (copy write buffer to read buffer) */
    LCD_Refresh();
  }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  LEDs Initialization
 * @param  None
 * @retval None
 */
static void LED_Init(void)
{
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_ORANGE);
  BSP_LED_Init(LED_RED);
  BSP_LED_Init(LED_BLUE);
}

/**
 * @brief  CAMERA Initialization
 * @param  None
 * @retval None
 */
static void CAMERA_Init(void)
{
  new_frame_ready = 0; /* clears flag */

  /* Reset and power down camera to be sure camera is Off prior start */
  BSP_CAMERA_PwrDown();

  /* Initialize the Camera */
  if (BSP_CAMERA_Init(CAMERA_RESOLUTION) != CAMERA_OK)
    Error_Handler();

  /* Start the camera capture */
  BSP_CAMERA_ContinuousStart((uint8_t *) camera_frame_buff);

  /* Wait for the camera initialization after HW reset */
  HAL_Delay(20);
}

/**
 * @brief  Camera Frame Event callback
 * @param  None
 * @retval None
 */
void BSP_CAMERA_FrameEventCallback(void)
{
  /*Notifies the backgound task about new frame available for processing*/
  new_frame_ready = 1;

  /*Suspend acquisition of the data stream coming from camera*/
  BSP_CAMERA_Suspend();
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 400000000 (Cortex-M7 CPU Clock)
 *            HCLK(Hz)                       = 200000000 (Cortex-M4 CPU, Bus
 * matrix Clocks) AHB Prescaler                  = 2 D1 APB3 Prescaler = 2 (APB3
 * Clock  100MHz) D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz) D2
 * APB2 Prescaler              = 2 (APB2 Clock  100MHz) D3 APB4 Prescaler = 2
 * (APB4 Clock  100MHz) HSE Frequency(Hz)              = 25000000 PLL_M = 5
 *            PLL_N                          = 160
 *            PLL_P                          = 2
 *            PLL_Q                          = 4
 *            PLL_R                          = 2
 *            VDD(V)                         = 3.3
 *            Flash Latency(WS)              = 4
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /*!< Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /* The voltage scaling allows optimizing the power consumption when the device
  is clocked below the maximum system frequency, to update the voltage scaling
  value regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
    {
    }

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if (ret != HAL_OK)
    {
      Error_Handler();
    }

  /* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType =
      (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 |
       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
  if (ret != HAL_OK)
    {
      Error_Handler();
    }

  /* Configure USART1 Clock */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

  /*
  Note : The activation of the I/O Compensation Cell is recommended with
  communication  interfaces (GPIO, SPI, FMC, QSPI ...)  when  operating at  high
  frequencies(please refer to product datasheet) The I/O Compensation Cell
  activation  procedure requires :
  - The activation of the CSI clock
  - The activation of the SYSCFG clock
  - Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR
  */

  /*activate CSI clock mandatory for I/O Compensation Cell*/
  __HAL_RCC_CSI_ENABLE();

  /* Enable SYSCFG clock mandatory for I/O Compensation Cell */
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* Enables the I/O Compensation Cell */
  HAL_EnableCompensationCell();
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void Error_Handler(void)
{
  BSP_LED_Off(LED_GREEN);
  BSP_LED_Off(LED_ORANGE);
  BSP_LED_Off(LED_RED);
  BSP_LED_Off(LED_BLUE);

  /* Turn LED RED on */
  BSP_LED_On(LED_RED);
  while (1)
    {
    }
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
 * @brief  Configure the MPU attributes for the device's memories.
 * @param  None
 * @retval None
 */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

#if EXT_SDRAM_CACHE_ENABLED == 0
  /*
  To make the memory region non cacheable and avoid any cache coherency
  maintenance:
  - either: MPU_ACCESS_NOT_BUFFERABLE + MPU_ACCESS_NOT_CACHEABLE
  - or: MPU_ACCESS_SHAREABLE => the S field is equivalent to non-cacheable
  memory
  */
  /* External SDRAM memory: LCD Frame buffer => non-cacheable */
  /*TEX=001, C=0, B=0*/
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0xD0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
#elif EXT_SDRAM_CACHE_ENABLED == 1
  /* External SDRAM memory: all as WBWA */
  /*TEX=001, C=1, B=1*/
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0xD0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
#elif EXT_SDRAM_CACHE_ENABLED == 2
  /*External SDRAM memory: all as Write Thru:*/
  /*TEX=000, C=1, B=0*/
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0xD0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
#else
#error Please check definition of EXT_SDRAM_CACHE_ENABLED define
#endif

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /*Internal SRAM memory: cache policies are WBWA (Write Back and Write
   * Allocate) by default */

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void UART_Init(void)
{
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

  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) !=
      HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) !=
      HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

void BSP_CAMERA_ErrorCallback(void)
{
  BSP_LED_On(LED_RED);
  while (1)
    ;
}

int _write(int fd, const void *buff, int count)
{
  HAL_StatusTypeDef status;

  if ((count < 0) && (fd != STDOUT_FILENO) && (fd != STDERR_FILENO))
  {
    errno = EBADF;
    return -1;
  }
  status = HAL_UART_Transmit(&huart1, (uint8_t *) buff, count, HAL_MAX_DELAY);
  return (status == HAL_OK ? count : 0);
}

static void WaitCameraFrame(void)
{
  while (new_frame_ready == 0)
  {
  }
}

void BSP_LCDEx_PrintfAtLineCenter(uint16_t line, const char *format, ...)
{

  static char buffer[128];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, 128, format, args);
  BSP_LCD_DisplayStringAt(0, LINE(line), (uint8_t *) buffer, CENTER_MODE);
  va_end(args);
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line
  number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
  line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
