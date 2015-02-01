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
 * @file		oma-parser-dd1.c
 * @brief		function for oma 1.0 dd parser
 * @author		Jungki Kwak(jungki.kwak@samsung.com)
 */

#include <stdio.h>
#include <string.h>

#include "oma-parser-dd1.h"

static dd1_Element_Info_table dd_element_table[DD_ELEMENT_COUNT_MAX] =
{
	{"type",		DD1_ELEMENT_TYPE,	-1},	/*Mandatory */
	{"size",		DD1_ELEMENT_SIZE,	-1},	/*Mandatory */
	{"objectURI",		DD1_ELEMENT_URL,	-1},	/*Mandatory */
	{"media",		DD1_ELEMENT_MEDIA,	-1},
	{"name",		DD1_ELEMENT_NAME,	-1},
	{"installNotifyURI",	DD1_ELEMENT_INSTALLNOTIFY,	-1},
	{"nextURL",		DD1_ELEMENT_NEXTURL,	-1 },
	{"DDVersion",		DD1_ELEMENT_DDVERSION,	-1},
	{"description",		DD1_ELEMENT_DESCRIPTION,	-1},
	{"vendor",		DD1_ELEMENT_VENDOR,	-1},
	{"infoURL",		DD1_ELEMENT_INFOURL,	-1},
	{"iconURI",		DD1_ELEMENT_ICONURL,	-1},
	{"progressiveDownloadFlag",	DD1_ELEMENT_PROGRESS,	-1}

};

#define IS_ELEMENT_OPEN(INDEX)		(dd_element_table[INDEX].element_inst_cnt > -1)
#define IS_ELEMENT_CONTINUING(INDEX)	(dd_element_table[INDEX].element_inst_cnt > 1)
#define CLOSE_ELEMENT(INDEX)		(dd_element_table[INDEX].element_inst_cnt = -1)
#define OPEN_ELEMENT(INDEX)		(dd_element_table[INDEX].element_inst_cnt = 0)
#define CONTINUE_ELEMENT(INDEX)		(dd_element_table[INDEX].element_inst_cnt++)

/* Warnings / Errors Logging Functions */
static void op_parse_warning(void *userData, const char *msg, ...)
{
    OP_LOGD("Warning : %s", msg);
}

static void op_parse_error(void *userData, const char *msg, ...)
{
    OP_LOGE("Error: %s", msg);
}

static void op_parse_fatal_error(void *userData, const char *msg, ...)
{
    OP_LOGE("Fatal Error : %s", msg);
}

int op_parse_dd1_file(xmlSAXHandler *sHandlerPtr, op_parser_app_data_t **app_data)
{
	int ret = OP_RESULT_OK;
	dd1_cntx *cntx = OP_NULL;
	dd_oma1_t *dd1 = OP_NULL;
	op_parser_app_data_t *app_data_local = OP_NULL;

	OP_LOGD("");

	app_data_local = (op_parser_app_data_t *)calloc(1,
	        sizeof(op_parser_app_data_t));
	if (OP_NULL == app_data) {
		OP_LOGE("MEMORY ALLOCATION FAIL");
		ret = OP_ERR_FAIL_TO_MEMALLOC;
		goto ERR;
	}

	cntx = (dd1_cntx *)calloc(1, sizeof(dd1_cntx));
	if (OP_NULL == cntx) {
		OP_LOGE("MEMORY ALLOCATION FAIL");
		ret = OP_ERR_FAIL_TO_MEMALLOC;
		goto ERR;
	}

	dd1 = (dd_oma1_t *)calloc(1, sizeof(dd_oma1_t));
	if (OP_NULL == dd1) {
		OP_LOGE("MEMORY ALLOCATION FAIL");
		ret = OP_ERR_FAIL_TO_MEMALLOC;
		goto ERR;
	}

	*app_data = app_data_local;
	cntx->dd1 = dd1;
	app_data_local->parseError = OP_RESULT_OK;
	app_data_local->data = (void *)cntx;

	sHandlerPtr->startElement = op_libxml_start_element_dd1;
	sHandlerPtr->endElement = op_libxml_end_element_dd1;
	sHandlerPtr->characters = op_libxml_characters_dd1;
	/* warnings / error logging functions */
	sHandlerPtr->error = op_parse_error;
	sHandlerPtr->warning = op_parse_warning;
	sHandlerPtr->fatalError = op_parse_fatal_error;

ERR:
	if (OP_RESULT_OK != ret) {
		free(cntx);
		free(app_data_local);
		free(dd1);
	}

	return ret;
}

