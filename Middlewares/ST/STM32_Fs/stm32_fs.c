/**
 ******************************************************************************
 * @file    stm32_fs.c
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
#include "stm32_fs.h"

/** @addtogroup Middlewares
  * @{
  */

/** @addtogroup STM32_Fs
  * @{
  */
  
/* File system */
FATFS SDFatFS;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */
char SDPath[4]; /* SD card logical drive path */

/* Private function prototypes -----------------------------------------------*/
static void STM32Fs_GetDimsFromString(char *string, uint32_t *width, uint32_t *height);

/**
 * @brief Initialize STM32Fs Library by linking FatFS Driver and mounting file system
 *
 * @warning The BSP_SD_Init() must be called before this function
 * @return stm32fs_err_t - Error message, can be one of NONE, LINK_DRIVER_FAIL, MOUNT_FS_FAIL
 */
stm32fs_err_t STM32Fs_Init(void)
{
  stm32fs_err_t ret = STM32FS_ERROR_NONE;
  if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
  {
    ret = STM32FS_ERROR_LINK_DRIVER_FAIL;
  }

  if (f_mount(&SDFatFS, (TCHAR const *)SDPath, 0) != FR_OK)
  {
    return STM32FS_ERROR_MOUNT_FS_FAIL;
  }

  return ret;
}

/**
 * @brief Deinitialize STM32Fs library by unlinking driver
 *
 * @return stm32fs_err_t Error message, one of NONE or LINK_DRIVER_FAIL
 */
stm32fs_err_t STM32Fs_DeInit(void)
{
  f_mount(0, "", 0);
  stm32fs_err_t ret = STM32FS_ERROR_NONE;
  if (FATFS_UnLinkDriver(SDPath) != 0)
  {
    ret = STM32FS_ERROR_LINK_DRIVER_FAIL;
  }
  return ret;
}

/**
 * @brief Writes an RGB image to file system in PPM (P6) format
 *
 * @param path[in] null-terminated string correspondig to the path of the file to be written
 * @param buffer[in] buffer containing image data to be written in RGB888 format
 * @param width[in] width of the image
 * @param height[in] height of the image
 * @return stm32fs_err_t Error type, can be one of NONE, FOPEN_FAIL, FWRITE_FAIL
 */
