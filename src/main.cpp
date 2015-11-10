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
 * @file	main.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	main file for download manager
 */

#include <list>
#include <string>
#include <iostream>
#include <memory>

#include "Ecore.h"
//#include "Ecore_X.h"
#include "app.h"
#include "app_control.h"

#ifdef _ENABLE_OMA_DOWNLOAD
#include <oma-parser-interface.h>
#endif

#include "download-manager-common.h"
#include "download-manager-view.h"
#include "download-manager-network.h"
#include "download-manager-downloadRequest.h"
#include "download-manager-history-db.h"
#include "download-manager-notification.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

using namespace std;

struct app_data_t {
	Ecore_Idler *idler;
	int history_count;
	int load_count;
};

static void __lang_changed_cb(app_event_info_h event_info, void *data)
{
	DM_LOGI("==Language changed notification==");
	char *lang_set = vconf_get_str(VCONFKEY_LANGSET);
	if (lang_set) {
		elm_language_set(lang_set);
		free(lang_set);
	}
	DownloadView &view = DownloadView::getInstance();
	view.updateLang();
	return;
}

static void __region_changed_cb(app_event_info_h event_info,void *data)
{
	DownloadView &view = DownloadView::getInstance();
	view.changedRegion();
	DM_LOGI("==Region changed notification==");
	return;
}

static void __low_memory_cb(app_event_info_h event_info, void *data)
{
	DM_LOGI("=== Low memory nofification ===");
	return;
}

