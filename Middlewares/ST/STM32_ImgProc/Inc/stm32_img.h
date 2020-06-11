/*******************************************************************************
 * @file           : stm32_img.h
 * @brief          : Header file for the STM32 Image processing library.
 * @copyright      : Copyright (c) 2020 STMicroelectronics.
 ******************************************************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(STM32L4R9xx)
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_dma2d.h"
#elif defined(STM32H747xx)
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_dma2d.h"
#endif

/**
 * @brief  Image function return code.
 */
typedef enum
{
  IMG_OK           = 0x00,
  IMG_ERROR        = 0x01,
  IMG_ERROR_SIZE   = 0x02,
  IMG_ERROR_FORMAT = 0x03
} imgstatus_t;

/**
 * @brief Image Pixel Formats.
 */
typedef enum
{
  PXFMT_GRAY8,    /*!< 8-bit Grayscale     */
  PXFMT_RGB565,   /*!< RGB565 color mode   */
  PXFMT_RGB888,   /*!< RGB888 color mode   */
  PXFMT_ARGB8888, /*!< ARGB8888 color mode */
  PXFMT_FLOAT32   /*!< FLOAT32 color mode */
} pxfmt_t;

/**
 * @brief Image resize interpolation method
 */
typedef enum
{
  NEAREST,  /*!< Nearest-neighbor interpolation */
  BILINEAR, /*!< Linear interpolation           */
  BICUBIC   /*!< Cubic interpolation            */
} intrpl_t;

/**
 * @brief Instance structure for images.
 */
typedef struct
{
  uint32_t width;  /*!< Image width         */
  uint32_t height; /*!< Image height        */
  void *pData;     /*!< Image data buffer   */
  pxfmt_t format;  /*!< Image pixel format  */
} Image_t;

/**
 * @brief Instance structure for 2D rectangles.
 */
typedef struct
{
  uint32_t x0;    /*!< Left     */
  uint32_t y0;    /*!< Top      */
  uint32_t width;  /*!< Width   */
  uint32_t height; /*!< Height  */
} ImgRect_t;


#define IMG_BYTES_PER_PX(pxfmt)  (    \
((pxfmt) == PXFMT_GRAY8) ? 1 :        \
((pxfmt) == PXFMT_RGB565) ? 2 :       \
((pxfmt) == PXFMT_RGB888) ? 3 :       \
((pxfmt) == PXFMT_ARGB8888) ? 4 : 0)

#ifdef USE_IMG_ASSERT
#define IMG_ASSERT(expr)  \
((expr) ? (void)0U : img_assert_failed((char *) __FUNCTION__, (char *)__FILE__, __LINE__))
void img_assert_failed(char *function, char *file, uint32_t line);
#else
#define IMG_ASSERT(expr) ((void)0U)
#endif /* USE_IMG_ASSERT */

void ImgCrop(Image_t *imgSrc, Image_t *imgDst, ImgRect_t *cropRect);
void ImgCropCenter(Image_t *imgSrc, Image_t *imgDst);
void ImgResize(Image_t *imgSrc, Image_t *imgDst, intrpl_t intrpl);
void ImgResizeCrop(Image_t *imgSrc, Image_t *imgDst, ImgRect_t *roi, intrpl_t intrpl);
void ImgToGrayscale(Image_t *imgSrc, Image_t *imgDst);
void ImgToRGB565(Image_t *imgSrc, Image_t *imgDst);
void ImgToRGB888(Image_t *imgSrc, Image_t *imgDst);
void ImgToARGB8888(Image_t *imgSrc, Image_t *imgDst);
#if defined (DMA2D)
void ImgToRGB565_DMA2D(DMA2D_HandleTypeDef *hdma2d, Image_t *imgSrc, Image_t *imgDst);
void ImgToRGB888_DMA2D(DMA2D_HandleTypeDef *hdma2d, Image_t *imgSrc, Image_t *imgDst);
#endif

#ifdef __cplusplus
}
#endif

#endif /* IMAGE_H */
