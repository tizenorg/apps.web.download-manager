/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
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

#define DP_LOGD(format, ...) LOGI(format, ##__VA_ARGS__)
#define DP_LOGV(format, ...) LOGD(format, ##__VA_ARGS__)
#define DP_LOG_START(msg) LOGI("<<Start>>[%s]",msg)
#define DP_LOGD_FUNC() LOGI("<<Check>>")
#define DP_LOGV_FUNC() LOGD("<<Check>>")
#define DP_LOGE(format, ...) LOGE("[ERR] "format, ##__VA_ARGS__)

#else

#include <stdio.h>
#include <pthread.h>

#define DP_LOGD(args...) do {\
		printf("[DP_D:%s][LN:%d][%lu]",__func__,__LINE__,pthread_self());\
		printf(args);printf("\n");}while(0)
#define DP_LOGE(args...) do {\
		printf("[DP_ERR:%s][LN:%d][%lu]",__func__,__LINE__,pthread_self());\
		printf(args);printf("\n");}while(0)
#define DP_LOGD_FUNC() do {\
		printf("<<==[DP:%s][LN:%d][%lu] ==>> \n",__func__,__LINE__,pthread_self());\
		}while(0)
#define DP_LOGV_FUNC() do {\
		printf("<<==[DP_D:%s][LN:%d][%lu] ==>> \n",__func__,__LINE__,pthread_self());\
		}while(0)
#define DP_LOG_START(msg) do {\
		printf("<<==[DP:%s][LN:%d][%lu] Start ==>> \n",\
		__FUNCTION__,__LINE__,pthread_self());\
		}while(0)
#endif /*_USE_DLOG*/

#endif /* DOWNLOAD_MANAGER_DEBUG_H */
