/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file 	download-manager-debug.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	debug function
 */

#ifndef DOWNLOAD_MANAGER_DEBUG_H
#define DOWNLOAD_MANAGER_DEBUG_H

#define _USE_DLOG 1

#ifdef _USE_DLOG
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "DOWNLOAD_MANAGER"


#define DM_LOGD(format, ...) LOGD(format, ##__VA_ARGS__)
#define DM_LOGI(format, ...) LOGI(format, ##__VA_ARGS__)
#define DM_LOGE(format, ...) LOGE(format, ##__VA_ARGS__)
#define DM_SLOGD(format, ...) LOGD(format, ##__VA_ARGS__)
#define DM_SLOGI(format, ...) LOGI(format, ##__VA_ARGS__)
#define DM_SLOGE(format, ...) LOGE(format, ##__VA_ARGS__)

#else

#include <stdio.h>
#include <pthread.h>

#define DM_LOGD(args...) do {\
		printf("[D:%s][LN:%d][%lu]",__func__,__LINE__,pthread_self());\
		printf(args);printf("\n");}while(0)
#define DM_LOGI(args...) do {\
		printf("[I:%s][LN:%d][%lu]",__func__,__LINE__,pthread_self());\
		printf(args);printf("\n");}while(0)
#define DM_LOGE(args...) do {\
		printf("[ERR:%s][LN:%d][%lu]",__func__,__LINE__,pthread_self());\
		printf(args);printf("\n");}while(0)
#define DM_SLOGD DM_LOGD
#define DM_SLOGI DM_LOGI
#define DM_SLOGE DM_LOGE
#endif /*_USE_DLOG*/

#endif /* DOWNLOAD_MANAGER_DEBUG_H */
