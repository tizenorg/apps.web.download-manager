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

#define BUFFSIZE	4096

static int op_check_mandatory_tags(XML_Parser parser);

int op_parse_dd_file(
        const char *dd_file_path,
        void **dd_info,
        int *error_code)
{
	int b_ret = OP_TRUE;
	FILE *fp = OP_NULL;
	XML_Parser parser = OP_NULL;
	char buff[BUFFSIZE] = {0, };
	op_parser_app_data_t *app_data = OP_NULL;
	dd_oma1_t *dd = OP_NULL;
	int ret_code = OP_RESULT_OK;

	OP_LOGI("");

	if (dd_file_path == OP_NULL || error_code == OP_NULL ||
		dd_info == OP_NULL) {

		OP_LOGE("OP INVALID ARGUMENT");
		if (error_code)
			*error_code = OP_PARSER_ERR_INVALID_ARG;

		b_ret = OP_FALSE;
		return b_ret;
	}

	*error_code = OP_PARSER_ERR_NO_ERROR;
	*dd_info = OP_NULL;

	fp = fopen(dd_file_path, "r");
	if (fp == OP_NULL) {
		OP_LOGE("op_parse_dd_file: Given file can not be opened \n");
		*error_code = OP_PARSER_ERROR_FILE;
		b_ret = OP_FALSE;
		goto ERR;
	}

	parser = XML_ParserCreate(NULL);
	if (parser == OP_NULL) {
		OP_LOGE("OP NULL RETURN");
		*error_code = OP_PARSER_ERR_CREATE;
		b_ret = OP_FALSE;
		goto ERR;
	}

	ret_code = op_parse_dd1_file(parser, &dd);
	if (OP_RESULT_OK == ret_code) {
		*dd_info = (void*)dd;
	} else {
		*error_code = ret_code;
		b_ret = OP_FALSE;
		goto ERR;
	}

	app_data = (op_parser_app_data_t*)XML_GetUserData(parser);

	for (;;) {
		int last_invokation = 0;
		int len = 0;

		len = fread(buff, 1, BUFFSIZE, fp);
		last_invokation = feof(fp); /* len < BUFFSIZE; */

		if (XML_Parse(parser, buff, len, last_invokation)
		        == XML_STATUS_ERROR) {/* In the last call, "last_invokation" must be 1, "len" can be zero for it. */
			OP_LOGE("Parse error at line[%ld], Error : [%s]\n", XML_GetCurrentLineNumber(parser), XML_ErrorString(XML_GetErrorCode(parser)));
			*error_code = OP_PARSER_ERR_INTERNAL_PARSING;
			b_ret = OP_FALSE;
			goto ERR;
		}

		if (app_data) {
			if (OP_RESULT_OK != app_data->parseError) {
				*error_code = app_data->parseError;
				b_ret = OP_FALSE;
				goto ERR;
			}
		}
		if (last_invokation || len == 0)
			break;
	}

	if (OP_FALSE == (b_ret = op_check_mandatory_tags(parser))) {
		OP_LOGE("Parse error happened. Mandatory Element Missing\n");
		*error_code = OP_PARSER_ERR_MISSING_MANOPTORY_TAG;
		goto ERR;
	}

	OP_LOGI("DD1 prasing is Successful");
ERR:
	if (OP_FALSE == b_ret) {
		if (OP_NULL != *dd_info) {
			op_free_dd1_info(*dd_info);
		}
	}

	if (fp)
		fclose(fp);

	if (parser)
		XML_ParserFree(parser);

	return b_ret;
}

void op_free_dd_info(void *dd_info)
{
	op_free_dd1_info((dd_oma1_t*)dd_info);
}

int op_check_mandatory_tags(XML_Parser parser)
{
	int ret = OP_FALSE;

	OP_LOGI("");

	ret = op_check_dd1_mandatory_tags(parser);

	return ret;
}
