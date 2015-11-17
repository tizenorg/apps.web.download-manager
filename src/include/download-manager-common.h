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

#define _EDJ(o) elm_layout_edje_get(o)
#define __(s) dgettext(PROJ_NAME, s)
#define S_(s) dgettext("sys_string", s)

#define DM_LOCALEDIR LOCALEDIR
#define DM_DOMAIN PROJ_NAME

// Download Manager Common Strings.
#define ERROR_POPUP_LOW_MEM_DEFAULT_PHONE_WITH_SD_CARD __("IDS_DM_BODY_THERE_IS_NOT_ENOUGH_SPACE_IN_YOUR_DEVICE_STORAGE_GO_TO_PS_SETTINGS_TO_FREE_UP_SOME_STORAGE_SPACE_THEN_TRY_AGAIN")
#define ERROR_POPUP_LOW_MEM_DEFAULT_PHONE_WITHOUT_SD_CARD __("IDS_DM_BODY_THERE_IS_NOT_ENOUGH_SPACE_IN_YOUR_DEVICE_STORAGE_GO_TO_PS_SETTINGS_TO_DELETE_SOME_DATA_OR_INSERT_AN_SD_CARD")
#define ERROR_POPUP_LOW_MEM_DEFAULT_MMC_WITH_NO_PHONE_MEMORY_AVAILABLE __("IDS_DM_BODY_THERE_IS_NOT_ENOUGH_SPACE_ON_YOUR_SD_CARD_GO_TO_PS_SETTINGS_TO_DELETE_SOME_DATA_THEN_TRY_AGAIN")
#define ERROR_POPUP_LOW_MEM_DEFAULT_MMC_WITH_PHONE_MEMORY_AVAILABLE __("IDS_DM_BODY_THERE_IS_NOT_ENOUGH_SPACE_ON_YOUR_SD_CARD_GO_TO_PS_SETTINGS_TO_DELETE_SOME_DATA_OR_CHANGE_THE_DEFAULT_STORAGE_LOCATION_TO_DEVICE")

#define DM_POP_TEXT_DOWNLOAD_COMPLETE __("IDS_DM_HEADER_DOWNLOAD_COMPLETE")
#define DM_POP_TEXT_DOWNLOAD_FAILED __("IDS_DM_BODY_DOWNLOAD_FAILED_M_STATUS_ABB")
#define DM_DOWNLOAD_COMPLETE_STRING_ID __("IDS_DM_HEADER_DOWNLOAD_COMPLETE")
#define DM_DOWNLOAD_FAILED_STRING_ID __("IDS_DM_BODY_DOWNLOAD_FAILED_M_STATUS_ABB")

#ifdef _TIZEN_2_3_UX
#define DM_POP_TEXT_ERROR ""
#define DM_BODY_TEXT_NO_NAME "No name"
#define DM_BODY_TEXT_UNSUPPORTED_FILE_TYPE "Unsupported file type"
#define DM_OPT_TEXT_DELETE __("IDS_DM_OPT_DELETE")
#define DM_SK_TEXT_CANCEL __("IDS_DM_BUTTON_CANCEL_ABB2")
#define DM_SK_TEXT_OK __("IDS_DM_BUTTON_OK_ABB")
#define DM_BODY_TEXT_SELECT_ALL __("IDS_DM_MBODY_SELECT_ALL")
#define DM_BODY_TEXT_PD_SELECTED __("IDS_DM_HEADER_PD_SELECTED_ABB3")
#define DM_SK_TEXT_DOWNLOAD __("IDS_DM_BUTTON_DOWNLOAD_ABB2")
#define DM_BODY_TEXT_TODAY __("IDS_DM_HEADER_TODAY")
#define DM_BODY_TEXT_YESTERDAY __("IDS_DM_HEADER_YESTERDAY")
#define DM_POP_TEXT_FAILED __("IDS_DM_BODY_DOWNLOAD_FAILED_M_STATUS_ABB")
#else
#define DM_POP_TEXT_ERROR S_("IDS_COM_POP_ERROR")
#define DM_BODY_TEXT_NO_NAME S_("IDS_COM_BODY_NO_NAME")
#define DM_BODY_TEXT_UNSUPPORTED_FILE_TYPE S_("IDS_COM_BODY_UNSUPPORTED_FILE_TYPE")
#define DM_OPT_TEXT_DELETE S_("IDS_COM_OPT_DELETE")
#define DM_SK_TEXT_CANCEL S_("IDS_COM_SK_CANCEL")
#define DM_SK_TEXT_OK S_("IDS_COM_SK_OK")
#define DM_BODY_TEXT_SELECT_ALL S_("IDS_COM_BODY_SELECT_ALL")
#define DM_BODY_TEXT_PD_SELECTED S_("IDS_COM_BODY_PD_SELECTED")
#define DM_SK_TEXT_DOWNLOAD S_("IDS_COM_SK_DOWNLOAD")
#define DM_BODY_TEXT_TODAY S_("IDS_COM_BODY_TODAY")
#define DM_BODY_TEXT_YESTERDAY S_("IDS_COM_BODY_YESTERDAY")
#define DM_POP_TEXT_FAILED S_("IDS_COM_POP_FAILED")
#endif

// Downlaod Manager resources
#ifdef _TIZEN_2_3_UX
#define DM_COLOR_TABLE_PATH TABLEDIR"/org.tizen.download-manager_ChangeableColorInfo.xml"
#define DM_FONT_TABLE_PATH TABLEDIR"/org.tizen.download-manager_ChangeableFontInfo.xml"