stm32fs_err_t STM32Fs_WriteImagePPM(const char *path, uint8_t *buffer, const uint32_t width, const uint32_t height)
{
  FIL File;
  /* Mount */
  uint32_t byteswritten; /* File write/read counts */

  /* Fopen */
  if (f_open(&File, path, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
  {
    return STM32FS_ERROR_FOPEN_FAIL;
  }

  /* Header data */
  char header[32];
  sprintf(header, "P6\n%d %d\n255\n", (unsigned int)width, (unsigned int)height);

  /* Write header to a new file */
  FRESULT res = f_write(&File, (uint8_t *)header, strlen(header), (void *)&byteswritten);

  /* Append image data to the file */
  uint32_t buffer_size = width * height * 3;

  uint8_t *ptr = buffer;

  __disable_irq();
  res = f_write(&File, ptr, buffer_size, (void *)&byteswritten);
  __enable_irq();

  f_close(&File);

  if (res != 0)
  {
    return STM32FS_ERROR_FWRITE_FAIL;
  }

  return STM32FS_ERROR_NONE;
}

/**
 * @brief Reads the images informations (widht and height) from a PPM image
 *
 * @param path[in] Absolute path to the image file
 * @param width[out] width of the image
 * @param height[out] height of the image
 *
 * @return Error type, one of FOPEN_FAIL, FILE_NOT_SUPPORTED or NONE
 */
stm32fs_err_t STM32Fs_GetImageInfoPPM(const char *path, uint32_t *width, uint32_t *height)
{
  FIL File;

  /* Fopen */
  if (f_open(&File, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
  {
    return STM32FS_ERROR_FOPEN_FAIL;
  }

  char line_buffer[128];

  f_gets(line_buffer, 128, &File);
  if (line_buffer[0] != 'P' || line_buffer[1] != '6')
  {
    return STM32FS_ERROR_FILE_NOT_SUPPORTED;
  }

  /* ignore comments if any */
  do
  {
    f_gets(line_buffer, 128, &File);
  } while (line_buffer[0] == '#');

  STM32Fs_GetDimsFromString(line_buffer, width, height);

  f_close(&File);

  return STM32FS_ERROR_NONE;
}

/**
 * @brief Reads an image from SDCard. The image should be in the PPM (P6) format
 *
 * @param path[in]          Path to the image to read
 * @param out_buffer[out]   Pointer to the image data to be written
 * @param width[out]        Width of the image
 * @param height[out]       Height of the image
 * @return stm32fs_err_t    Error message, one of NONE, FOPEN_FAIL, FILE_NOT_SUPPORTED
 */
stm32fs_err_t STM32Fs_ReadImagePPM(const char *path, uint8_t *out_buffer, uint32_t *width, uint32_t *height)
{
  FIL File;
  uint32_t bytesread; /* File read counts */

  /* Fopen */
  if (f_open(&File, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
  {
    return STM32FS_ERROR_FOPEN_FAIL;
  }

  char line_buffer[128];

  f_gets(line_buffer, 128, &File);
  if (line_buffer[0] != 'P' || line_buffer[1] != '6')
  {
    return STM32FS_ERROR_FILE_NOT_SUPPORTED;
  }

  /* ignore comments if any */
  do
  {
    f_gets(line_buffer, 128, &File);
  } while (line_buffer[0] == '#');

  STM32Fs_GetDimsFromString(line_buffer, width, height);

  /* Get dynamic (should be 255) */
  f_gets(line_buffer, 128, &File);

  if (line_buffer[0] != '2' || line_buffer[1] != '5' || line_buffer[2] != '5')
  {
    return STM32FS_ERROR_FILE_NOT_SUPPORTED;
  }

  uint32_t buffer_size = (*width) * (*height) * 3;

  if (f_read(&File, out_buffer, buffer_size, (unsigned int *)&bytesread) != FR_OK)
  {
    return STM32FS_ERROR_FREAD_FAIL;
  }
  if (bytesread < buffer_size)
  {
    return STM32FS_ERROR_FILE_READ_UNDERFLOW;
  }

  f_close(&File);

  return STM32FS_ERROR_NONE;
}

/**
 * @brief Utility function when working with PPM P6 format.
 *        Gets dimensions of image from header data
 *
 * @param string[in] String data from the header
 * @param width[out] Width of the image
 * @param height[out] Height of the image
 */
static void STM32Fs_GetDimsFromString(char *string, uint32_t *width, uint32_t *height)
{
  char dim_str[2][8];              /* holds widht/height as string */
  uint32_t dim_int[2];             /* holds widht/height as unsigned integer */
  char separator[2] = {' ', '\n'}; /* string is in the form char-char-space-char-char-\n */

  char *src = string;
  for (uint32_t i = 0; i < 2; i++)
  { /* for width, height */

    /* Copy source string into dim_str[i] until separator */
    char *dst = dim_str[i];
    while (*src != separator[i])
    {
      *dst = *src;
      src++;
      dst++;
    }

    /* Go through each char to build the number */
    uint32_t base = 1;
    dim_int[i] = 0;
    while (dst-- != dim_str[i])
    {
      dim_int[i] += (*dst - '0') * base;
      base *= 10;
    }
    src++; /* Increment src pointer to skip the first separator */
  }

  *width = dim_int[0];
  *height = dim_int[1];
}

/**
 * @brief Get number of files in a directory
 *
 * @param path[in]              Path to the directory
 * @param nbr[out]              Number of files/directories in this directory
 * @param mode[in]              STM32FS_COUNT_DIRS and STM32FS_COUNT_FILES flags can be use
 *                              with bitwise OR to choose either to count files, directories or both
 * @return stm32fs_err_t        Error status, can be one of NONE or DIR_NOT_FOUND
 */
stm32fs_err_t STM32Fs_GetNumberFiles(char *path, uint32_t *nbr, uint8_t mode)
{
  FRESULT res;
  DIR dir;
  static FILINFO fno;

  uint32_t dir_counter = 0;
  uint32_t file_counter = 0;

  res = f_opendir(&dir, path); /* Open the directory */
  if (res == FR_OK)
  {
    for (;;)
    {
      res = f_readdir(&dir, &fno);                  /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break; /* Break on error or end of dir */
      if (fno.fattrib & AM_DIR)
      { /* It is a directory */
        dir_counter++;
      }
      else
      { /* It is a file */
        file_counter++;
      }
    } /* End for */

    f_closedir(&dir);
  }
  else
  {
    return STM32FS_ERROR_DIR_NOT_FOUND;
  }

  *nbr = 0;
  if (mode & STM32FS_COUNT_DIRS)
  {
    *nbr += dir_counter;
  }
  if (mode & STM32FS_COUNT_FILES)
  {
    *nbr += file_counter;
  }
  return STM32FS_ERROR_NONE;
}

/**
 * @brief Open a directory
 *
 * @param path[in] path in the filesystem
 * @param dir[out] pointer to the open directory
 * @return stm32fs_err_t Error code, one of NONE or DIR_NOT_FOUND
 */
stm32fs_err_t STM32Fs_OpenDir(char *path, DIR *dir)
{
  FRESULT res;
  res = f_opendir(dir, path);
  if (res != FR_OK)
  {
    return STM32FS_ERROR_DIR_NOT_FOUND;
  }

  return STM32FS_ERROR_NONE;
}

/**
 * @brief Iterates over the directories inside a directory in a filesystem.
 * returns DIR_NOT_FOUND when there are no more directories.
 *
 * @param dir[in] pointer to an opened directory. This directory will be searched for directories
 * @param fno[out] pointer to FILEINFO struct containing the information about the next directory inside dir
 * @return stm32fs_err_t Error code, DIR_NOT_FOUND when all directories have been listed or NONE
 */
stm32fs_err_t STM32Fs_GetNextDir(DIR *dir, FILINFO *fno)
{
  FRESULT res;

  res = f_readdir(dir, fno);                                                  /* Read a directory item */
  if (res != FR_OK || fno->fname[0] == 0) return STM32FS_ERROR_DIR_NOT_FOUND; /* Break on error or end of dir */
  if (fno->fattrib & AM_DIR)
  { /* It is a directory */
    return STM32FS_ERROR_NONE;
  }
  return STM32FS_ERROR_DIR_NOT_FOUND; /* Break on error or end of dir */
}

/**
 * @brief Iterates over the files inside a directory in a filesystem.
 * returns DIR_NOT_FOUND when there are no more files.
 *
 * @param dir[in] pointer to an opened directory. This directory will be searched for directories
 * @param fno[out] pointer to FILEINFO struct containing the information about the next file inside dir
 * @return stm32fs_err_t Error code, DIR_NOT_FOUND when all directories have been listed or NONE
 */
stm32fs_err_t STM32Fs_GetNextFile(DIR *dir, FILINFO *fno)
{
  FRESULT res;

  res = f_readdir(dir, fno);                                                  /* Read a directory item */
  if (res != FR_OK || fno->fname[0] == 0) return STM32FS_ERROR_DIR_NOT_FOUND; /* Break on error or end of dir */
  if (!(fno->fattrib & AM_DIR))
  { /* It is a file */
    return STM32FS_ERROR_NONE;
  }
  return STM32FS_ERROR_DIR_NOT_FOUND; /* Break on error or end of dir */
}

/**
 * @brief Writes a raw buffer in a file on the filesystem
 *
 * @param path[in] path where to write the file
 * @param buffer[in] pointer to the data
 * @param length[in] lenght of the data in bytes
 * @return stm32fs_err_t Error code, one of FOPEN_FAIL, FILE_WRITE_UNDERFLOW, NONE
 */
stm32fs_err_t STM32Fs_WriteRaw(const char *path, uint8_t *buffer, const size_t length)
{
  FIL File;
  int byteswritten;

  if (f_open(&File, path, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
  {
    return STM32FS_ERROR_FOPEN_FAIL;
  }

  __disable_irq();
  f_write(&File, buffer, length, (void *)&byteswritten);
  __enable_irq();

  f_close(&File);

  if (byteswritten != length)
  {
    return STM32FS_ERROR_FILE_WRITE_UNDERFLOW;
  }

  return STM32FS_ERROR_NONE;
}

/**
 * @brief Write an image as Bitmap to filesystem
 *
 * @param path[in] path in the filesystem
 * @param buffer[in] pointer to the RGB888 image data
 * @param width[in] width of the image in pixels
 * @param height[in] height of the image in pixels
 * @return stm32fs_err_t error code
 */
stm32fs_err_t STM32Fs_WriteImageBMP(const char *path, uint8_t *buffer, const uint32_t width, const uint32_t height)
{
  FIL File;
  int byteswritten;

  if (f_open(&File, path, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
  {
    return STM32FS_ERROR_FOPEN_FAIL;
  }

  static unsigned char header[54] = {66, 77, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40,
                                     0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 0, 24}; /* rest is zeroes */
  unsigned int pixelBytesPerRow = width * 3;
  unsigned int paddingBytesPerRow = (4 - (pixelBytesPerRow % 4)) % 4;
  unsigned int *sizeOfFileEntry = (unsigned int *)&header[2];
  *sizeOfFileEntry = 54 + (pixelBytesPerRow + paddingBytesPerRow) * height;
  unsigned int *widthEntry = (unsigned int *)&header[18];
  *widthEntry = width;
  unsigned int *heightEntry = (unsigned int *)&header[22];
  *heightEntry = -height; /* '-' required so to avoid having to rotate the image when opening .bmp file*/
  static unsigned char zeroes[3] = {0, 0, 0}; /* for padding */

  __disable_irq();
  f_write(&File, header, 54, (void *)&byteswritten);

  if (width % 4 == 0)
  {
    f_write(&File, buffer, width * height * 3, (void *)&byteswritten);
  }
  else
  { /* Padding is necessary when row is not a multiple of 4*/
    for (int row = 0; row < height; row++)
    {
      f_write(&File, buffer + 3 * width * row, pixelBytesPerRow, (void *)&byteswritten);
      f_write(&File, zeroes, paddingBytesPerRow, (void *)&byteswritten);
    }
  }
  __enable_irq();

  f_close(&File);

  return STM32FS_ERROR_NONE;
}

/**
 * @brief Writes a text file to filesystem
 *
 * @param path[in] path in the filesystem
 * @param content[in] pointer to a null-terminated string
 * @param append_to_file[in] flag to choose bewteen erasing an existing file or appending to it. Either
 * STM32FS_CREATE_NEW_FILE or STM32FS_APPEND_TO_FILE
 * @return stm32fs_err_t
 */
stm32fs_err_t STM32Fs_WriteTextToFile(char *path, char *content, int append_to_file)
{
  FIL File;
  uint32_t byteswritten; /* File write/read counts */

  /* Open the file */
  BYTE flags = FA_WRITE;
  if (append_to_file == STM32FS_APPEND_TO_FILE)
  {
    flags |= FA_OPEN_APPEND;
  }
  else
  {
    flags |= FA_CREATE_ALWAYS;
  }

  if (f_open(&File, path, flags) != FR_OK)
  {
    return STM32FS_ERROR_FOPEN_FAIL;
  }

  FRESULT res = f_write(&File, (uint8_t *)content, strlen(content), (void *)&byteswritten);
  if (res != FR_OK)
  {
    return STM32FS_ERROR_FWRITE_FAIL;
  }

  if (byteswritten != strlen(content))
  {
    return STM32FS_ERROR_FILE_WRITE_UNDERFLOW;
  }

  f_close(&File);

  return STM32FS_ERROR_NONE;
}

/**
  * @}
  */

/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
