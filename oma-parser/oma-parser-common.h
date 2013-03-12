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
/*
 * @file		oma-parser-common.h
 * @brief		common header file for oma dd parser
 * @author		Jungki Kwak(jungki.kwak@samsung.com)
 */

#ifndef OMA_PARSER_COMMON_H_
#define OMA_PARSER_COMMON_H_

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <asm/unistd.h>

#define OP_CHAR_IS_SPACE(c)	((c) == ' ' || (c) == '\t')
#define OP_CHAR_IS_CRLT(c)	((c) == '\r' || (c) == '\n')

#define OP_NULL		0
#define OP_TRUE		1
#define OP_FALSE	0
#define OP_RESULT_OK	0

#define OP_MAX_URI_LEN			1024
#define OP_MAX_FILE_PATH_LEN	256
#define OP_MAX_STR_LEN			256
#define OP_MAX_MIME_STR_LEN		256

#define OP_ERR_INVALID_ARGUMENT     -100
#define OP_ERR_FAIL_TO_MEMALLOC     -110

#define OP_ERR_PARSE_FAIL       -200
#define OP_ERR_INVALID_MIME_TYPE    -210
#define OP_ERR_SERVER_NOTI_CODE_NOT_FOUND -220
#define OP_ERR_USER_RESPONSE_WAITING_TIME_OUT -230
#define OP_ERR_INVALID_ATTRIBUTE    -240

#ifdef DEBUG_USING_DLOG
#include "dlog.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "oma-parser"
#define OP_LOGI(format, ...) LOGV_IF("[%s:%d] "format" \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define OP_LOGD(format, ...) LOGD_IF("[%s:%d] "format" \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define OP_LOGE(format, ...) LOGE_IF("[%s:%d] "format" \n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else /* DEBUG_USING_DLOG */
#define OP_LOGI(format, ...) fprintf(stderr, "[oma-parser:V][%u][%s:%d] "format" \n", \
	(unsigned int)syscall(__NR_gettid), __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define OP_LOGD(format, ...) fprintf(stderr, "[oma-parser:D][%u][%s:%d] "format" \n", \
	(unsigned int)syscall(__NR_gettid), __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define OP_LOGE(format, ...) fprintf(stderr, "[oma-parser:E][%u][%s:%d] "format" \n", \
	(unsigned int)syscall(__NR_gettid), __FUNCTION__, __LINE__, ##__VA_ARGS__);
#endif /* DEBUG_USING_DLOG */

/**
 * Enumeration of Errors returned by OP-Parser Module
 */
typedef enum {
	OP_PARSER_ERR_NO_ERROR = 200,
	OP_PARSER_ERR_CREATE,
	OP_PARSER_ERR_INVALID_ARG,
	OP_PARSER_ERROR_FILE,
	OP_PARSER_ERR_INTERNAL_PARSING,
	OP_PARSER_ERR_MISSING_MANOPTORY_TAG,
	OP_PARSER_ERR_INVALID_VERSION_STRING,
	OP_PARSER_ERR_UNKNOWN_ELEMENT,
	OP_PARSER_ERR_XML_PARSER
} op_parser_err_t;

/**
 * User Data to be registered with parser library
 */
typedef struct _op_parser_app_data_t op_parser_app_data_t;
struct _op_parser_app_data_t {
	void *data;
	int element_index;
	int parseError;
};

/**
 * ampersand replacement table
 */
typedef struct _amp_character_table amp_character_table;
struct _amp_character_table {
	char *amp_str;
	char actual_ch;
};

int op_com_util_remove_blankspace(char *Source, int srcLen);
int op_com_util_resolve_version(char *source, int *major, int *minor,
                int *micro);
void op_com_utils_convert_amp_string(char *amp_string);
int op_com_utils_is_number(const char *source);
char *op_com_utils_concat(char *source, char *substring);

#endif /* OMA_PARSER_COMMON_H_ */