#define DM_DRM_ICON_PATH IMAGEDIR"/download_manager_icon_drm.png"
#define DM_UNKNOWN_ICON_PATH IMAGEDIR"/download_manager_icon_unknown.png"
#define DM_EXCEL_ICON_PATH IMAGEDIR"/download_manager_icon_xls.png"
#define DM_HTML_ICON_PATH IMAGEDIR"/download_manager_icon_html.png"
#define DM_PDF_ICON_PATH IMAGEDIR"/download_manager_icon_pdf.png"
#define DM_PPT_ICON_PATH IMAGEDIR"/download_manager_icon_ppt.png"
#define DM_TEXT_ICON_PATH IMAGEDIR"/download_manager_icon_text.png"
#define DM_WORD_ICON_PATH IMAGEDIR"/download_manager_icon_word.png"
#define DM_MUSIC_ICON_PATH IMAGEDIR"/download_manager_icon_music.png"
#define DM_VIDEO_ICON_PATH IMAGEDIR"/download_manager_icon_movie.png"
#define DM_IMAGE_ICON_PATH IMAGEDIR"/download_manager_icon_img.png"
#define DM_FLASH_ICON_PATH IMAGEDIR"/download_manager_icon_swf.png"
#define DM_TPK_ICON_PATH IMAGEDIR"/download_manager_icon_tpk.png"
#define DM_VCAL_ICON_PATH IMAGEDIR"/download_manager_icon_date.png"
#define DM_FAILED_ICON_PATH IMAGEDIR"/download_manager_icon_broken.png"
#define DM_CANCEL_BUTTON_ICON_PATH IMAGEDIR"/download_manager_list_icon_cancel.png"
#define DM_CANCEL_BUTTON_BG_PATH IMAGEDIR"/download_manager_list_button_bg.png"
#define DM_NOTI_COMPLETED_ICON_PATH IMAGEDIR"/noti_download_complete.png"
#define DM_NOTI_FAILED_ICON_PATH IMAGEDIR"/noti_download_failed.png"
#define DM_NOTI_FAILED_INDICATOR_ICON_PATH IMAGEDIR"/B03_Processing_download_failed.png"
#define DM_NOTI_COMPLETED_INDICATOR_ICON_PATH IMAGEDIR"/B03_Processing_download_ani_06.png"

#define DM_NOTI_ONGOING_ICON_PATH "reserved://quickpanel/ani/downloading"
#define DM_NOTI_DOWNLOADING_ICON_PATH "reserved://indicator/ani/downloading"
#else
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
#define DM_FLASH_ICON_PATH IMAGEDIR"/U01_icon_swf.png"
#define DM_TPK_ICON_PATH IMAGEDIR"/U01_icon_tpk.png"
#define DM_VCAL_ICON_PATH IMAGEDIR"/U01_icon_vcs.png"
#define DM_FAILED_ICON_PATH IMAGEDIR"/U01_icon_broken.png"
#define DM_NOTI_COMPLETED_ICON_PATH IMAGEDIR"/noti_download_complete.png"
#define DM_NOTI_FAILED_ICON_PATH IMAGEDIR"/noti_download_failed.png"
#define DM_NOTI_ONGOING_ICON_PATH "reserved://quickpanel/ani/downloading"
#define DM_NOTI_DOWNLOADING_ICON_PATH "reserved://indicator/ani/downloading"
#define DM_SELECT_ALL_ICON_PATH IMAGEDIR"/00_icon_select_all.png"
#define DM_DELETE_ICON_PATH IMAGEDIR"/00_icon_delete.png"
#define DM_NO_CONTENT_ICON_PATH IMAGEDIR"/00_nocontents_text.png"
#define DM_NOTI_FAILED_INDICATOR_ICON_PATH IMAGEDIR"/B03_processing_download_failed.png"
#define DM_NOTI_COMPLETED_INDICATOR_ICON_PATH IMAGEDIR"/B03_processing_download_completed.png"
#endif

// Other defines
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
#define KEY_DOWNLOADING_HISTORYID "downloading_historyid"
#define KEY_DEFAULT_STORAGE "default_storage"
#define KEY_NETWORK_BONDING "network_bonding"
#define KEY_FILE_NAME "file_name"

#define DM_DRM_MIME_STR "application/vnd.oma.drm.message"
#define DM_DCF_MIME_STR "application/vnd.oma.drm.content"

#ifdef _TIZEN_2_3_UX
#define MYFILE_PKG_NAME "org.tizen.myfile-lite"
#else
#define MYFILE_PKG_NAME "org.tizen.myfile"
#endif

//#define DM_DEFAULT_PHONE_TEMP_DIR "/opt/usr/media/.temp_download"
//#define DM_DEFAULT_PHONE_INSTALL_DIR "/opt/usr/media/Downloads/"
#define DM_DEFAULT_PHONE_TEMP_DIR "/home/owner/content/Downloads/.temp_download"
#define DM_DEFAULT_PHONE_INSTALL_DIR "/home/owner/content/Downloads/"

#define DM_DEFAULT_MMC_TEMP_DIR "/opt/storage/sdcard/.temp_download"
#define DM_DEFAULT_MMC_INSTALL_DIR "/opt/storage/sdcard/Downloads/"
//#define DM_DEFAULT_MMC_TEMP_DIR "/home/owner/content/Downloads/.temp_download"
//#define DM_DEFAULT_MMC_INSTALL_DIR "/home/owner/content/Downloads"

#define DM_TEMP_DIR_NAME ".temp_download/"
#define DM_INVALID_PATH_STRING ";\\\":*?<>|()"
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

namespace NOTIFICATION_TYPE{
enum TYPE {
	NOTI_NONE = 0,
	NOTI_DOWNLOADING,
	NOTI_FAILED,
	NOTI_COMPLETED
};
}

#endif /* DOWNLOAD_MANAGER_DOWNLOAD_COMMON_H */
