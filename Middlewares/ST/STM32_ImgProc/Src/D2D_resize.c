/* File: D2D_resize.c

#########################################################################################
#                                                                                       #
#         DMA2D bilinear bitmap resize (C)2015-2016 Alessandro Rocchegiani              #
#                                                                                       #
#########################################################################################
*/

#include "D2D_resize.h"
#define CR_MASK     ((uint32_t)0xFFFCE0FC)  /* DMA2D CR Mask */

/* Setup FG/BG address and FGalpha for linear blend, start DMA2D */
void D2D_Blend_Line(void);

/* resize loop parameter */
typedef struct
{
  uint32_t Counter;           /* Loop Counter */
  uint32_t BaseAddress;       /* Loop Base Address */
  uint32_t BlendIndex;        /* Loop Blend index (Q21) */
  uint32_t BlendCoeff;        /* Loop Blend coefficient (Q21) */
  uint16_t SourcePitchBytes;  /* Loop Source pitch bytes */
  uint16_t OutputPitchBytes;  /* Loop Output pitch bytes */
}D2D_Loop_Typedef;  

/*Current resize stage*/
D2D_Stage_Typedef  D2D_Loop_Stage = D2D_STAGE_IDLE;

/*First loop parameter*/
D2D_Loop_Typedef   D2D_First_Loop;
/*2nd loop parameter*/
D2D_Loop_Typedef   D2D_2nd_Loop;

/*current parameter pointer*/
D2D_Loop_Typedef*  D2D_Loop = &D2D_First_Loop;


/* storage of misc. parameter for 2nd loop DMA2D register setup */
struct
{
  uint32_t OutputBaseAddress; /* output bitmap Base Address */
  uint16_t OutputColorMode;   /* output color mode */
  uint16_t OutputHeight;      /* output height */
  uint16_t OutputPitch;       /* output pixel pitch */
  uint16_t SourceWidth;       /* source width */ 
}D2D_Misc_Param;


/* 

D2D_Resize_Setup() Setup and start the resize process.
parameter:         RESIZE_InitTypedef structure  
return value:      D2D_STAGE_SETUP_DONE if process start or D2D_STAGE_SETUP_BUSY 
                   when a resize process already in progress

*/
D2D_Stage_Typedef D2D_Resize_Setup(RESIZE_InitTypedef* R)
{
  uint16_t PixelBytes,PitchBytes;
  uint32_t BaseAddress;
  
  const uint16_t BitsPerPixel[6]={32,24,16,16,16,8};

  /* Test for loop already in progress */
  if(D2D_Loop_Stage != D2D_STAGE_IDLE)
    return (D2D_STAGE_SETUP_BUSY);
  
  /* DMA2D operation mode */ 
	DMA2D->CR &= (uint32_t)CR_MASK;
  DMA2D->CR |= DMA2D_M2M_BLEND;
  
  /* DMA2D operation mode */ 
	DMA2D_ITConfig(DMA2D_IT_TC | DMA2D_IT_TE | DMA2D_IT_CE , ENABLE);
    
  /* first loop parameter init */
  PixelBytes                      = BitsPerPixel[R->SourceColorMode]>>3;
  PitchBytes                      = R->SourcePitch * PixelBytes;
  BaseAddress                     = (uint32_t)R->SourceBaseAddress + 
                                    R->SourceY * PitchBytes + R->SourceX * PixelBytes;

  D2D_First_Loop.Counter          = R->OutputHeight;
  D2D_First_Loop.SourcePitchBytes = PitchBytes;
  D2D_First_Loop.OutputPitchBytes = R->SourceWidth<<2;  
  D2D_First_Loop.BaseAddress      = BaseAddress;
  D2D_First_Loop.BlendCoeff       = ((R->SourceHeight-1)<<21) / R->OutputHeight;
  D2D_First_Loop.BlendIndex       = D2D_First_Loop.BlendCoeff>>1; 

  DMA2D->FGPFCCR                  = (REPLACE_ALPHA_VALUE<<16) | R->SourceColorMode; 
  DMA2D->BGPFCCR                  = (REPLACE_ALPHA_VALUE<<16) | R->SourceColorMode | 0xff000000; 
  DMA2D->OPFCCR                   = DMA2D_ARGB8888;
  DMA2D->NLR                      = (1 | (R->SourceWidth<<16));
  DMA2D->OMAR                     = (uint32_t)R->WorkBuffer;

  /* 2nd loop parameter init */
  PixelBytes                      = BitsPerPixel[R->OutputColorMode]>>3;
  PitchBytes                      = R->OutputPitch * PixelBytes;
  BaseAddress                     = (uint32_t)R->OutputBaseAddress +
                                      R->OutputY * PitchBytes + R->OutputX * PixelBytes;
    
  D2D_2nd_Loop.Counter            = R->OutputWidth;
  D2D_2nd_Loop.SourcePitchBytes   = 4;
  D2D_2nd_Loop.OutputPitchBytes   = PixelBytes;  
  D2D_2nd_Loop.BaseAddress        = (uint32_t)R->WorkBuffer;
  D2D_2nd_Loop.BlendCoeff         = ((R->SourceWidth-1)<<21) / R->OutputWidth;
  D2D_2nd_Loop.BlendIndex         = D2D_2nd_Loop.BlendCoeff>>1; 
  
  D2D_Misc_Param.OutputBaseAddress= BaseAddress;
  D2D_Misc_Param.OutputColorMode  = R->OutputColorMode;
  D2D_Misc_Param.OutputHeight     = R->OutputHeight;
  D2D_Misc_Param.OutputPitch      = R->OutputPitch;
  D2D_Misc_Param.SourceWidth      = R->SourceWidth;

  /* start first loop stage */
  D2D_Loop = &D2D_First_Loop;
  D2D_Loop_Stage = D2D_STAGE_FIRST_LOOP;
  
  D2D_Blend_Line();
  return(D2D_STAGE_SETUP_DONE);
}

