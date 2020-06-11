#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "display.h"
#include "stm32_img.h"

#include "microtrace.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h> /* STDOUT_FILENO, STDERR_FILENO */

// include "onboard_validation.h"

/*Defines related to cache settings*/
#define EXT_SDRAM_CACHE_ENABLED 1

#define LCD_BRIGHTNESS_MIN 0
#define LCD_BRIGHTNESS_MAX 100
#define LCD_BRIGHTNESS_MID 50
#define LCD_BRIGHTNESS_STEP 10

#define VGA_640_480_RES 1
#define QVGA_320_240_RES 2

#define CAMERA_CONTRAST_MIN CAMERA_CONTRAST_LEVEL0
#define CAMERA_CONTRAST_MAX CAMERA_CONTRAST_LEVEL4
#define CAMERA_BRIGHTNESS_MIN CAMERA_BRIGHTNESS_LEVEL0
#define CAMERA_BRIGHTNESS_MAX CAMERA_BRIGHTNESS_LEVEL4

#if CAMERA_CAPTURE_RES == VGA_640_480_RES
#define CAMERA_RESOLUTION RESOLUTION_R640x480
#define CAM_RES_WIDTH 640
#define CAM_RES_HEIGHT 480
#elif CAMERA_CAPTURE_RES == QVGA_320_240_RES
#define CAMERA_RESOLUTION RESOLUTION_R320x240
#define CAM_RES_WIDTH 320
#define CAM_RES_HEIGHT 240
#else
#error Please check defintion of CAMERA_CAPTURE_RES define
#endif

#endif /* __MAIN_H */
