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
#include "Ecore_X.h"
#include "aul.h"
#include "app.h"
#include "app_service.h"

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

static void __lang_changed_cb(void *data)
{
	DM_LOGI("==Language changed notification==");
	DownloadView &view = DownloadView::getInstance();
	view.updateLang();
	return;
}

static void __region_changed_cb(void *data)
{
	DownloadView &view = DownloadView::getInstance();
	view.changedRegion();
	DM_LOGI("==Region changed notification==");
	return;
}

static void __low_memory_cb(void *data)
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

	/* Init network */
	NetMgr &netObj = NetMgr::getInstance();
	netObj.initNetwork();

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
	NetMgr &netObj = NetMgr::getInstance();
	netObj.deinitNetwork();
	DownloadView &view = DownloadView::getInstance();
	view.destroy();
	if (app_data && app_data->idler)
		ecore_idler_del(app_data->idler);
	if (app_data) {
		free(app_data);
		app_data = NULL;
	}
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
	view.resume();
	return;
}

static void __app_service(service_h s, void *data)
{
	string s_url = std::string();
	string s_cookie = std::string();
	string s_req_header_field = std::string();
	string s_req_header_value = std::string();
	string s_install_dir = std::string();
	char *url = NULL;
	char *mode = NULL;
	char *app_op = NULL;
	char *historyid_str = NULL;
	char *req_header_field = NULL;
	char *req_header_value = NULL;
	char *default_storage = NULL;
	DownloadView &view = DownloadView::getInstance();
	int ret = 0;

	DM_LOGI("");

	/* The default view mode is normal*/
	view.setSilentMode(true);
	if (service_get_operation(s, &app_op) < 0) {
		DM_LOGE("Fail to get service operation");
		return;
	}
	DM_SLOGI("operation[%s]", app_op);
	free(app_op);

	if (service_get_uri(s, &url) < 0) {
		DM_LOGE("Invalid URL");
	} else {
		DM_SLOGI("url[%s]",url);
		if (url)
			s_url = url;
		free(url);
	}

	ret = service_get_extra_data(s, KEY_DEFAULT_STORAGE, &default_storage);
	if (ret == SERVICE_ERROR_NONE) {
		if (default_storage) {
			string defaultStorage = string(default_storage);
			if (defaultStorage.compare("0") == 0)
				s_install_dir.assign(DM_DEFAULT_PHONE_INSTALL_DIR);
			else if (defaultStorage.compare("1") == 0)
				s_install_dir.assign(DM_DEFAULT_MMC_INSTALL_DIR);
			else
				s_install_dir.assign(DM_DEFAULT_PHONE_INSTALL_DIR);
			DM_SLOGI("install dir[%s]",s_install_dir.c_str());
		}
		free(default_storage);
	} else {
		DM_LOGI("Fail to get extra data default storage[%d]", ret);
	}

	ret = service_get_extra_data(s, KEY_REQ_HTTP_HEADER_FIELD, &req_header_field);
	if (ret == SERVICE_ERROR_NONE) {
		DM_SLOGI("request header filed[%s]",req_header_field);
		if (req_header_field)
			s_req_header_field = req_header_field;
		free(req_header_field);
		ret = service_get_extra_data(s, KEY_REQ_HTTP_HEADER_VALUE, &req_header_value);
		if (ret == SERVICE_ERROR_NONE) {
			DM_SLOGI("request header value[%s]",req_header_value);
			if (req_header_value)
				s_req_header_value = req_header_value;
			free(req_header_value);
		}
	}

	ret = service_get_extra_data(s, KEY_MODE, &mode);
	if (ret == SERVICE_ERROR_NONE) {
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

	ret = service_get_extra_data(s, KEY_FAILED_HISTORYID, &historyid_str);
	if (ret == SERVICE_ERROR_NONE) {
		view.setSilentMode(false);
		view.activateWindow();
		if (historyid_str) {
			unsigned int id = atoi(historyid_str);
			DM_LOGI("History Id from Failed noti[%s]", historyid_str);
			view.clickedItemFromNoti(id);
		} else {
			DM_LOGE("Invalid history Id string");
		}
		free(historyid_str);
		return;
	} else {
		DM_LOGI("Fail to get extra data failed history ID[%d]", ret);
	}

	if (s_url.empty()) {
		view.setSilentMode(false);
		view.activateWindow();
		return;
	}
	DownloadRequest request(s_url, s_cookie,
			s_req_header_field, s_req_header_value, s_install_dir);
	Item::create(request);
	view.activateWindow();

	return;
}

EXPORT_API int main(int argc, char *argv[])
{
	app_event_callback_s evt_cb = {0,};
	int ret = 0;
	struct app_data_t *app_data = NULL;

	app_data = (struct app_data_t *)calloc(1, sizeof(struct app_data_t));
	if (!app_data) {
		DM_LOGE("Fail to call calloc");
		return ret;
	}

	evt_cb.create = __app_create;
	evt_cb.terminate = __app_terminate;
	evt_cb.pause = __app_pause;
	evt_cb.resume = __app_resume;
	evt_cb.service = __app_service;
	evt_cb.low_memory = __low_memory_cb;
	evt_cb.low_battery = NULL;
	evt_cb.device_orientation = NULL;
	evt_cb.language_changed = __lang_changed_cb;
	evt_cb.region_format_changed = __region_changed_cb;

	ret = app_efl_main(&argc, &argv, &evt_cb, app_data);
	DM_LOGI("Done");
	free(app_data);

	return ret;
}