/* Setup FG/BG address and FGalpha for linear blend, start DMA2D */
void D2D_Blend_Line(void)
{
  uint32_t FirstLine, FGalpha;

  /* Integer part of BlendIndex (Q21) is the first line number */
  FirstLine = D2D_Loop->BlendIndex>>21;
  /* calculate and setup address for first and 2nd lines */ 
  DMA2D->BGMAR = D2D_Loop->BaseAddress + FirstLine * D2D_Loop->SourcePitchBytes;
  DMA2D->FGMAR = DMA2D->BGMAR + D2D_Loop->SourcePitchBytes;
  /* 8 MSB of fractional part as FG alpha (Blend factor) */
  FGalpha = D2D_Loop->BlendIndex>>13;
  DMA2D->FGPFCCR &= 0x00ffffff;
  DMA2D->FGPFCCR |= (FGalpha<<24);
  /* restart DMA2D transfer*/
  DMA2D_StartTransfer();
}


void DMA2D_IRQHandler(void)
{
  /* Test on DMA2D Transfer Complete interrupt */
  if(DMA2D_GetITStatus(DMA2D_IT_TC)) 
  {
    /* Clear DMA2D Transfer Complete interrupt Flag */
    DMA2D_ClearITPendingBit(DMA2D_IT_TC);
    /* Test for loop in progress */
    if (D2D_Loop_Stage != D2D_STAGE_IDLE)
    {
      /* decrement loop counter and if != 0 process loop row*/ 
      if(--D2D_Loop->Counter)
      { 
        /* Update output memory address */
        DMA2D->OMAR += D2D_Loop->OutputPitchBytes;
        /* Add BlenCoeff to BlendIndex */
        D2D_Loop->BlendIndex+=D2D_Loop->BlendCoeff;
        /* Setup FG/BG address and FGalpha for linear blend, start DMA2D */
        D2D_Blend_Line();
      }
      else
      {
        /* else test for current D2D Loop stage */
        if(D2D_Loop_Stage == D2D_STAGE_FIRST_LOOP)
        {
          /* setup DMA2D register */
          DMA2D->FGPFCCR = (REPLACE_ALPHA_VALUE<<16) | CM_ARGB8888;
          DMA2D->BGPFCCR = (REPLACE_ALPHA_VALUE<<16) | CM_ARGB8888 | 0xff000000;
          DMA2D->OPFCCR  = D2D_Misc_Param.OutputColorMode;
          DMA2D->NLR     = (D2D_Misc_Param.OutputHeight | (1<<16));
          DMA2D->OOR     = D2D_Misc_Param.OutputPitch-1;
          DMA2D->FGOR    = D2D_Misc_Param.SourceWidth-1;
          DMA2D->BGOR    = D2D_Misc_Param.SourceWidth-1;
          DMA2D->OMAR    = D2D_Misc_Param.OutputBaseAddress;
          /* start 2nd loop stage */
          D2D_Loop = &D2D_2nd_Loop;
          D2D_Loop_Stage = D2D_STAGE_2ND_LOOP;
          /* Setup FG/BG address and FGalpha for linear blend, start DMA2D */
          D2D_Blend_Line();
        }
        else
        {
          /* else resize complete */
          D2D_Resize_Callback(D2D_STAGE_DONE);
          /* reset to idle stage */
          D2D_Loop_Stage = D2D_STAGE_IDLE;
        }
      }
    }
    else
    {
      /* add here other DMA2d TC Irq stuff */
    }
  }
  else
  {
    /* Test on DMA2D Transfer Error interrupt */
    if(DMA2D_GetITStatus(DMA2D_IT_TE))
    {
      /* Clear DMA2D Transfer Error interrupt Flag */
      DMA2D_ClearITPendingBit(DMA2D_IT_TE);
      /* Test for resize loop in progress */
      if (D2D_Loop_Stage != D2D_STAGE_IDLE)
      { 
        /* resize error callback */
        D2D_Resize_Callback(D2D_STAGE_ERROR);
        /* reset to IDLE stage */        
        D2D_Loop_Stage = D2D_STAGE_IDLE;
      }
      else
      {
        /* add here other DMA2D TE Irq stuff */
      }
    }
  }
} 

D2D_Stage_Typedef D2D_Resize_Stage(void)
{
  return (D2D_Loop_Stage);
}


__weak void D2D_Resize_Callback(D2D_Stage_Typedef D2D_Stage)
{
  /* Halt on DMA2D Transfer error */
  while( D2D_Stage == D2D_STAGE_ERROR);
}

/* D2D_resize.c - END of File */
