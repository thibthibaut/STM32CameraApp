/*******************************************************************************
 * @file           : stm32_img_convert.c
 * @brief          : Image color space conversion module.
 * @copyright      : Copyright (c) 2020 STMicroelectronics.
 ******************************************************************************/

#include "stm32_img.h"
#include <stddef.h>

static void rgb565_to_gray8(uint16_t *pIn, uint8_t *pOut, uint32_t num_pixels);
static void rgb565_to_rgb888(uint16_t *pIn, uint8_t *pOut, uint32_t num_pixels);
static void rgb565_to_argb8888(uint16_t *pIn, uint8_t *pOut,
                               uint32_t num_pixels);
static void rgb888_to_rgb565(uint8_t *pIn, uint16_t *pOut, uint32_t num_pixels);
static void rgb888_to_gray8(uint8_t *pIn, uint8_t *pOut, uint32_t num_pixels);
static void gray8_to_rgb888(uint8_t *pIn, uint8_t *pOut, uint32_t num_pixels);
static void gray8_to_argb8888(uint8_t *pIn, uint8_t *pOut, uint32_t num_pixels);

void ImgToGrayscale(Image_t *imgSrc, Image_t *imgDst)
{
  IMG_ASSERT(imgSrc->format == PXFMT_RGB565 || imgSrc->format == PXFMT_RGB888);
  IMG_ASSERT(imgSrc->pData != NULL);
  IMG_ASSERT(imgSrc->width == imgDst->width);
  IMG_ASSERT(imgSrc->height == imgDst->height);
  IMG_ASSERT(imgDst->pData != NULL);
  IMG_ASSERT(imgDst->format == PXFMT_GRAY8);

  const uint32_t width = imgSrc->width;
  const uint32_t height = imgSrc->height;
  const uint32_t num_pixels = width * height;

  switch (imgSrc->format)
    {
    case PXFMT_RGB565:
      rgb565_to_gray8(imgSrc->pData, imgDst->pData, num_pixels);
      break;

    case PXFMT_RGB888:
      rgb888_to_gray8(imgSrc->pData, imgDst->pData, num_pixels);
      break;

    default:
      break;
    }
}

void ImgToRGB565(Image_t *imgSrc, Image_t *imgDst)
{
  IMG_ASSERT(imgSrc->format == PXFMT_RGB888);
  IMG_ASSERT(imgSrc->pData != NULL);
  IMG_ASSERT(imgSrc->width == imgDst->width);
  IMG_ASSERT(imgSrc->height == imgDst->height);
  IMG_ASSERT(imgDst->pData != NULL);
  IMG_ASSERT(imgDst->format == PXFMT_RGB565);

  const uint32_t width = imgSrc->width;
  const uint32_t height = imgSrc->height;
  const uint32_t num_pixels = width * height;

  switch (imgSrc->format)
  {
  case PXFMT_RGB888:
    rgb888_to_rgb565(imgSrc->pData, imgDst->pData, num_pixels);
    break;

  default:
    break;
  }
}

void ImgToRGB888(Image_t *imgSrc, Image_t *imgDst)
{
  IMG_ASSERT(imgSrc->format == PXFMT_RGB565 || imgSrc->format == PXFMT_GRAY8);
  IMG_ASSERT(imgSrc->pData != NULL);
  IMG_ASSERT(imgSrc->width == imgDst->width);
  IMG_ASSERT(imgSrc->height == imgDst->height);
  IMG_ASSERT(imgDst->pData != NULL);
  IMG_ASSERT(imgDst->format == PXFMT_RGB888);

  const uint32_t width = imgSrc->width;
  const uint32_t height = imgSrc->height;
  const uint32_t num_pixels = width * height;

  switch (imgSrc->format)
    {
    case PXFMT_GRAY8:
      gray8_to_rgb888(imgSrc->pData, imgDst->pData, num_pixels);
      break;

    case PXFMT_RGB565:
      rgb565_to_rgb888(imgSrc->pData, imgDst->pData, num_pixels);
      break;

    default:
      break;
    }
}

