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
 * @file		oma-parser-dd1.h
 * @brief		header file for oma 1.0 dd parser
 * @author		Jungki Kwak(jungki.kwak@samsung.com)
 */

#ifndef OMA_PARSER_DD1_H_
#define OMA_PARSER_DD1_H_

#include <expat.h>

#include "oma-parser-common.h"

#define DD_ELEMENT_COUNT_MAX	13
#define DD_ELEMENT_MANOPT_COUNT	3

typedef struct _more_type_info_t more_type_info_t;
struct _more_type_info_t {
	char type[OP_MAX_MIME_STR_LEN];
	struct _more_type_info_t *next;
};

/**
 * This structure contains Download descriptor information.
 * character pointer variable is used in case using rarely for optimization of memory
 * That's okay to change same type of all char. variables for consistency
 */
typedef struct _download_descriptor_oma1_t dd_oma1_t;
struct _download_descriptor_oma1_t {
	char type[OP_MAX_MIME_STR_LEN]; /* type : Mandatory */
	char name[OP_MAX_STR_LEN]; /* name : Optional */
	int major_version; /* DD Version, major : Optional */
	int minor_version; /* DD Version, minor : Optional */
	char *vendor; /* vendor : Optional */
	char object_uri[OP_MAX_URI_LEN]; /* object URI : Mandatory */
	int size; /* size : Mandatory */
	char *install_notify_uri; /* installNotifyURI : Optional */
	char *description; /* description : Optional */
	char *midlet_info_url; /* info url : Optional */
	char *icon_uri; /* iconURI : Optional */
	char *next_url; /* nextURL : Optional */
	char *install_param; /* install param : Optional */
	int other_type_count; /* count of other type : Optional */
	int progressive_download_flag; /* progressiveDownloadFlag : Optional*/
	more_type_info_t *other_type_info; /* other type : Optional */
};

typedef struct _dd1_cntx dd1_cntx;
struct _dd1_cntx {
	int bMand_element_registrar[DD_ELEMENT_MANOPT_COUNT];
	dd_oma1_t *dd1;
};

typedef enum {
	DD1_ELEMENT_NONE = -1,
	DD1_ELEMENT_TYPE, /* 0 to 3 are mandatory elements */
	DD1_ELEMENT_SIZE,
	DD1_ELEMENT_URL,
	DD1_ELEMENT_MEDIA,
	DD1_ELEMENT_NAME,
	DD1_ELEMENT_INSTALLNOTIFY,
	DD1_ELEMENT_NEXTURL, /*5*/
	DD1_ELEMENT_DDVERSION,
	DD1_ELEMENT_DESCRIPTION,
	DD1_ELEMENT_VENDOR,
	DD1_ELEMENT_INFOURL,
	DD1_ELEMENT_ICONURL,
	DD1_ELEMENT_PROGRESS /*12*/
} dd1_element_index;

typedef struct _dd1_Element_Info_table dd1_Element_Info_table;
struct _dd1_Element_Info_table {
	const char *element_str;
	dd1_element_index element_index;
	int element_inst_cnt;
};

int op_parse_dd1_file(XML_Parser parser, dd_oma1_t **dd);
void op_free_dd1_info(dd_oma1_t *dd);
int op_check_dd1_mandatory_tags(XML_Parser parser);
void op_expat_startelement_dd1(void *userData, const XML_Char *name,
                const XML_Char **atts);
void op_expat_endelement_dd1(void *userData, const XML_Char *name);
void op_expat_character_dd1(void *userData, const XML_Char *s, int len);

#endif /* OMA_PARSER_DD1_H_ */
