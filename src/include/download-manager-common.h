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

#define ERROR_POPUP_LOW_MEM __("IDS_DM_POP_NOT_ENOUGH_MEMORY_DELETE_SOME_ITEMS")
#define ERROR_POPUP_UNKNOWN S_("IDS_COM_POP_INTERNAL_ERROR")
#define ERROR_POPUP_INVALID_URL S_("IDS_COM_POP_INVALID_URL")

#define DM_DRM_ICON_PATH IMAGEDIR"/U01_icon_drm.png"
#define DM_UNKNOWN_ICON_PATH IMAGEDIR"/U01_icon_unkown.png"
#define DM_EXCEL_ICON_PATH IMAGEDIR"/U01_icon_excel.png"
#define DM_HTML_ICON_PATH IMAGEDIR"/U01_icon_html.png"
#define DM_MUSIC_ICON_PATH IMAGEDIR"/U01_list_icon_mp3.png"
#define DM_PDF_ICON_PATH IMAGEDIR"/U01_icon_pdf.png"
#define DM_PPT_ICON_PATH IMAGEDIR"/U01_icon_ppt.png"
#define DM_TEXT_ICON_PATH IMAGEDIR"/U01_icon_text.png"
#define DM_WORD_ICON_PATH IMAGEDIR"/U01_icon_word.png"
#define DM_VIDEO_ICON_PATH IMAGEDIR"/U01_list_icon_mp4.png"
#define DM_IMAGE_ICON_PATH IMAGEDIR"/U01_list_icon_image.png"
#define DM_FALSH_ICON_PATH IMAGEDIR"/U01_icon_swf.png"
#define DM_TPK_ICON_PATH IMAGEDIR"/U01_icon_tpk.png"
#define DM_VCAL_ICON_PATH IMAGEDIR"/U01_icon_vcs.png"
#define DM_FAILED_ICON_PATH IMAGEDIR"/U01_icon_broken.png"
#define DM_NOTI_COMPLETED_ICON_PATH IMAGEDIR"/Q02_Notification_download_complete.png"
#define DM_NOTI_FAILED_ICON_PATH IMAGEDIR"/Q02_Notification_Download_failed.png"
#define DM_NOTI_ONGOING_ICON_PATH IMAGEDIR"/Notification_download_animation.gif"
#define DM_NOTI_DOWNLOADING_ICON_PATH "reserved://indicator/ani/downloading"

#define MAX_FILE_PATH_LEN 256
#define MAX_BUF_LEN 256
#define LOAD_HISTORY_COUNT 500
#define WAITING_RO_MAX_SECONDS 20

#define KEY_MODE "mode"
#define KEY_REQ_HTTP_HEADER_FIELD "req_http_header_field"
#define KEY_REQ_HTTP_HEADER_VALUE "req_http_header_value"
#define KEY_MODE_VALUE_VIEW "view"
#define KEY_MODE_VALUE_SILENT "silent"
#define KEY_FAILED_HISTORYID "failed_historyid"
#define KEY_DEFAULT_STORAGE "default_storage"

#define DM_DRM_MIME_STR "application/vnd.oma.drm.message"
#define DM_DCF_MIME_STR "application/vnd.oma.drm.content"

#define MYFILE_PKG_NAME "tizen.filemanager"

#define DM_DEFAULT_PHONE_TEMP_DIR "/opt/usr/media/.temp_download"
#define DM_DEFAULT_PHONE_INSTALL_DIR "/opt/usr/media/Downloads/"
#define DM_DEFAULT_MMC_TEMP_DIR "/opt/storage/sdcard/.temp_download"
#define DM_DEFAULT_MMC_INSTALL_DIR "/opt/storage/sdcard/Downloads/"
#define DM_TEMP_DIR_NAME ".temp_download/"
enum
{
	DM_CONTENT_NONE = 0,
	DM_CONTENT_IMAGE,
	DM_CONTENT_VIDEO,
	DM_CONTENT_MUSIC,
	DM_CONTENT_PDF,
	DM_CONTENT_WORD,
	DM_CONTENT_PPT, // 5
	DM_CONTENT_EXCEL,
	DM_CONTENT_HTML,
	DM_CONTENT_TEXT,
	DM_CONTENT_DRM,
	DM_CONTENT_SD_DRM,//10
	DM_CONTENT_FLASH,
	DM_CONTENT_TPK,
	DM_CONTENT_VCAL,
	DM_CONTENT_UNKOWN //14
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