void ImgToARGB8888(Image_t *imgSrc, Image_t *imgDst)
{
  IMG_ASSERT(imgSrc->format == PXFMT_RGB565 || imgSrc->format == PXFMT_GRAY8);
  IMG_ASSERT(imgSrc->pData != NULL);
  IMG_ASSERT(imgSrc->width == imgDst->width);
  IMG_ASSERT(imgSrc->height == imgDst->height);
  IMG_ASSERT(imgDst->pData != NULL);
  IMG_ASSERT(imgDst->format == PXFMT_RGB888);

  const uint32_t width = imgSrc->width;
  const uint32_t height = imgSrc->height;
  const uint32_t num_pixels = width * height;

  switch (imgSrc->format)
    {
    case PXFMT_GRAY8:
      gray8_to_argb8888(imgSrc->pData, imgDst->pData, num_pixels);
      break;

    case PXFMT_RGB565:
      rgb565_to_argb8888(imgSrc->pData, imgDst->pData, num_pixels);
      break;

    default:
      break;
    }
}

#if defined(DMA2D)

void ImgToRGB565_DMA2D(DMA2D_HandleTypeDef *hdma2d, Image_t *imgSrc,
                       Image_t *imgDst)
{
  IMG_ASSERT(imgSrc->format == PXFMT_RGB888); /* Only RGB888 is supported */
  IMG_ASSERT(imgSrc->pData != NULL);
  IMG_ASSERT(imgSrc->width == imgDst->width);
  IMG_ASSERT(imgSrc->height == imgDst->height);
  IMG_ASSERT(imgDst->pData != NULL);
  IMG_ASSERT(imgDst->format == PXFMT_RGB565);
  IMG_ASSERT(imgDst->width <= 0x3fff);
  IMG_ASSERT(imgDst->height <= 0xffff);

  const uint32_t width = imgSrc->width;
  const uint32_t height = imgSrc->height;
  const uint32_t pSrc = (uint32_t) imgSrc->pData;
  const uint32_t pDst = (uint32_t) imgDst->pData;

  hdma2d->Instance = DMA2D;
  hdma2d->Init.Mode = DMA2D_M2M_PFC;
  hdma2d->Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d->Init.OutputOffset = 0;
  hdma2d->Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d->Init.RedBlueSwap = DMA2D_RB_SWAP;

  /* Configure foreground input layer */
  hdma2d->LayerCfg[1].InputOffset = 0;
  hdma2d->LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
  hdma2d->LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d->LayerCfg[1].InputAlpha = 0xFF;
  hdma2d->LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d->LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;

  if (HAL_DMA2D_Init(hdma2d) == HAL_OK)
    {
      if (HAL_DMA2D_ConfigLayer(hdma2d, 1) == HAL_OK)
        {
          HAL_DMA2D_Start(hdma2d, pSrc, pDst, width, height);
        }
    }
}

void ImgToRGB888_DMA2D(DMA2D_HandleTypeDef *hdma2d, Image_t *imgSrc, Image_t *imgDst)
{
  IMG_ASSERT(imgSrc->format == PXFMT_RGB565); /* Only RGB565 is supported */
  IMG_ASSERT(imgSrc->pData != NULL);
  IMG_ASSERT(imgSrc->width == imgDst->width);
  IMG_ASSERT(imgSrc->height == imgDst->height);
  IMG_ASSERT(imgDst->pData != NULL);
  IMG_ASSERT(imgDst->format == PXFMT_RGB888);
  IMG_ASSERT(imgDst->width <= 0x3fff);
  IMG_ASSERT(imgDst->height <= 0xffff);

  const uint32_t width = imgSrc->width;
  const uint32_t height = imgSrc->height;
  const uint32_t pSrc = (uint32_t) imgSrc->pData;
  const uint32_t pDst = (uint32_t) imgDst->pData;

  hdma2d->Instance = DMA2D;
  hdma2d->Init.Mode          = DMA2D_M2M_PFC;
  hdma2d->Init.ColorMode     = DMA2D_OUTPUT_RGB888;
  hdma2d->Init.OutputOffset  = 0;
  hdma2d->Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d->Init.RedBlueSwap   = DMA2D_RB_REGULAR;

  /* Configure foreground input layer */
  hdma2d->LayerCfg[1].InputOffset    = 0;
  hdma2d->LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d->LayerCfg[1].AlphaMode      = DMA2D_NO_MODIF_ALPHA;
  hdma2d->LayerCfg[1].InputAlpha     = 0xFF;
  hdma2d->LayerCfg[1].AlphaInverted  = DMA2D_REGULAR_ALPHA;
  hdma2d->LayerCfg[1].RedBlueSwap    =  DMA2D_RB_SWAP;

  if (HAL_DMA2D_Init(hdma2d) == HAL_OK) {
    if (HAL_DMA2D_ConfigLayer(hdma2d, 1) == HAL_OK) {
      HAL_DMA2D_Start(hdma2d, pSrc, pDst, width, height);
    }
  }
}

