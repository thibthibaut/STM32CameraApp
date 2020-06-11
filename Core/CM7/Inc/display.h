/**
 ******************************************************************************
 * @file    display.h
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
#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C"
{
#endif

/* #include "fp_vision_app.h" */
#include "stm32h747i_discovery.h"
#include "stm32h747i_discovery_camera.h"
#include "stm32h747i_discovery_lcd_patch.h"
#include "stm32h747i_discovery_qspi.h"
#include "stm32h747i_discovery_sdram.h"

/* Display related defines */
#define ARGB8888_BYTE_PER_PIXEL 4
#define LCD_RES_WIDTH 800
#define LCD_RES_HEIGHT 480
#define LCD_BBP ARGB8888_BYTE_PER_PIXEL
#define LCD_FRAME_BUFFER_SIZE (LCD_RES_WIDTH * LCD_RES_HEIGHT * LCD_BBP)

  /* Protoypes */

  uint8_t *get_lcd_frame_write_buff(void);
  void LCD_Init(void);
  int DisplayWelcomeScreen(void);
  void LCD_Refresh(void);
  void LCD_DMA2D2LCDWriteBuffer(uint32_t *pSrc, uint16_t x, uint16_t y,
                                uint16_t xsize, uint16_t ysize,
                                uint32_t input_color_format, int red_blue_swap);
  void DMA2D_MEMCOPY(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y,
                     uint16_t xsize, uint16_t ysize, uint32_t rowStride,
                     uint32_t input_color_format, uint32_t output_color_format,
                     int pfc, int red_blue_swap);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DISPLAY_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
