/*******************************************************************************
 * @file           : stm32_img_crop.c
 * @brief          : Crop module providing image crop functions.
 * @copyright      : Copyright (c) 2020 STMicroelectronics.
 ******************************************************************************/

#include "stm32_img.h"
#include <string.h>


void ImgCropCenter(Image_t *imgSrc, Image_t *imgDst)
{
  IMG_ASSERT(imgSrc->width >= imgDst->width);
  IMG_ASSERT(imgSrc->height >= imgDst->height);

  ImgRect_t crop_rect;
  crop_rect.x0 = (imgSrc->height - imgDst->height) / 2;
  crop_rect.y0 = (imgSrc->width - imgDst->width) / 2;
  crop_rect.width = imgDst->width;
  crop_rect.height = imgDst->height;

  ImgCrop(imgSrc, imgDst, &crop_rect);
}

void ImgCrop(Image_t *imgSrc, Image_t *imgDst, ImgRect_t *cropRect)
{
  IMG_ASSERT(imgSrc->width >= cropRect->width);
  IMG_ASSERT(imgSrc->height >= cropRect->height);
  IMG_ASSERT(imgSrc->pData != NULL);
  IMG_ASSERT(imgDst->pData != NULL);
  IMG_ASSERT(imgSrc->format == imgDst->format);

  const uint32_t src_width = imgSrc->width;
  const uint32_t dst_width = cropRect->width;
  const uint32_t dst_height = cropRect->height;
  const uint32_t left = cropRect->x0;
  const uint32_t top = cropRect->y0;

  const uint32_t pixel_size = IMG_BYTES_PER_PX(imgSrc->format);

  const uint8_t *pIn = (uint8_t *)imgSrc->pData + left * pixel_size;
  uint8_t *pOut = (uint8_t *)imgDst->pData;

  /* Copy line per line */
  for (uint32_t i = 0; i < dst_height; i++)
  {
    const size_t dst_line_size = dst_width * pixel_size;
    memcpy(pOut, pIn + (i + top) * src_width * pixel_size, dst_line_size);
    pOut += dst_line_size;
  }
}
