/**
 ******************************************************************************
 * @file    stm32_fs.h
 * @author  MCD Application Team
 * @brief   File and image manipulation library built on top of FatFs
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
#ifndef __STM32_FS_H
#define __STM32_FS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>

#include "ff_gen_drv.h"
#include "sd_diskio.h"

/* Modes for counting files and directories */
#define STM32FS_COUNT_FILES (0x1)
#define STM32FS_COUNT_DIRS (0x2)

/* Modes for appending to file or not */
#define STM32FS_CREATE_NEW_FILE (0x0)
#define STM32FS_APPEND_TO_FILE (0x1)

/*! Error types */
typedef enum stm32fs_error
{
  STM32FS_ERROR_NONE = 0,
  STM32FS_ERROR_LINK_DRIVER_FAIL,
  STM32FS_ERROR_MOUNT_FS_FAIL,
  STM32FS_ERROR_FOPEN_FAIL,
  STM32FS_ERROR_FWRITE_FAIL,
  STM32FS_ERROR_FILE_NOT_SUPPORTED,
  STM32FS_ERROR_FREAD_FAIL,
  STM32FS_ERROR_FILE_READ_UNDERFLOW,
  STM32FS_ERROR_FILE_WRITE_UNDERFLOW,
  STM32FS_ERROR_DIR_NOT_FOUND,
  stm32fs_err_tOOMANY_DIRS
} stm32fs_err_t;

/* Functions prototypes */
stm32fs_err_t STM32Fs_Init(void);
stm32fs_err_t STM32Fs_DeInit(void);
stm32fs_err_t STM32Fs_WriteImageBMP(const char *path, uint8_t *buffer, const uint32_t width, const uint32_t height);
stm32fs_err_t STM32Fs_WriteImagePPM(const char *, uint8_t *, const uint32_t, const uint32_t);
stm32fs_err_t STM32Fs_GetImageInfoPPM(const char *path, uint32_t *width, uint32_t *height);
stm32fs_err_t STM32Fs_ReadImagePPM(const char *, uint8_t *, uint32_t *, uint32_t *);
stm32fs_err_t STM32Fs_GetNumberFiles(char *, uint32_t *, uint8_t);
stm32fs_err_t STM32Fs_OpenDir(char *, DIR *);
stm32fs_err_t STM32Fs_GetNextDir(DIR *, FILINFO *);
stm32fs_err_t STM32Fs_GetNextFile(DIR *, FILINFO *);
stm32fs_err_t STM32Fs_WriteTextToFile(char *, char *, int);
stm32fs_err_t STM32Fs_WriteRaw(const char *path, uint8_t *buffer, const size_t length);

#ifdef __cplusplus
} /*  extern "C" */
#endif

#endif  // __STM32_FS_H
