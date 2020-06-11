/*******************************************************************************
 * @file           : stm32_img_resize.c
 * @brief          : Resize module providing image scaling with cropping.
 * @copyright      : Copyright (c) 2020 STMicroelectronics.
 ******************************************************************************/

#include "stm32_img.h"
#include <stddef.h>

static void ImageResize_NearestNeighbor(uint8_t *srcImage, uint32_t srcW, uint32_t srcH,
                                        uint32_t pixelSize, uint32_t roiX, uint32_t roiY,
                                        uint32_t roiW, uint32_t roiH, uint8_t *dstImage,
                                        uint32_t dstW, uint32_t dstH);
static void ImageResize_Bilinear(uint8_t *srcImage, uint32_t srcW, uint32_t srcH,
                                 uint32_t pixelSize, uint32_t roiX, uint32_t roiY,
                                 uint32_t roiW, uint32_t roiH, uint8_t *dstImage,
                                 uint32_t dstW, uint32_t dstH);


void ImgResize(Image_t *imgSrc, Image_t *imgDst, intrpl_t intrpl)
{
  IMG_ASSERT(intrpl == NEAREST || intrpl == BILINEAR);
  IMG_ASSERT(imgSrc->pData != NULL);
  IMG_ASSERT(imgSrc->format == imgDst->format);
  IMG_ASSERT(imgDst->pData != NULL);

  const uint32_t pixel_size = IMG_BYTES_PER_PX(imgSrc->format);

  switch (intrpl)
  {
  case NEAREST:
    ImageResize_NearestNeighbor(imgSrc->pData, imgSrc->width, imgSrc->height,
                                pixel_size, 0, 0, imgSrc->width, imgSrc->height,
                                imgDst->pData, imgDst->width, imgDst->height);
    break;

  case BILINEAR:
    ImageResize_Bilinear(imgSrc->pData, imgSrc->width, imgSrc->height,
                         pixel_size, 0, 0, imgSrc->width, imgSrc->height,
                         imgDst->pData, imgDst->width, imgDst->height);
    break;

  default:
    break;
  }

}

void ImgResizeCrop(Image_t *imgSrc, Image_t *imgDst, ImgRect_t* roi, intrpl_t intrpl)
{
  IMG_ASSERT(intrpl == NEAREST || intrpl == BILINEAR);
  IMG_ASSERT(imgSrc->pData != NULL);
  IMG_ASSERT(imgSrc->format == imgDst->format);
  IMG_ASSERT(imgDst->pData != NULL);

  const uint32_t pixel_size = IMG_BYTES_PER_PX(imgSrc->format);

  switch (intrpl)
  {
  case NEAREST:
    ImageResize_NearestNeighbor(imgSrc->pData, imgSrc->width, imgSrc->height,
                                pixel_size, roi->x0, roi->y0, roi->width, roi->height,
                                imgDst->pData, imgDst->width, imgDst->height);
    break;

  case BILINEAR:
    ImageResize_Bilinear(imgSrc->pData, imgSrc->width, imgSrc->height,
                         pixel_size, roi->x0, roi->y0, roi->width, roi->height,
                         imgDst->pData, imgDst->width, imgDst->height);
    break;

  default:
    break;
  }

}

