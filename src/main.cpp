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

/**
 * @file	main.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	main file for download manager
 */

#include <list>
#include <string>
#include <iostream>
#include <memory>

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

#ifdef _ENABLE_ROTATE
int __get_rotate_angle()
{
	app_device_orientation_e rotation_state;
	rotation_state = app_get_device_orientation();

	DP_LOGD("Rotate angle[%d]",rotation_state);
	return rotation_state;
}

static void __rotate_changed_cb(app_device_orientation_e m, void *data)
{
	int angle = 0;
	DownloadView &view = DownloadView::getInstance();

	angle = __get_rotate_angle();
	view.rotateWindow(angle);
	return;
}
#endif

static void __lang_changed_cb(void *data)
{
	DP_LOGD("=== Language changed nofification ===");
	DownloadView &view = DownloadView::getInstance();
	view.updateLang();
	return;
}

static void __region_changed_cb(void *data)
{
	DownloadView &view = DownloadView::getInstance();
	view.changedRegion();
	DP_LOGD("=== Region changed nofification ===");
	return;
}

static void __low_memory_cb(void *data)
{
	DP_LOGD("=== Low memory nofification ===");
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
	} else
		return ECORE_CALLBACK_CANCEL;
}

static bool __app_create(void *data)
{
	int count = 0;
#ifdef _ENABLE_ROTATE
	int angle = 0;
#endif
	struct app_data_t *app_data = (struct app_data_t *)data;
	DP_LOG_START("App Create");

	DownloadView &view = DownloadView::getInstance();
	Evas_Object *window = view.create();
	if (!window) {
		DP_LOGE("Fail to create main window");
		return false;
	}

	/* Init network */
	NetMgr &netObj = NetMgr::getInstance();
	netObj.initNetwork();

#ifdef _ENABLE_ROTATE
	angle = __get_rotate_angle();
	view.rotateWindow(angle);
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
				app_data->idler = ecore_idler_add(__load_remained_history, app_data);
			}
		}
	}

	DP_LOGD("App Create - DONE");

	return true;
}

static void __app_terminate(void *data)
{
	DP_LOGD_FUNC();
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
	DP_LOGD_FUNC();
	DownloadView &view = DownloadView::getInstance();
	view.pause();
	return;
}

static void __app_resume(void *data)
{
	return;
}

static void __app_service(service_h s, void *data)
{
	string s_url = std::string();
	string s_cookie = std::string();
	char *url = NULL;
	char *cookie = NULL;
	char *mode = NULL;
	char *app_op = NULL;
	DownloadView &view = DownloadView::getInstance();

	DP_LOGD_FUNC();

	/* The default view mode is normal*/
	view.setSilentMode(false);
	if (service_get_operation(s, &app_op) < 0) {
		DP_LOGE("Fail to get service operation");
		return;
	}
	DP_LOGD("operation[%s]", app_op);

	if (service_get_uri(s, &url) < 0) {
		DP_LOGE("Invalid URL");
	} else {
		DP_LOGD("url[%s]",url);
		if (url)
			s_url = url;
	}

	if (service_get_extra_data(s, "cookie", &cookie) < 0) {
		DP_LOGD("No cookie");
	} else {
		DP_LOGD("cookie[%s]",cookie);
		if (cookie)
			s_cookie = cookie;
	}

	if (service_get_extra_data(s, "mode", &mode) < 0) {
		DP_LOGD("No mode");
	} else {
		DP_LOGD("mode[%s]",mode);
		if ( 0 == strncmp(mode, "view", strlen("view"))) {
			DP_LOGD("View mode");
			view.activateWindow();
			return;
		} else if ( 0 == strncmp(mode, "silent", strlen("silent"))) {
			DP_LOGD("Silent mode");
			view.setSilentMode(true);
			view.activateWindow();
		} else {
			DP_LOGE("Invalid mode");
			view.activateWindow();
			return;
		}
	}

	if (s_url.empty()) {
		view.setSilentMode(false);
		view.activateWindow();
		return;
	}
	DownloadRequest request(s_url, s_cookie);
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
		DP_LOGE("Fail to calloc of app data");
		return ret;
	}

	evt_cb.create = __app_create;
	evt_cb.terminate = __app_terminate;
	evt_cb.pause = __app_pause;
	evt_cb.resume = __app_resume;
	evt_cb.service = __app_service;
	evt_cb.low_memory = __low_memory_cb;
	evt_cb.low_battery = NULL;
#ifdef _ENABLE_ROTATE
	evt_cb.device_orientation = __rotate_changed_cb;
#else
	evt_cb.device_orientation = NULL;
#endif
	evt_cb.language_changed = __lang_changed_cb;
	evt_cb.region_format_changed = __region_changed_cb;

	ret = app_efl_main(&argc, &argv, &evt_cb, app_data);
	DP_LOGD("Main return");

	return ret;
}