void op_free_dd1_info(dd_oma1_t *dd_info)
{
	more_type_info_t *temp_ptr = NULL;
	more_type_info_t *temp_ptr_next = NULL;

	OP_LOGD("");

	if (dd_info == NULL)
		return;

	if (dd_info->description) {
		free(dd_info->description);
		dd_info->description = OP_NULL;
	}

	if (dd_info->install_notify_uri) {
		free(dd_info->install_notify_uri);
		dd_info->install_notify_uri = OP_NULL;
	}

	if (dd_info->icon_uri) {
		free(dd_info->icon_uri);
		dd_info->icon_uri = OP_NULL;
	}

	if (dd_info->next_url) {
		free(dd_info->next_url);
		dd_info->next_url = OP_NULL;
	}

	if (dd_info->vendor) {
		free(dd_info->vendor);
		dd_info->vendor = OP_NULL;
	}

	if (dd_info->midlet_info_url) {
		free(dd_info->midlet_info_url);
		dd_info->midlet_info_url = OP_NULL;
	}

	if (dd_info->install_param) {
		free(dd_info->install_param);
		dd_info->install_param = OP_NULL;
	}

	temp_ptr = dd_info->other_type_info;
	while (temp_ptr) {
		temp_ptr_next = temp_ptr->next;
		free(temp_ptr);
		temp_ptr = temp_ptr_next;
	}

	free(dd_info);
	dd_info = NULL;
	return;
}

int op_check_dd1_mandatory_tags(op_parser_app_data_t *app_data)
{
	int count = 0;

	OP_LOGD("");

	dd1_cntx *cntx = (dd1_cntx*)app_data->data;

	while (count < DD_ELEMENT_MANOPT_COUNT) {
		if (OP_FALSE == cntx->bMand_element_registrar[count]) {
			OP_SLOG("ERR:One ore more Mandatory Elements Missing:[%s]",
					dd_element_table[count].element_str);
			return OP_FALSE;
		}
		count++;
	}

	OP_LOGD("DD1 Mandatory Element Check Pass");
	return OP_TRUE;
}

void op_libxml_start_element_dd1(
        void *userData,
        const xmlChar *name,
        const xmlChar **atts)
{
	op_parser_app_data_t *app_data = OP_NULL;
	int index = 0;

	OP_LOGD("");

	app_data = (op_parser_app_data_t*)userData;

	OP_SLOG("Start Element [%s]", name);
	app_data->element_index = DD1_ELEMENT_NONE;

	if (OP_RESULT_OK != app_data->parseError) {
		OP_LOGE("Error Encountered Already");
		goto ERR;
	}

	for (index = 0; index < DD_ELEMENT_COUNT_MAX; index++) {
		if (0 == xmlStrcmp(name,
		        (const xmlChar *)(dd_element_table[index].element_str))) {
			OP_LOGD("Element matches with listed's. Index[%d]",
					dd_element_table[index].element_index);
			app_data->element_index
			        = dd_element_table[index].element_index;
			OPEN_ELEMENT(app_data->element_index);
			break;
		}
	}

	if (index == DD_ELEMENT_COUNT_MAX) {
		app_data->parseError = OP_PARSER_ERR_UNKNOWN_ELEMENT;
		OP_SLOG("ERR:Element [%s] is not listed.", name);
		goto ERR;
	}

ERR:
	return;
}

void op_libxml_end_element_dd1(void *userData, const xmlChar *name)
{
	op_parser_app_data_t *app_data = OP_NULL;
	int index = 0;

	OP_LOGD("");

	app_data = (op_parser_app_data_t*)userData;

	OP_SLOG("End Element [%s]", name);

	if (OP_RESULT_OK != app_data->parseError) {
		OP_LOGE("Inetrnal Error Occured");
		goto ERR;
	}

	for (index = 0; index < DD_ELEMENT_COUNT_MAX; index++) {
		if (0 == xmlStrcmp(name,
		        (const xmlChar *)(dd_element_table[index].element_str))) {
			if (!IS_ELEMENT_OPEN(index)) {
				app_data->parseError
				        = OP_PARSER_ERR_INTERNAL_PARSING;
				OP_LOGE("End Element Does not match with the start element");
				goto ERR;
			}

			CLOSE_ELEMENT(app_data->element_index);
			break;
		}
	}

	if (index == DD_ELEMENT_COUNT_MAX) {
		OP_LOGE("Element is not listed");
		app_data->parseError = OP_PARSER_ERR_UNKNOWN_ELEMENT;
		goto ERR;
	}

ERR:
	return;
}

