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
 * @file 	download-manager-common.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Common define and data
 */

#ifndef DOWNLOAD_MANAGER_DOWNLOAD_COMMON_H
#define DOWNLOAD_MANAGER_DOWNLOAD_COMMON_H

#include <libintl.h>
#include "download-manager-debug.h"

#if !defined(PACKAGE)
	#define PACKAGE "download-manager"
#endif

#define _EDJ(o) elm_layout_edje_get(o)
#define __(s) dgettext(PACKAGE, s)
#define S_(s) dgettext("sys_string", s)

#define ERROR_POPUP_LOW_MEM S_("IDS_COM_POP_NOT_ENOUGH_MEMORY")
#define ERROR_POPUP_UNKNOWN S_("IDS_COM_POP_INTERNAL_ERROR")
#define ERROR_POPUP_INVALID_URL S_("IDS_COM_POP_INVALID_URL")

#define DP_DRM_ICON_PATH IMAGEDIR"/Q02_icon_drm.png"
#define DP_UNKNOWN_ICON_PATH IMAGEDIR"/Q02_icon_unknown.png"
#define DP_EXCEL_ICON_PATH IMAGEDIR"/Q02_icon_excel.png"
#define DP_HTML_ICON_PATH IMAGEDIR"/Q02_icon_html.png"
#define DP_MUSIC_ICON_PATH IMAGEDIR"/Q02_icon_music.png"
#define DP_PDF_ICON_PATH IMAGEDIR"/Q02_icon_pdf.png"
#define DP_PPT_ICON_PATH IMAGEDIR"/Q02_icon_ppt.png"
#define DP_TEXT_ICON_PATH IMAGEDIR"/Q02_icon_text.png"
#define DP_WORD_ICON_PATH IMAGEDIR"/Q02_icon_word.png"
#define DP_VIDEO_ICON_PATH IMAGEDIR"/Q02_icon_video.png"
#define DP_IMAGE_ICON_PATH IMAGEDIR"/Q02_icon_image.png"
#define DP_FAILED_ICON_PATH IMAGEDIR"/Q02_icon_failed.png"
#define DP_NOTIFICATION_ICON_PATH IMAGEDIR"/Q02_Notification_Download_failed.png"

#define MAX_FILE_PATH_LEN 256
#define MAX_BUF_LEN 256

#define LOAD_HISTORY_COUNT 500

enum
{
	DP_CONTENT_NONE = 0,
	DP_CONTENT_IMAGE,
	DP_CONTENT_VIDEO,
	DP_CONTENT_MUSIC,
	DP_CONTENT_PDF,
	DP_CONTENT_WORD,
	DP_CONTENT_PPT, // 5
	DP_CONTENT_EXCEL,
	DP_CONTENT_HTML,
	DP_CONTENT_TEXT,
	DP_CONTENT_DRM,
	DP_CONTENT_UNKOWN // 10
};

namespace DL_TYPE{
enum TYPE {
	TYPE_NONE,
	HTTP_DOWNLOAD,
};
}

namespace ERROR {
enum CODE {
	NONE = 0,
	INVALID_URL,
	NETWORK_FAIL,
	NOT_ENOUGH_MEMORY,
	FAIL_TO_INSTALL,
	ENGINE_FAIL,
#ifdef _ENABLE_OMA_DOWNLOAD
	PARSING_FAIL,
#endif
	UNKNOWN
};
}

#endif /* DOWNLOAD_MANAGER_DOWNLOAD_COMMON_H */
