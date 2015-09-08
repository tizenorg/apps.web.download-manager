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
/*
 * @file		oma-parser-interface.c
 * @brief		interface function for oma dd parser
 * @author		Jungki Kwak(jungki.kwak@samsung.com)
 */

#include <expat.h>
#include <string.h>
#include <stdio.h>

#include "oma-parser-interface.h"

static int op_check_mandatory_tags(op_parser_app_data_t *app_data);

void op_parser_init(void)
{
	xmlInitParser();
}

void op_parser_deinit(void)
{
	xmlCleanupParser();
}

int op_parse_dd_file(
        const char *dd_file_path,
        void **dd_info,
        int *error_code)
{
	int b_ret = OP_TRUE;
	op_parser_app_data_t *app_data = OP_NULL;
	int ret_code = OP_RESULT_OK;

	OP_LOGD("");

	if (dd_file_path == OP_NULL || strpbrk(dd_file_path, OP_INVALID_PATH_STRING) != NULL ||
		error_code == OP_NULL || dd_info == OP_NULL) {

		OP_LOGE("OP INVALID ARGUMENT");
		if (error_code)
			*error_code = OP_PARSER_ERR_INVALID_ARG;

		b_ret = OP_FALSE;
		return b_ret;
	}

	*error_code = OP_PARSER_ERR_NO_ERROR;
	*dd_info = OP_NULL;

	xmlSAXHandler *sHandlerPtr = calloc(1, sizeof(xmlSAXHandler));
	if (sHandlerPtr == OP_NULL) {
		OP_LOGE("OP NULL RETURN");
		*error_code = OP_PARSER_ERR_CREATE;
		b_ret = OP_FALSE;
		goto ERR;
	}

	ret_code = op_parse_dd1_file(sHandlerPtr, &app_data);
	if (OP_RESULT_OK == ret_code) {
		*dd_info = (void *)(((dd1_cntx *)(app_data->data))->dd1);
	} else {
		*error_code = ret_code;
		b_ret = OP_FALSE;
		goto ERR;
	}

	if (xmlSAXUserParseFile(sHandlerPtr, app_data, dd_file_path) < 0) {
		OP_LOGD("Error : return value less than zero");
		*error_code = app_data->parseError;
		b_ret = OP_FALSE;
		goto ERR;
	}

	if (OP_RESULT_OK != app_data->parseError) {
		*error_code = app_data->parseError;
		b_ret = OP_FALSE;
		goto ERR;
	}

	if (OP_FALSE == (b_ret = op_check_mandatory_tags(app_data))) {
		OP_LOGE("Parse error happened. Mandatory Element Missing");
		*error_code = OP_PARSER_ERR_MISSING_MANOPTORY_TAG;
		goto ERR;
	}

	OP_LOGD("DD1 prasing is Successful");
ERR:
	if (app_data) {
		free(app_data->data);
		free(app_data);
	}
	free(sHandlerPtr);

	return b_ret;
}

void op_free_dd_info(void *dd_info)
{
	op_free_dd1_info((dd_oma1_t*)dd_info);
}

int op_check_mandatory_tags(op_parser_app_data_t *app_data)
{
	int ret = OP_FALSE;

	OP_LOGD("");

	ret = op_check_dd1_mandatory_tags(app_data);

	return ret;
}
