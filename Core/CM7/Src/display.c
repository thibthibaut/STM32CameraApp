/**
 ******************************************************************************
 * @file    display.c
 * @author  MCD Application Team
 * @brief   Library to manage LCD display through DMA2D
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
#include "main.h"
#include "microtrace.h"

/** @addtogroup STM32H747I-DISCO_Applications
 * @{
 */

/** @addtogroup FoodReco_MobileNetDerivative
 * @{
 */

/* Global variables ----------------------------------------------------------*/
#if defined(__ICCARM__)
#pragma location = "Lcd_Display"
#elif defined(__CC_ARM)
__attribute__((section(".Lcd_Display"), zero_init))
#elif defined(__GNUC__)
__attribute__((section(".Lcd_Display")))
#else
#error Unknown compiler
#endif
uint8_t lcd_display_global_memory[LCD_FRAME_BUFFER_SIZE * 2];

/*External SDRAM memory is used to store the LCD buffer*/
uint8_t *lcd_frame_read_buff = lcd_display_global_memory;
uint8_t *lcd_frame_write_buff =
    lcd_display_global_memory + LCD_FRAME_BUFFER_SIZE;

/* Private function prototypes -----------------------------------------------*/
static uint32_t GetBytesPerPixel(uint32_t dma2d_color);

uint8_t *get_lcd_frame_write_buff(void)
{
  return lcd_frame_write_buff;
}

/**
 * @brief  LCD Initialization
 * @param  None
 * @retval None
 */
void LCD_Init(void)
{

  MICROTRACE_BEGIN("Display", "LCD_INIT");
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, (uint32_t)lcd_frame_read_buff);

  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetFont(&Font24);

  LCD_SetFBStartAdress(0, (uint32_t)lcd_frame_write_buff);
  MICROTRACE_END("Display", "LCD_INIT");
}

uint32_t* GetWriteBuffer(void){
	return lcd_frame_read_buff;
}

/**
 * @brief Displays a food logo to LCD
 *
 * @param x x position in pixel
 * @param y y position in pixel
 * @param index Index of logo to be displayed. Should be between 0 and number of logos
 */
//void DisplayFoodLogo(const uint32_t x, const uint32_t y, const size_t index)
//{
//  BSP_LCD_DrawBitmap(x, y, (uint8_t *)Logos_128x128_bmp[index]);
//}

/**
 * @brief Refreshes LCD screen by performing a DMA transfer from lcd write buffer to lcd read buffer
 *
 */
void LCD_Refresh(void)
{
  /*Coherency purpose: clean the lcd_frame_write_buff area in L1 D-Cache before DMA2D reading*/
  SCB_CleanDCache_by_Addr((void *)lcd_frame_write_buff, LCD_FRAME_BUFFER_SIZE);

  DMA2D_MEMCOPY((uint32_t *)(lcd_frame_write_buff), (uint32_t *)(lcd_frame_read_buff), 0, 0, LCD_RES_WIDTH,
                LCD_RES_HEIGHT, LCD_RES_WIDTH, DMA2D_INPUT_ARGB8888, DMA2D_OUTPUT_ARGB8888, 0, 0);
}

/**
 * @brief Performs a DMA transfer from buffer to LCD write buffer with optional pixel format conversion and red/blue
 * channel swap
 *
 * @param pSrc pointer to input buffer
 * @param x x position on LCD in pixels
 * @param y y position on LCD in pixels
 * @param xsize width of the image to write in pixels
 * @param ysize height of the image to write in pixels
 * @param input_color_format input color format (e.g DMA2D_INPUT_RGB888)
 * @param red_blue_swap boolean flag for red-blue channel swap, 0 is no swap, 1 is swap
 */
void LCD_DMA2D2LCDWriteBuffer(uint32_t *pSrc, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize,
                              uint32_t input_color_format, int red_blue_swap)
{
  DMA2D_MEMCOPY((uint32_t *)pSrc, (uint32_t *)lcd_frame_write_buff, x, y, xsize, ysize, LCD_RES_WIDTH,
                input_color_format, DMA2D_OUTPUT_ARGB8888, 1, red_blue_swap);
}

/**
 * @brief Performs a DMA transfer from an arbitrary address to an arbitrary address
 *
 * @param pSrc address of the source
 * @param pDst address of the destination
 * @param x x position in the destination
 * @param y y position in the destination
 * @param xsize width of the source
 * @param ysize height of the source
 * @param rowStride width of the destination
 * @param input_color_format input color format (e.g DMA2D_INPUT_RGB888)
 * @param output_color_format output color format (e.g DMA2D_OUTPUT_ARGB888)
 * @param pfc boolean flag for pixel format conversion (set to 1 if input and output format are different, else 0)
 * @param red_blue_swap boolean flag for red-blue channel swap, 0 if no swap, else 1
 */
void DMA2D_MEMCOPY(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize,
                   uint32_t rowStride, uint32_t input_color_format, uint32_t output_color_format, int pfc,
                   int red_blue_swap)
{
  static DMA2D_HandleTypeDef DMA2D_Handle;

  uint32_t bytepp = GetBytesPerPixel(output_color_format);

  uint32_t destination = (uint32_t)pDst + (y * rowStride + x) * bytepp;
  uint32_t source = (uint32_t)pSrc;

  HAL_DMA2D_DeInit(&DMA2D_Handle);

  /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
  DMA2D_Handle.Init.Mode = pfc ? DMA2D_M2M_PFC : DMA2D_M2M;
  DMA2D_Handle.Init.ColorMode = output_color_format;

  /* Output offset in pixels == nb of pixels to be added at end of line to come to the  */
  /* first pixel of the next line : on the output side of the DMA2D computation         */
  DMA2D_Handle.Init.OutputOffset = rowStride - xsize;

  /*##-2- DMA2D Callbacks Configuration ######################################*/
  DMA2D_Handle.XferCpltCallback = NULL;

  /*##-3- Foreground Configuration ###########################################*/
  DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
  DMA2D_Handle.LayerCfg[1].InputAlpha = 0xFF;
  DMA2D_Handle.LayerCfg[1].InputColorMode = input_color_format;
  DMA2D_Handle.LayerCfg[1].InputOffset = 0;
  DMA2D_Handle.LayerCfg[1].RedBlueSwap = red_blue_swap ? DMA2D_RB_SWAP : DMA2D_RB_REGULAR;
  DMA2D_Handle.Instance = DMA2D;

  /* DMA2D Initialization */
  if (HAL_DMA2D_Init(&DMA2D_Handle) == HAL_OK)
  {
    if (HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1) == HAL_OK)
    {
      if (HAL_DMA2D_Start(&DMA2D_Handle, source, destination, xsize, ysize) == HAL_OK)
      {
        /* Polling For DMA transfer */
        HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 30);
      }
    }
  }
}

/**
 * @brief Helper function to get the bytes per pixels according to the DMA2D color mode
 *
 * @param dma2d_color DMA2D color mode
 * @return uint32_t bytes per pixels for the input mode, either 4,3,2 or 0 if unknown input mode
 */
static uint32_t GetBytesPerPixel(uint32_t dma2d_color)
{
  switch (dma2d_color)
  {
    case DMA2D_OUTPUT_ARGB8888:
      return 4;
    case DMA2D_OUTPUT_RGB888:
      return 3;
    case DMA2D_OUTPUT_RGB565:
    case DMA2D_OUTPUT_ARGB1555:
    case DMA2D_OUTPUT_ARGB4444:
      return 2;
    default:
      return 0;
  }
}

/**
  * @}
  */

/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