#endif /* DMA2D */

void rgb565_to_gray8(uint16_t *pIn, uint8_t *pOut, uint32_t num_pixels)
{
  for (uint32_t i = 0; i < num_pixels; i++) {
    uint16_t pixel = *pIn++;
    uint32_t red   = ((pixel & 0xf800u) >> 8);
    uint32_t green = ((pixel & 0x07e0u) >> 3);
    uint32_t blue  = ((pixel & 0x001fu) << 3);
    red   = red   * 19595;
    green = green * 38470;
    blue  = blue  *  7471;
    /* Add 2^15 before right-shift for round-to-the-nearest-integer div */
    *pOut++ = (uint8_t) ((red + green + blue + 0x8000) >> 16);
  }
}

void rgb565_to_rgb888(uint16_t *pIn, uint8_t *pOut, uint32_t num_pixels)
{
  for (uint32_t i = 0; i < num_pixels; i++)
    {
      uint16_t pixel = *pIn++;
      /* Extract R:5 G:6 B:5 components */
      uint32_t red = ((pixel & 0xf800u) >> 11);
      uint32_t green = ((pixel & 0x07e0u) >> 5);
      uint32_t blue = ((pixel & 0x001fu) >> 0);
      /* Left shift and copy MSBs to LSBs to improve conversion linearity */
      red = (red << 3) | (red >> 2);
      green = (green << 2) | (green >> 4);
      blue = (blue << 3) | (blue >> 2);
      *pOut++ = (uint8_t) red;
      *pOut++ = (uint8_t) green;
      *pOut++ = (uint8_t) blue;
    }
}

void rgb565_to_argb8888(uint16_t *pIn, uint8_t *pOut, uint32_t num_pixels)
{
  for (uint32_t i = 0; i < num_pixels; i++)
    {
      uint16_t pixel = *pIn++;
      /* Extract R:5 G:6 B:5 components */
      uint32_t red = ((pixel & 0xf800u) >> 11);
      uint32_t green = ((pixel & 0x07e0u) >> 5);
      uint32_t blue = ((pixel & 0x001fu) >> 0);
      /* Left shift and copy MSBs to LSBs to improve conversion linearity */
      red = (red << 3) | (red >> 2);
      green = (green << 2) | (green >> 4);
      blue = (blue << 3) | (blue >> 2);
      *pOut++ = (uint8_t) 0xFF;
      *pOut++ = (uint8_t) red;
      *pOut++ = (uint8_t) green;
      *pOut++ = (uint8_t) blue;
    }
}

void rgb888_to_gray8(uint8_t *pIn, uint8_t *pOut, uint32_t num_pixels)
{
  /* ITU-R BT.601-7 Table 2 - Integer coefficients of luminance */
  for (uint32_t i = 0; i < num_pixels; i++)
    {
      uint32_t red = *pIn++ * 19595;
      uint32_t green = *pIn++ * 38470;
      uint32_t blue = *pIn++ * 7471;
      /* Add 2^16 before right-shift for round-to-the-nearest-integer div */
      *pOut++ = (uint8_t)((red + green + blue + 0x8000) >> 16);
    }
}

void rgb888_to_rgb565(uint8_t *pIn, uint16_t *pOut, uint32_t num_pixels)
{
  for (uint32_t i = 0; i < num_pixels; i++) {
    uint32_t red   = *pIn++ >> 3;
    uint32_t green = *pIn++ >> 2;
    uint32_t blue  = *pIn++ >> 3;
    *pOut++ = (uint16_t) ((red << 11) | (green << 5) | blue);
  }
}

static void gray8_to_rgb888(uint8_t *pIn, uint8_t *pOut, uint32_t num_pixels)
{
  /* Copy GRAY8 value into RGB888 red, green and blue values. */
  for (uint32_t i = 0; i < num_pixels; i++)
    {
      *pOut++ = *pIn;
      *pOut++ = *pIn;
      *pOut++ = *pIn;
      pIn++;
    }
}

static void gray8_to_argb8888(uint8_t *pIn, uint8_t *pOut, uint32_t num_pixels)
{
  /* Copy GRAY8 value into RGB888 red, green and blue values. */
  for (uint32_t i = 0; i < num_pixels; i++)
    {
      *pOut++ = 0xFF;
      *pOut++ = *pIn;
      *pOut++ = *pIn;
      *pOut++ = *pIn;
      pIn++;
    }
}
