/**
 * @file      app_storage.h
 * @author:   Shubhendu B B
 * @date:     28/09/2025
 * @brief
 * @details
 *
 * @copyright
 *
 **/
#ifndef _APP_STORAGE_H_
#define _APP_STORAGE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int app_storage_Init(const char *pBasePathSrtSz);
int app_storage_Deinit(char *pBasePathSrtSz);

#ifdef __cplusplus
}
#endif

#endif /* @end  _APP_STORAGE_H_*/