/**
* @brief  Performs image (or selected Region Of Interest) resizing using Nearest Neighbor interpolation algorithm
* @param  srcImage     Pointer to source image buffer
* @param  srcW         Source image width
* @param  srcH         Source image height
* @param  pixelSize    Number of bytes per pixel
* @param  roiX         Region Of Interest x starting location
* @param  roiY         Region Of Interest y starting location
* @param  roiW         Region Of Interest width
* @param  roiH         Region Of Interest height
* @param  dstImage     Pointer to destination image buffer
* @param  dstW         Destination image width
* @param  dstH         Destination image height
* @retval void         None
*/
#if 0
// 31,375 cycles (gcc 8.3.1 -Ofast) 128x128x3 --> 32x32x3
static void ImageResize_NearestNeighbor(uint8_t *srcImage, uint32_t srcW, uint32_t srcH,
                                        uint32_t pixelSize, uint32_t roiX, uint32_t roiY,
                                        uint32_t roiW, uint32_t roiH, uint8_t *dstImage,
                                        uint32_t dstW, uint32_t dstH)
{
  int x_ratio = (int)(((roiW ? roiW : srcW)<<16)/dstW)+1;
  int y_ratio = (int)(((roiH ? roiH : srcH)<<16)/dstH)+1;

  for (int y=0, i=0; y<dstH; y++)
  {
    int sy = (y*y_ratio)>>16;
    for (int x=0; x<dstW; x++, i+=pixelSize)
    {
      int sx = (x*x_ratio)>>16;

      for(int j=0; j<pixelSize; j++)
      {
        dstImage[i+j] = (uint8_t) srcImage[(((sy+roiY)*srcW) + (sx+roiX))*pixelSize + j];
      }
    }
  }
}
#else
static void ImageResize_NearestNeighbor(uint8_t *pIn, uint32_t srcW, uint32_t srcH,
                                        uint32_t pixelSize, uint32_t roiX, uint32_t roiY,
                                        uint32_t roiW, uint32_t roiH, uint8_t *pOut,
                                        uint32_t dstW, uint32_t dstH)
{
  const uint32_t x_ratio = (uint32_t) ((roiW << 16) / dstW) + 1;
  const uint32_t y_ratio = (uint32_t) ((roiH << 16) / dstH) + 1;

  for (uint32_t y = 0; y < dstH; y++) {
    uint32_t src_y = (((y * y_ratio) >> 16) + roiY) * srcW * pixelSize;

    for (uint32_t x = 0; x < dstW; x++) {
      uint32_t src_xy = (((x * x_ratio) >> 16) + roiX) * pixelSize + src_y;
      uint8_t *src_pixel = pIn + src_xy;

      for (uint32_t j = 0; j < pixelSize; j++) {
        *pOut++ = (uint8_t) *src_pixel++;
      }
    }
  }
}
#endif


void ImageResize_Bilinear(uint8_t *srcImage, uint32_t srcW, uint32_t srcH,
                          uint32_t pixelSize, uint32_t roiX, uint32_t roiY,
                          uint32_t roiW, uint32_t roiH,  uint8_t *dstImage,
                          uint32_t dstW, uint32_t dstH)
{
  int32_t srcStride;
  float widthRatio;
  float heightRatio;

  int32_t maxWidth;
  int32_t maxHeight;

  float srcX, srcY, dX1, dY1, dX2, dY2;
  int32_t dstX1, srcY1, dstX2, srcY2;

  uint8_t *tmp1, *tmp2;
  uint8_t *p1, *p2, *p3, *p4;

  int32_t offset1;
  int32_t offset2;

  srcStride = pixelSize * srcW;

  widthRatio = ((roiW ? roiW : srcW) / (float) dstW);
  heightRatio = ((roiH ? roiH : srcH) / (float) dstH);

  /* Get horizontal and vertical limits. */
  maxWidth = (roiW ? roiW : srcW) - 1;
  maxHeight = (roiH ? roiH : srcH) - 1;

  for (int32_t y = 0; y < dstH; y++)
  {
    /* Get Y from source. */
    srcY = ((float) y * heightRatio) + roiY;
    srcY1 = (int32_t) srcY;
    srcY2 = (srcY1 == maxHeight) ? srcY1 : srcY1 + 1;
    dY1 = srcY - (float) srcY1;
    dY2 = 1.0f - dY1;

    /* Calculates the pointers to the two needed lines of the source. */
    tmp1 = srcImage + srcY1 * srcStride;
    tmp2 = srcImage + srcY2 * srcStride;

    for (int32_t x = 0; x < dstW; x++)
    {
      /* Get X from source. */
      srcX = x * widthRatio + roiX;
      dstX1 = (int32_t) srcX;
      dstX2 = (dstX1 == maxWidth) ? dstX1 : dstX1 + 1;
      dX1 = srcX - /*(float32)*/dstX1;
      dX2 = 1.0f - dX1;

      /* Calculates the four points (p1,p2, p3, p4) of the source. */
      offset1 = dstX1 * pixelSize;
      offset2 = dstX2 * pixelSize;
      p1 = tmp1 + offset1;
      p2 = tmp1 + offset2;
      p3 = tmp2 + offset1;
      p4 = tmp2 + offset2;
      /* For each channel, interpolate the four points. */
      for (int32_t ch = 0; ch < pixelSize; ch++, dstImage++, p1++, p2++, p3++, p4++)
      {
        *dstImage = (uint8_t)(dY2 * (dX2 * (*p1) + dX1 * (*p2)) + dY1 * (dX2 * (*p3) + dX1 * (*p4)));
      }
    }
  }
}