static Eina_Bool __load_remained_history(void *data)
{
	struct app_data_t *app_data = (struct app_data_t *)data;
	if (app_data && app_data->load_count <= app_data->history_count) {
		app_data->load_count += LOAD_HISTORY_COUNT;
		DownloadHistoryDB::createRemainedItemsFromHistoryDB(
			LOAD_HISTORY_COUNT, app_data->load_count);
		return ECORE_CALLBACK_RENEW;
	} else {
		if(app_data)
			app_data->idler = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
}

static bool __app_create(void *data)
{
	int count = 0;
	struct app_data_t *app_data = (struct app_data_t *)data;
	DM_LOGI("");

	DownloadView &view = DownloadView::getInstance();
	Evas_Object *window = view.create();
	if (!window) {
		DM_LOGE("Fail to create main window");
		return false;
	}
#ifdef _ENABLE_OMA_DOWNLOAD
	op_parser_init();
#endif
	/* Make genlist items of history for UI performance */
	view.update();

	DownloadNoti::clearOngoingNoti();
	DownloadHistoryDB::getCountOfHistory(&count);
	if (count > 0) {
		DownloadHistoryDB::createItemsFromHistoryDB();
		if (count > LOAD_HISTORY_COUNT) {
			if (app_data) {
				app_data->history_count = count;
				app_data->idler = ecore_idler_add(__load_remained_history,
						app_data);
			}
		}
	}

	DM_LOGD("DONE");

	return true;
}

static void __app_terminate(void *data)
{
	DM_LOGI("");
	struct app_data_t *app_data = (struct app_data_t *)data;
	DownloadView &view = DownloadView::getInstance();
	view.destroy();
#ifdef _ENABLE_OMA_DOWNLOAD
	op_parser_deinit();
#endif
	if (app_data && app_data->idler)
		ecore_idler_del(app_data->idler);
	DownloadHistoryDB::closeDBConn();
	DM_LOGD("[Info] DB Connection closed");
	return;
}

static void __app_pause(void *data)
{
	DM_LOGI("");
	DownloadView &view = DownloadView::getInstance();
	view.pause();
	return;
}

static void __app_resume(void *data)
{
	DM_LOGI("");
	DownloadView &view = DownloadView::getInstance();
	view.setSilentMode(true);
	view.resume();
	return;
}

static void __app_control(app_control_h s, void *data)
{
	string reqUrl = std::string();
	string reqHeaderField = std::string();
	string reqHeaderValue = std::string();
	string reqInstallDir = std::string();
	string reqFileName = std::string();
	bool reqNetworkBonding = false;
	char *url = NULL;
	char *mode = NULL;
	char *app_op = NULL;
	char *historyid_str = NULL;
	char *req_header_field = NULL;
	char *req_header_value = NULL;
	char *default_storage = NULL;
	char *file_name = NULL;
	char *network_bonding = NULL;
	DownloadView &view = DownloadView::getInstance();
	int ret = 0;

	DM_LOGI("");

	/* The default view mode is normal*/
	view.setSilentMode(true);
	if (app_control_get_operation(s, &app_op) < 0) {
		DM_LOGE("Fail to get app control operation");
		return;
	}
	DM_SLOGI("operation[%s]", app_op);
	free(app_op);

	if (app_control_get_uri(s, &url) < 0) {
		DM_LOGE("Invalid URL");
	} else {
		DM_SLOGI("url[%s]",url);
		if (url)
			reqUrl = url;
		free(url);
	}

	ret = app_control_get_extra_data(s, KEY_DEFAULT_STORAGE, &default_storage);
	if (ret == APP_CONTROL_ERROR_NONE) {
		if (default_storage) {
			string defaultStorage = string(default_storage);
			if (defaultStorage.compare("1") == 0)
				reqInstallDir.assign(DM_DEFAULT_MMC_INSTALL_DIR);
			else
				reqInstallDir.assign(DM_DEFAULT_PHONE_INSTALL_DIR);
			DM_SLOGI("install dir[%s]",reqInstallDir.c_str());
		}
		free(default_storage);
	} else {
		DM_LOGI("Fail to get extra data default storage[%d]", ret);
	}

	ret = app_control_get_extra_data(s, KEY_FILE_NAME, &file_name);
	if (ret == APP_CONTROL_ERROR_NONE) {
		if (file_name) {
			reqFileName.assign(file_name);
			DM_SLOGI("User preferred file name[%s]",reqFileName.c_str());
		}
		free(file_name);
	} else {
		DM_LOGI("Fail to get extra data file name[%d]", ret);
	}

	ret = app_control_get_extra_data(s, KEY_REQ_HTTP_HEADER_FIELD, &req_header_field);
	if (ret == APP_CONTROL_ERROR_NONE) {
		DM_SLOGI("request header filed[%s]",req_header_field);
		if (req_header_field)
			reqHeaderField = req_header_field;
		free(req_header_field);
		ret = app_control_get_extra_data(s, KEY_REQ_HTTP_HEADER_VALUE, &req_header_value);
		if (ret == APP_CONTROL_ERROR_NONE) {
			DM_SLOGI("request header value[%s]",req_header_value);
			if (req_header_value)
				reqHeaderValue = req_header_value;
			free(req_header_value);
		}
	}

	ret = app_control_get_extra_data(s, KEY_MODE, &mode);
	if (ret == APP_CONTROL_ERROR_NONE) {
		DM_SLOGI("mode[%s]",mode);
		if (0 == strncmp(mode, KEY_MODE_VALUE_VIEW,
				strlen(KEY_MODE_VALUE_VIEW))) {
			DM_LOGI("View mode");
			view.setSilentMode(false);
			view.activateWindow();
			free(mode);
			return;
		} else if (0 == strncmp(mode, KEY_MODE_VALUE_SILENT,
				strlen(KEY_MODE_VALUE_SILENT))) {
			DM_LOGI("Silent mode");
			view.setSilentMode(true);
			view.activateWindow();
			free(mode);
		} else {
			DM_LOGE("Invalid mode");
			view.activateWindow();
			free(mode);
			return;
		}
	} else {
		DM_LOGI("Fail to get extra data mode[%d]", ret);
	}

	ret = app_control_get_extra_data(s, KEY_DOWNLOADING_HISTORYID, &historyid_str);
	if (ret == APP_CONTROL_ERROR_NONE) {
		view.setSilentMode(false);
		view.activateWindow();
		if (historyid_str) {
			errno = 0;
			char *end_ptr = historyid_str;
			unsigned int id = (int) strtol(historyid_str, &end_ptr, 0);
			if (ERANGE == errno) {
				DM_LOGE("HistoryId String conversion failed");
				return;
			}
			DM_LOGI("History Id from downloading noti[%s]", historyid_str);
			view.clickedItemFromNoti(id, NOTIFICATION_TYPE::NOTI_DOWNLOADING);
		} else {
			DM_LOGE("Invalid history Id string");
		}
		free(historyid_str);
		return;
	} else {
		DM_LOGI("Fail to get extra data downloading history ID[%d]", ret);
	}

	ret = app_control_get_extra_data(s, KEY_FAILED_HISTORYID, &historyid_str);
	if (ret == APP_CONTROL_ERROR_NONE) {
		view.setSilentMode(false);
		view.activateWindow();
		if (historyid_str) {
			unsigned int id = atoi(historyid_str);
			DM_LOGI("History Id from Failed noti[%s]", historyid_str);
			view.clickedItemFromNoti(id, NOTIFICATION_TYPE::NOTI_FAILED);
		} else {
			DM_LOGE("Invalid history Id string");
		}
		free(historyid_str);
		return;
	} else {
		DM_LOGI("Fail to get extra data failed history ID[%d]", ret);
	}

	ret = app_control_get_extra_data(s, KEY_NETWORK_BONDING, &network_bonding);
	if (ret == APP_CONTROL_ERROR_NONE) {
		if (network_bonding) {
			string tmpStr = string(network_bonding);
			if (tmpStr.compare("true") == 0)
				reqNetworkBonding = true;
			DM_LOGI("network_bonding option[%s]",tmpStr.c_str());
		}
		free(network_bonding);
	} else {
		DM_LOGD("Fail to get extra data network bonding[%d]", ret);
	}

	if (reqUrl.empty()) {
		view.setSilentMode(false);
		view.activateWindow();
		return;
	}
	DownloadRequest request(reqUrl, reqHeaderField, reqHeaderValue,
			reqInstallDir, reqFileName);
	request.setNetworkBondingOption(reqNetworkBonding);
	Item::create(request);
	view.activateWindow();
	return;
}

EXPORT_API int main(int argc, char *argv[])
{
	ui_app_lifecycle_callback_s evt_cb = {0,};

	int ret = 0;
	struct app_data_t *app_data = NULL;

	app_data = (struct app_data_t *)calloc(1, sizeof(struct app_data_t));
	if (!app_data) {
		DM_LOGE("Fail to call calloc");
		return ret;
	}
	if (DownloadHistoryDB::openDBConn() == true)
		DM_LOGD("[Info] DB Connection opened");

	evt_cb.create = __app_create;
	evt_cb.terminate = __app_terminate;
	evt_cb.pause = __app_pause;
	evt_cb.resume = __app_resume;
	evt_cb.app_control = __app_control;

	app_event_handler_h lang_changed_handler;
	app_event_handler_h low_memory_handler;
	app_event_handler_h region_changed_handler;

	ui_app_add_event_handler(&lang_changed_handler, APP_EVENT_LANGUAGE_CHANGED, __lang_changed_cb, &app_data);
	ui_app_add_event_handler(&low_memory_handler, APP_EVENT_LOW_MEMORY, __low_memory_cb, &app_data);
	ui_app_add_event_handler(&region_changed_handler, APP_EVENT_REGION_FORMAT_CHANGED, __region_changed_cb, &app_data);

	ret = ui_app_main(argc, argv, &evt_cb, app_data);
	DM_LOGI("Done");
	free(app_data);

	return ret;
}

