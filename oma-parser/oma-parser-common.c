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
 * @file		oma-parser-common.c
 * @brief		common function for oma dd parser
 * @author		Jungki Kwak(jungki.kwak@samsung.com)
 *
 */

#include <stdlib.h>
#include <string.h>
#include "oma-parser-common.h"

static amp_character_table amp_table[] = {
		{"&amp;", '&'},
		{"&lt;", '<'},
		{"&gt;", '>'},
		{"&quot;", '\"'},
		{"&apos;", '\''},
		{OP_NULL, ' '}};

int op_com_util_remove_blankspace(char *Source, int srcLen)
{
	int ret = OP_RESULT_OK;
	int i = 0;
	int j = 0;
	int spaceFound = OP_FALSE;
	char *target = Source;
	char *Stripped = Source;

	OP_SLOG("Stripped String:[%s],len[%d]", Source, srcLen);

	if (OP_NULL == Source && srcLen > i) {
		ret = OP_ERR_INVALID_ARGUMENT;
		goto ERR;
	}

	while ('\0' != target[i] && i < srcLen) {
		if (OP_CHAR_IS_SPACE(target[i]) || OP_CHAR_IS_CRLT(target[i])) {
			spaceFound = OP_TRUE;
		} else {
			if (spaceFound)
				Stripped[j] = target[i];
			j++;
		}
		i++;
	}

	if (spaceFound)
		Stripped[j] = '\0';

ERR:
	OP_SLOG("Stripped String : [%s]", Source);
	return ret;
}

int op_com_util_resolve_version(char *source, int *major, int *minor,
                int *micro)
{
	int ret = OP_RESULT_OK;
	int ma = 0;
	int mi = 0;
	int mc = 0;
	char *Ch = NULL;
	char *pDotCh = NULL;

	OP_LOGD("");

	/*For major version*/
	for (Ch = source; *Ch != '.' && *Ch; Ch++) {
		if (*Ch == ' ')
			continue;

		if (*Ch >= '0' && *Ch <= '9') {
			ma *= 10;
			ma += *Ch - '0';
		} else {
			ret = OP_PARSER_ERR_INVALID_VERSION_STRING;
			goto ERR;
		}
	}

	if (*Ch != '.') {
		ret = OP_PARSER_ERR_INVALID_VERSION_STRING;
		goto ERR;
	}

	pDotCh = Ch;

	/* For minor version. */
	for (Ch = pDotCh + 1; *Ch != '.' && *Ch; Ch++) {
		if (*Ch == ' ')
			continue;

		if (*Ch >= '0' && *Ch <= '9') {
			mi *= 10;
			mi += *Ch - '0';
		} else {
			ret = OP_PARSER_ERR_INVALID_VERSION_STRING;
			goto ERR;
		}
	}

	/*micro version; if it exists.*/
	if (*Ch == '.' && micro) {
		pDotCh = Ch;
		for (Ch = pDotCh + 1; *Ch; Ch++) {
			if (*Ch == ' ')
				continue;

			if (*Ch >= '0' && *Ch <= '9') {
				mc *= 10;
				mc += *Ch - '0';
			} else {
				ret = OP_PARSER_ERR_INVALID_VERSION_STRING;
				goto ERR;
			}
		}
	}

	*major = ma;
	*minor = mi;

	if (micro)
		*micro = mc;

ERR:
	return ret;
}

void op_com_utils_convert_amp_string(char *amp_string)
{
	char *matched_str = NULL;
	int i = 0;

	OP_LOGD("");

	if (amp_string == OP_NULL || strlen(amp_string) <= 0) {
		OP_LOGE("Invalid Argument");
		return;
	}

	for (i = 0; OP_NULL != amp_table[i].amp_str; i++) {
		int len = strlen(amp_table[i].amp_str);
		char *trailing_str_ptr = amp_string;
		char *trailing_end_ptr = NULL;

		while (*trailing_str_ptr &&
				(matched_str = strstr(trailing_str_ptr, amp_table[i].amp_str))
					!= OP_NULL) {
			*matched_str = amp_table[i].actual_ch;
			matched_str++;
			trailing_str_ptr = matched_str;
			trailing_end_ptr = matched_str + len - 1;
			while (*trailing_end_ptr) {
				*matched_str++ = *trailing_end_ptr++;
			}
			*matched_str = '\0';
		}
	}

	OP_SLOG("amp_string[%s]", amp_string);
	return;
}

int op_com_utils_is_number(const char *source)
{
	const char *pCh;

	OP_LOGD("");

	pCh = source;

	while (*pCh) {
		if (*pCh < '0' || *pCh > '9')
			return OP_FALSE;

		pCh++;
	}

	return OP_TRUE;
}

char *op_com_utils_concat(char *source, char *substring)
{
	char *ret_str = NULL;

	ret_str = (char*)calloc(1, strlen(source) + strlen(substring) + 1);
	if (OP_NULL != ret_str) {
		strncpy(ret_str, source, strlen(source));
		strncat(ret_str, substring, strlen(substring));
	}

	return ret_str;
}