void op_libxml_characters_dd1(void *userData, const xmlChar *s, int len)
{
	dd_oma1_t *dd_info = OP_NULL;
	dd1_cntx *cntxt = OP_NULL;
	op_parser_app_data_t *app_data = OP_NULL;
	char *ch_str = OP_NULL;

	OP_LOGD("");

	app_data = (op_parser_app_data_t *)userData;
	cntxt = (dd1_cntx *)(app_data->data);
	dd_info = cntxt->dd1;

	if (OP_NULL == dd_info) {
		OP_LOGE("dd structure is NULL");
		goto ERR;
	}

	if (len <= 0) {
		goto ERR;
	}

	ch_str = (char *)calloc(1, len + 1);
	if (OP_NULL == ch_str) {
		app_data->parseError = OP_ERR_FAIL_TO_MEMALLOC;
		goto ERR;
	}

	memcpy(ch_str, (char *)s, len);
	if (app_data->element_index >= 0) {
		OP_LOGD("app_data->parseError[%d], app_data->element_index[%d], \
				dd_element_table[app_data->element_index].isTagOpen[%d]",
				app_data->parseError,app_data->element_index,
				IS_ELEMENT_OPEN(app_data->element_index));
	} else {
		OP_LOGD("app_data->parseError[%d], app_data->element_index[%d]",
				app_data->parseError,app_data->element_index);
	}
	if ((OP_RESULT_OK == app_data->parseError) &&
		(app_data->element_index != DD1_ELEMENT_NONE) &&
		(IS_ELEMENT_OPEN(app_data->element_index))) {

		CONTINUE_ELEMENT(app_data->element_index);

		switch (app_data->element_index) {
		case DD1_ELEMENT_TYPE:
			op_com_util_remove_blankspace(ch_str, len);
			if ((dd_info->type)[0] == 0) {
				if ((app_data->element_index)
				        < DD_ELEMENT_MANOPT_COUNT) {
					cntxt->bMand_element_registrar[app_data->element_index]
					        = OP_TRUE;
				} else {
					OP_LOGE("Element Index[%d] is not in Mandatory List",
							app_data->element_index);
					app_data->parseError
					        = OP_PARSER_ERR_INTERNAL_PARSING;
					goto ERR;
				}

				strncpy(dd_info->type, ch_str,
				        OP_MAX_MIME_STR_LEN - 1);
				dd_info->type[OP_MAX_MIME_STR_LEN - 1] = '\0';
				OP_SLOG("dd_info->type:[%s]", dd_info->type);
			} else {
				more_type_info_t *temp_type = OP_NULL;

				temp_type = (more_type_info_t *)calloc(1,
				        sizeof(more_type_info_t));
				if (temp_type == OP_NULL) {
					app_data->parseError
					        = OP_ERR_FAIL_TO_MEMALLOC;
					goto ERR;
				}

				temp_type->next = OP_NULL;
				strncpy((char*)(temp_type->type), ch_str,
				        OP_MAX_MIME_STR_LEN - 1);
				temp_type->type[OP_MAX_MIME_STR_LEN - 1] = '\0';
				OP_SLOG("dd_infoerror:->type[other]:[%s]", temp_type->type);

				if (dd_info->other_type_info == OP_NULL) {
					dd_info->other_type_info = temp_type;
				} else {
					more_type_info_t *Cur_Type = OP_NULL;
					Cur_Type = dd_info->other_type_info;

					while (Cur_Type->next) {
						Cur_Type = Cur_Type->next;
					}
					Cur_Type->next = temp_type;
				}

				dd_info->other_type_count++;
			}
			break;

		case DD1_ELEMENT_SIZE:
			if (app_data->element_index < DD_ELEMENT_MANOPT_COUNT) {
				cntxt->bMand_element_registrar[app_data->element_index]
				        = OP_TRUE;
			} else {
				OP_LOGE("Element Index[%d] is not in Mandatory List",
						app_data->element_index);
				app_data->parseError
				        = OP_PARSER_ERR_INTERNAL_PARSING;
				goto ERR;
			}

			op_com_util_remove_blankspace(ch_str, len);
			if (op_com_utils_is_number((const char*)ch_str)) {
				dd_info->size = atoll(ch_str);
				if (dd_info->size <= 0)
					OP_LOGE("size is invalid");

				OP_SLOG("dd_info->size:[%llu]", dd_info->size);
			} else {
				OP_LOGE("size is invalid");
			}

			break;

		case DD1_ELEMENT_URL:
			if (app_data->element_index < DD_ELEMENT_MANOPT_COUNT) {
				cntxt->bMand_element_registrar[app_data->element_index]
				        = OP_TRUE;
			} else {
				OP_LOGE("Element Index [%d] is not in Mandatory List",
						app_data->element_index);
				app_data->parseError
				        = OP_PARSER_ERR_INTERNAL_PARSING;
				goto ERR;
			}

			op_com_util_remove_blankspace(ch_str, len);
			op_com_utils_convert_amp_string(ch_str);
			if ('\0' == dd_info->object_uri[0]) {
				snprintf(dd_info->object_uri, OP_MAX_URI_LEN,
				        "%s", ch_str);
			} else if (IS_ELEMENT_CONTINUING(app_data->element_index)) {
				strncat(dd_info->object_uri, ch_str,
				        OP_MAX_URI_LEN - strlen(
				                dd_info->object_uri));
			}

			OP_SLOG("dd_info->object_uri:[%s]", dd_info->object_uri);

			break;

		case DD1_ELEMENT_NAME:
			if (IS_ELEMENT_CONTINUING(app_data->element_index)) {
				strncat(dd_info->name, ch_str, OP_MAX_URI_LEN
				        - strlen(dd_info->name));
			} else if (strlen(ch_str) < OP_MAX_STR_LEN) {
				strncpy(dd_info->name, ch_str, strlen(ch_str));
			}

			OP_SLOG("dd_info->name:[%s]", dd_info->name);

			break;

		case DD1_ELEMENT_DDVERSION:
			op_com_util_remove_blankspace(ch_str, len);
			if (op_com_util_resolve_version(ch_str,
			        &(dd_info->major_version),
			        &(dd_info->minor_version), OP_NULL)
			        != OP_RESULT_OK)
					OP_LOGE("Version Resolution Error");

			OP_SLOG("major_version:[%d],minor_version:[%d]",
					dd_info->major_version, dd_info->minor_version);

			break;

		case DD1_ELEMENT_VENDOR:
			if (IS_ELEMENT_CONTINUING(app_data->element_index)) {
				char *temp = NULL;
				temp = op_com_utils_concat(dd_info->vendor,
				        ch_str);
				if (temp) {
					free(dd_info->vendor);
					dd_info->vendor = temp;
				}
			} else {
				dd_info->vendor = (char*)calloc(1, strlen(
				        ch_str) + 1);
				if (OP_NULL == dd_info->vendor) {
					app_data->parseError
					        = OP_ERR_FAIL_TO_MEMALLOC;
					goto ERR;
				}
				strncpy(dd_info->vendor, ch_str, strlen(ch_str));
			}

			OP_SLOG("dd_info->vendor:[%s]", dd_info->vendor);

			break;

		case DD1_ELEMENT_INSTALLNOTIFY:
			op_com_util_remove_blankspace(ch_str, len);
			op_com_utils_convert_amp_string(ch_str);
			if (IS_ELEMENT_CONTINUING(app_data->element_index)) {
				char *temp = NULL;
				temp = op_com_utils_concat(
				        dd_info->install_notify_uri, ch_str);
				if (temp) {
					free(dd_info->install_notify_uri);
					dd_info->install_notify_uri = temp;
				}
			} else {
				dd_info->install_notify_uri = (char*)calloc(1,
				        strlen(ch_str) + 1);
				if (OP_NULL == dd_info->install_notify_uri) {
					app_data->parseError
					        = OP_ERR_FAIL_TO_MEMALLOC;
					goto ERR;
				}
				strncpy(dd_info->install_notify_uri,
				        (const char*)ch_str, strlen(ch_str));
			}
			OP_SLOG("dd_info->install_notify_uri:[%s]",
					dd_info->install_notify_uri);

			break;

		case DD1_ELEMENT_DESCRIPTION:
			if (IS_ELEMENT_CONTINUING(app_data->element_index)) {
				char *temp = NULL;
				temp = op_com_utils_concat(
				        dd_info->description, ch_str);
				if (temp) {
					free(dd_info->description);
					dd_info->description = temp;
				}
			} else {
				dd_info->description = (char*)calloc(1, strlen(
				        ch_str) + 1);
				if (OP_NULL == dd_info->description) {
					app_data->parseError
					        = OP_ERR_FAIL_TO_MEMALLOC;
					goto ERR;
				}
				strncpy(dd_info->description,
				        (const char*)ch_str, strlen(ch_str));
			}
			OP_SLOG("dd_info->description:[%s]",
					dd_info->description);

			break;

		case DD1_ELEMENT_INFOURL:
			op_com_util_remove_blankspace(ch_str, len);
			op_com_utils_convert_amp_string(ch_str);
			if (IS_ELEMENT_CONTINUING(app_data->element_index)) {
				char *temp = NULL;
				temp = op_com_utils_concat(
				        dd_info->midlet_info_url, ch_str);
				if (temp) {
					free(dd_info->midlet_info_url);
					dd_info->midlet_info_url = temp;
				}
			} else {
				dd_info->midlet_info_url = (char*)calloc(1,
				        strlen(ch_str) + 1);
				if (OP_NULL == dd_info->midlet_info_url) {
					app_data->parseError
					        = OP_ERR_FAIL_TO_MEMALLOC;
					goto ERR;
				}
				strncpy(dd_info->midlet_info_url,
				        (const char*)ch_str, strlen(ch_str));
			}
			OP_SLOG("dd_info->midlet_info_url:[%s]",
					dd_info->midlet_info_url);

			break;

		case DD1_ELEMENT_ICONURL:
			op_com_util_remove_blankspace(ch_str, len);
			op_com_utils_convert_amp_string(ch_str);
			if (IS_ELEMENT_CONTINUING(app_data->element_index)) {
				char *temp = NULL;
				temp = op_com_utils_concat(dd_info->icon_uri,
				        ch_str);
				if (temp) {
					free(dd_info->icon_uri);
					dd_info->icon_uri = temp;
				}
			} else {
				dd_info->icon_uri = (char*)calloc(1, strlen(
				        ch_str) + 1);
				if (OP_NULL == dd_info->icon_uri) {
					app_data->parseError
					        = OP_ERR_FAIL_TO_MEMALLOC;
					goto ERR;
				}
				strncpy(dd_info->icon_uri, (const char*)ch_str,
				        strlen(ch_str));
				OP_SLOG("dd_info->icon_uri:[%s]",
						dd_info->icon_uri);
			}
			break;

		case DD1_ELEMENT_NEXTURL:
			op_com_util_remove_blankspace(ch_str, len);
			op_com_utils_convert_amp_string(ch_str);
			if (IS_ELEMENT_CONTINUING(app_data->element_index)) {
				char *temp = NULL;
				temp = op_com_utils_concat(dd_info->next_url,
				        ch_str);
				if (temp) {
					free(dd_info->next_url);
					dd_info->next_url = temp;
				}
			} else {
				dd_info->next_url = (char*)calloc(1, strlen(
				        ch_str) + 1);
				if (OP_NULL == dd_info->next_url) {
					app_data->parseError
					        = OP_ERR_FAIL_TO_MEMALLOC;
					goto ERR;
				}
				strncpy(dd_info->next_url, (const char*)ch_str,
				        strlen(ch_str));
			}

			OP_SLOG("dd_info->next_url:[%s]",
					dd_info->next_url);

			break;

		case DD1_ELEMENT_PROGRESS:
			if (strcmp("true", (char*)ch_str) == 0 || strcmp(
			        "TRUE", (char*)ch_str) == 0) {
					dd_info->progressive_download_flag = OP_TRUE;
			} else {
				dd_info->progressive_download_flag = OP_FALSE;
			}
			OP_LOGD("dd_info->progressive_download_flag:[%d]",
					dd_info->progressive_download_flag);

			break;

		case DD1_ELEMENT_MEDIA:
			break;

		default:
			OP_LOGE("Element not listed.");

			break;
		}
	} else {
		if (app_data->element_index >= 0)
			OP_SLOG("ERR:Element[%s] Value String[%s]",
					dd_element_table[app_data->element_index].element_str,
					ch_str);
	}

ERR:
	if (ch_str)
		free(ch_str);

	return;
}

