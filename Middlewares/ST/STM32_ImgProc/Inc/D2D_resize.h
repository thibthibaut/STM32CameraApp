/* File: D2D_resize.h

#########################################################################################
# # #         DMA2D bilinear bitmap resize (C)2015-2016 Alessandro Rocchegiani #
# #
#########################################################################################
*/

#ifndef __D2D_RESIZE_H
#define __D2D_RESIZE_H

/* #include "stm32h7xx.h" */
/* #include "stm32h7xx_dma2d.h" */
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_dma2d.h"

#define REPLACE_ALPHA_VALUE (0xF1)

typedef struct
{
  void *SourceBaseAddress;  /* source bitmap Base Address */
  uint16_t SourcePitch;     /* source pixel pitch */
  uint16_t SourceColorMode; /* source color mode */
  uint16_t SourceX;         /* souce X */
  uint16_t SourceY;         /* sourceY */
  uint16_t SourceWidth;     /* source width */
  uint16_t SourceHeight;    /* source height */
  void *OutputBaseAddress;  /* output bitmap Base Address */
  uint16_t OutputPitch;     /* output pixel pitch */
  uint16_t OutputColorMode; /* output color mode */
  uint16_t OutputX;         /* output X */
  uint16_t OutputY;         /* output Y */
  uint16_t OutputWidth;     /* output width */
  uint16_t OutputHeight;    /* output height */
  uint32_t *WorkBuffer;     /* storage buffer */
} RESIZE_InitTypedef;

/*
parameter notes:

The work buffer area (in the ARGB_8888 format for alignment purpose) must have at
least ( SourceWidth * OutputHeight ) word avaliable.

The source bitmap can be in any DMA2D input color format, the alfa channel of related
color mode is currently ignored. CM_L4 bitmap parameter must be byte aligned.
*/

typedef enum
{
  D2D_STAGE_IDLE=0,
  D2D_STAGE_FIRST_LOOP=1,
  D2D_STAGE_2ND_LOOP=2,
  D2D_STAGE_DONE=3,
  D2D_STAGE_ERROR=4,
  D2D_STAGE_SETUP_BUSY=5,
  D2D_STAGE_SETUP_DONE=6
}D2D_Stage_Typedef;


/* resize DMA2D_IRQHANDLER */
void D2D_DMA2D_IRQHandler(void);

/* resize setup */
D2D_Stage_Typedef D2D_Resize_Setup(RESIZE_InitTypedef* R);

/* resize callback */
__weak void D2D_Resize_Callback(D2D_Stage_Typedef D2D_Stage);

/* resize stage inquire */
D2D_Stage_Typedef D2D_Resize_Stage(void);

#endif

/* D2D_resize.h - END of File */
