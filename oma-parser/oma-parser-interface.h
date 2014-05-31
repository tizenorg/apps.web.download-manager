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
 * @file		oma-parser-interface.h
 * @brief		Including interface functions regarding oma dd parser
 * @author		Jungki Kwak(jungki.kwak@samsung.com)
 */

#ifndef _OMA_PARSER_INTERFACE_H
#define _OMA_PARSER_INTERFACE_H


#ifdef __cplusplus
 extern "C"
 {
#endif

#include "oma-parser-dd1.h"
#include "oma-parser-common.h"

/**
  * This function parses the OMA Download Descriptor and returns parsed data specific to version.
  * It takes file path as input
  * @param[in]		dd_file_path	DD file path
  * @param[out]	dd_info		parserd data
  * @param[out]	error_code	op_parser_err_t
  * @return		true for success, or false for fail
  *
  **/

int op_parse_dd_file(const char* dd_file_path, void** dd_info, int* error_code);

/**
  * This function frees the DD data structure.
  *
  * @param[in]		dd_info	DD data info
  *
  **/
void op_free_dd_info(void* dd_info);
/**
 * @}
 */

#ifdef __cplusplus
  }
#endif


#endif
