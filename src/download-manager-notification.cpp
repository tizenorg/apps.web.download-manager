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
 * @file	download-manager-notification.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief   Noti APIs and interface
 */

#include <stdio.h>
#include <vector>

#include "app_control.h"
#include "app_control_internal.h"
#include "download-manager-notification.h"

DownloadNoti::DownloadNoti(Item *item)
	: m_notiHandle(NULL)
	, m_item(item)
{
	DM_LOGD("");

	if (item) {
		m_aptr_observer = auto_ptr<Observer>(
			new Observer(updateCB, this, "notiObserver"));
		item->subscribe(m_aptr_observer.get());
	}
}

DownloadNoti::~DownloadNoti()
{
	DM_LOGD("");
}

void DownloadNoti::updateCB(void *data)
{
	if (data)
		static_cast<DownloadNoti*>(data)->updateFromItem();
}

void DownloadNoti::updateFromItem()
{
	string msg;
	if (!m_item)
		return;

	DM_LOGV("state:[%d]", m_item->getState());
	switch(m_item->getState()) {
	case ITEM::REQUESTING:
	case ITEM::QUEUED:
		DM_LOGD("REQUESTING or QUEUED");
		if (!m_notiHandle)
			addOngoingNoti();
		break;
	case ITEM::PREPARE_TO_RETRY:
		DM_LOGD("PREPARE_TO_RETRY");
		/* In case of retry, previous noti is deletead and
		 * will add ongoing notification */
		deleteCompleteNoti();
		addOngoingNoti();
		break;
	case ITEM::RECEIVING_DOWNLOAD_INFO:
		DM_LOGD("RECEIVING_DOWNLOAD_INFO");
#ifdef _ENABLE_OMA_DOWNLOAD
		if (!m_item->isOMAMime())
#endif
			updateTitleOngoingNoti();
		break;
	case ITEM::DOWNLOADING:
		updateOngoingNoti();
		break;
	case ITEM::CANCEL:
		DM_LOGD("CANCEL");
		if (m_notiHandle) {
			deleteNotiData(m_notiHandle);
			freeNotiData(m_notiHandle);
		}
		addFailedNoti();
		break;
	case ITEM::REGISTERING_TO_SYSTEM:
		DM_LOGD("REGISTERING_TO_SYSTEM");
		break;
	case ITEM::FAIL_TO_DOWNLOAD:
		DM_LOGD("FAIL_TO_DOWNLOAD");
		if (m_notiHandle) {
			deleteNotiData(m_notiHandle);
			freeNotiData(m_notiHandle);
		}
		addFailedNoti();
		break;
	case ITEM::FINISH_DOWNLOAD:
		DM_LOGD("FINISH_DOWNLOAD");
		if (m_notiHandle) {
			deleteNotiData(m_notiHandle);
			freeNotiData(m_notiHandle);
		}
		addCompleteNoti();
		break;
	case ITEM::DESTROY:
		DM_LOGD("DESTROY");
		if (m_item)
			m_item->deSubscribe(m_aptr_observer.get());
		m_aptr_observer->clear();
		break;
	default:
		break;
	}
}

void DownloadNoti::freeNotiData(notification_h notiHandle)
{
	DM_LOGV("");
	int err = NOTIFICATION_ERROR_NONE;

	if (notiHandle) {
		err = notification_free(notiHandle);
		if (err != NOTIFICATION_ERROR_NONE)
			DM_LOGE("Fail to free noti data [%d]",err);
		notiHandle = NULL;
	}
}

void DownloadNoti::deleteNotiData(notification_h notiHandle)
{
	DM_LOGV("");
	int err = NOTIFICATION_ERROR_NONE;

	if (notiHandle) {
		err = notification_delete(notiHandle);
		if (err != NOTIFICATION_ERROR_NONE)
			DM_LOGE("Fail to delete noti data [%d]",err);
	}
}

void DownloadNoti::addOngoingNoti()
{
	notification_h notiHandle = NULL;
	int err = NOTIFICATION_ERROR_NONE;
	app_control_h handle;

	if (!m_item) {
		DM_LOGE("NULL Check:item");
		return;
	}
	DM_LOGV("downloadId[%d]", m_item->getId());
	DM_SLOGD("title[%s]", m_item->getTitle().c_str());

	notiHandle = notification_create(NOTIFICATION_TYPE_ONGOING);
	if (!notiHandle) {
		DM_LOGE("Fail to create notification handle");
		return;
	}

	string tmpStr;
	if (m_item->getTitle().empty()) {
		tmpStr = string(DM_BODY_TEXT_NO_NAME);
	} else {
		tmpStr = m_item->getTitle();
	}

	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
			tmpStr.c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set title [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON,
			DM_NOTI_ONGOING_ICON_PATH);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set icon [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	err = notification_set_property(notiHandle, NOTIFICATION_PROP_DISABLE_TICKERNOTI);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set property [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
			DM_NOTI_DOWNLOADING_ICON_PATH);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("[FAIL] set icon indicator [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	err = notification_set_display_applist(notiHandle, NOTIFICATION_DISPLAY_APP_ALL);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set disable icon [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	if (app_control_create(&handle) != APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to create app_control");
		freeNotiData(notiHandle);
		return;
	}

	if (app_control_set_app_id(handle, PACKAGE_NAME) !=
			APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to app_control set pkgname");
		app_control_destroy(handle);
		freeNotiData(notiHandle);
		return;
	}
	char buff[MAX_BUF_LEN] = {0,};
	snprintf(buff, MAX_BUF_LEN, "%u", m_item->getHistoryId());
	if (app_control_add_extra_data(handle, KEY_DOWNLOADING_HISTORYID,
			(const char *)buff) != APP_CONTROL_ERROR_NONE) {
				DM_LOGE("Fail to app_control set extra param data");
				app_control_destroy(handle);
				freeNotiData(notiHandle);
				return;
	}
    if (app_control_foreach_extra_data(handle, __app_control_extra_data_cb, NULL) != APP_CONTROL_ERROR_NONE) {
        DM_LOGE("Fail to app_control_foreach_extra_data");
		app_control_destroy(handle);
		freeNotiData(notiHandle);
		return;
	}
    err = notification_set_launch_option(notiHandle, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, (void *)handle);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set apps [%d]", err);
		freeNotiData(notiHandle);
		app_control_destroy(handle);
		return;
	}
	if (app_control_destroy(handle) != APP_CONTROL_ERROR_NONE)
		DM_LOGE("Failed to destroy the app_control");

    err = notification_post(notiHandle);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to insert [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	m_notiHandle = notiHandle;
	return;
}

void DownloadNoti::updateTitleOngoingNoti()
{
	DM_LOGD("");
	if (m_notiHandle) {
		string tmpStr = m_item->getTitle();
		int err = NOTIFICATION_ERROR_NONE;
		DM_SLOGD("title[%s]", m_item->getTitle().c_str());
		err = notification_set_text(m_notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
				tmpStr.c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to set title [%d]", err);
			freeNotiData(m_notiHandle);
			m_notiHandle = NULL;
			return;
		}
		err = notification_update(m_notiHandle);
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to update [%d]", err);
			freeNotiData(m_notiHandle);
			m_notiHandle = NULL;
			return;
		}
	}
}

void DownloadNoti::updateOngoingNoti(void)
{
	int err = NOTIFICATION_ERROR_NONE;

	if (m_item->getFileSize() > 0) {
		double progress = (double)m_item->getReceivedFileSize() / (double)m_item->getFileSize();
        err = notification_set_progress(m_notiHandle, progress);
		if (err != NOTIFICATION_ERROR_NONE)
			DM_LOGE("Fail to update noti progress[%d]", err);
	} else {
        err = notification_set_size(m_notiHandle, (double)m_item->getReceivedFileSize());
		if (err != NOTIFICATION_ERROR_NONE)
            DM_LOGE("Fail to update noti size[%d]", err);
	}
}

string DownloadNoti::convertSizeStr(unsigned long long size)
{
	const char *unitStr[4] = {"B", "KB", "MB", "GB"};
	double doubleTypeBytes = 0.0;
	int unit = 0;
	unsigned long long bytes = size;
	unsigned long long unitBytes = bytes;
	string temp;

	/* using bit operation to avoid floating point arithmetic */
	for (unit = 0; (unitBytes > 1024 && unit < 4); unit++) {
		unitBytes = unitBytes >> 10;
	}
	unitBytes = 1 << (10 * unit);

	if (unit > 3)
		unit = 3;

	char str[64] = {0};
	if (unit == 0) {
		snprintf(str, sizeof(str), "%llu %s", bytes, unitStr[unit]);
	} else {
		doubleTypeBytes = ((double)bytes / (double)unitBytes);
		snprintf(str, sizeof(str), "%.2f %s", doubleTypeBytes, unitStr[unit]);
	}

	str[63] = '\0';
	temp = string(str);
	DM_LOGV("size[%ld]",size);
	return temp;
}

notification_h DownloadNoti::createNoti(NOTIFICATION_TYPE::TYPE type)
{
	notification_h notiHandle = NULL;
	int err = NOTIFICATION_ERROR_NONE;
	const char *fileName = NULL;
	const char *statusText = NULL;
	const char *statusTextId = NULL;

	DM_LOGI("historyId [%d], downloadId[%d]",
			m_item->getHistoryId(), m_item->getId());
	DM_SLOGD("title[%s]", m_item->getTitle().c_str());

	/*If setting complete noti Remove ongoing noti first */
    err = notification_delete(m_notiHandle);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to delete ongoing noti [%d]", err);
	}

	notiHandle = notification_create(NOTIFICATION_TYPE_NOTI);
	if (!notiHandle) {
		DM_LOGE("Fail to insert notification");
		return NULL;
	}

	err = notification_set_layout(notiHandle, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set notification layout [%d]", err);
		freeNotiData(notiHandle);
		return NULL;
	}

	err = notification_set_text_domain(notiHandle, DM_DOMAIN, DM_LOCALEDIR);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set text domain [%d]", err);
		freeNotiData(notiHandle);
		return NULL;
	}

	if (m_item->getTitle().empty())
		fileName = DM_BODY_TEXT_NO_NAME;
	else
		fileName = m_item->getTitle().c_str();

	if (NOTIFICATION_TYPE::NOTI_COMPLETED == type) {
		statusText = DM_POP_TEXT_DOWNLOAD_COMPLETE;
		statusTextId = "IDS_DM_HEADER_DOWNLOAD_COMPLETE";
	} else if (NOTIFICATION_TYPE::NOTI_FAILED == type) {
		statusText = DM_POP_TEXT_DOWNLOAD_FAILED;
		statusTextId = "IDS_DM_BODY_DOWNLOAD_FAILED_M_STATUS_ABB";
	}

#ifdef _TIZEN_2_3_UX
	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
			fileName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set status [%d]", err);
		freeNotiData(notiHandle);
		return NULL;
	}

	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_CONTENT,
				statusText, statusTextId, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set content name [%d]", err);
		freeNotiData(notiHandle);
		return NULL;
	}
#else
	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_CONTENT,
			fileName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set status [%d]", err);
		freeNotiData(notiHandle);
		return NULL;
	}

	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
				statusText, statusTextId, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set content name [%d]", err);
		freeNotiData(notiHandle);
		return NULL;
	}
#endif
	err = notification_set_time(notiHandle, (time_t)(m_item->getFinishedTime()));
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set time [%d]", err);
		freeNotiData(notiHandle);
		return NULL;
	}
	return notiHandle;
}

void DownloadNoti::addCompleteNoti()
{
	notification_h notiHandle = NULL;
	int err = NOTIFICATION_ERROR_NONE;
	app_control_h handle;

	if (!m_item) {
		DM_LOGE("m_item is NULL");
		return;
	}
	if((notiHandle = createNoti(NOTIFICATION_TYPE::NOTI_COMPLETED)) == NULL) {
		DM_LOGD("Failed to create noti");
		return;
	}
#ifdef OLD
	if (!m_item->getThumbnailPath().empty()) {
		err = notification_set_image(notiHandle,
				NOTIFICATION_IMAGE_TYPE_ICON,
				m_item->getThumbnailPath().c_str());
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to set thumbnail image [%d]", err);
			freeNotiData(notiHandle);
			return;
		}
	} else {
		err = notification_set_image(notiHandle,
				NOTIFICATION_IMAGE_TYPE_ICON, m_item->getIconPath().c_str());
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to set icon [%d]", err);
			freeNotiData(notiHandle);
			return;
		}
	}
#endif

    err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON,DM_NOTI_COMPLETED_QUICKPANNEL_ICON_PATH);
    if (err != NOTIFICATION_ERROR_NONE) {
                DM_LOGE("Fail to set icon [%d]", err);
                freeNotiData(notiHandle);
                return;
    }

	err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
			DM_NOTI_COMPLETED_INDICATOR_ICON_PATH);

	DM_LOGE("[yjkim] path : [%s]", DM_NOTI_COMPLETED_INDICATOR_ICON_PATH);

	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("[yjkim] Fail to set icon [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	if (app_control_create(&handle) != APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to create app_control");
		freeNotiData(notiHandle);
		return;
	}

	if (app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW) !=
			APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to app_control set operation");
		app_control_destroy(handle);
		freeNotiData(notiHandle);
		return;
	}
	if (app_control_set_uri(handle, m_item->getRegisteredFilePath().c_str()) !=
			APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to app_control set uri");
		app_control_destroy(handle);
		freeNotiData(notiHandle);
		return;
	}
    if (app_control_foreach_extra_data(handle, __app_control_extra_data_cb, NULL) != APP_CONTROL_ERROR_NONE) {
        DM_LOGE("Fail to app_control_foreach_extra_data");
		app_control_destroy(handle);
		freeNotiData(notiHandle);
		return;
	}
    err = notification_set_launch_option(notiHandle, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, (void *)handle);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set apps [%d]", err);
		freeNotiData(notiHandle);
		app_control_destroy(handle);
		return;
	}
	if (app_control_destroy(handle) != APP_CONTROL_ERROR_NONE)
		DM_LOGE("Failed to destroy the app_control");

	err = notification_set_property(notiHandle,
			NOTIFICATION_PROP_DISABLE_TICKERNOTI);

	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set property [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

    err = notification_post(notiHandle);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to insert [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	freeNotiData(notiHandle);
}

void DownloadNoti::addFailedNoti()
{
	notification_h notiHandle = NULL;
	int err = NOTIFICATION_ERROR_NONE;
	app_control_h handle;

	if (!m_item) {
		DM_LOGE("m_item is NULL");
		return;
	}
	if((notiHandle = createNoti(NOTIFICATION_TYPE::NOTI_FAILED)) == NULL) {
		DM_LOGD("Failed to create noti");
		return;
	}

	err = notification_set_image(notiHandle,
			NOTIFICATION_IMAGE_TYPE_ICON, DM_NOTI_FAILED_QUICKPANNEL_ICON_PATH);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set icon [%d]", err);
		freeNotiData(notiHandle);
		return;
	}
	err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
			DM_NOTI_FAILED_INDICATOR_ICON_PATH);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set icon [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	if (app_control_create(&handle) != APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to create app_control");
		freeNotiData(notiHandle);
		return;
	}
	char buff[MAX_BUF_LEN] = {0,};
	if (app_control_set_app_id(handle, PACKAGE_NAME) !=
			APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to app_control set pkgname");
		app_control_destroy(handle);
		freeNotiData(notiHandle);
		return;
	}
	snprintf(buff, MAX_BUF_LEN, "%u", m_item->getHistoryId());
	if (app_control_add_extra_data(handle, KEY_FAILED_HISTORYID,
			(const char *)buff) != APP_CONTROL_ERROR_NONE) {
		DM_LOGE("Fail to app_control set extra param data");
		app_control_destroy(handle);
		freeNotiData(notiHandle);
		return;
	}
    if (app_control_foreach_extra_data(handle, __app_control_extra_data_cb, NULL) != APP_CONTROL_ERROR_NONE) {
        DM_LOGE("Fail to app_control_foreach_extra_data");
		app_control_destroy(handle);
		freeNotiData(notiHandle);
		return;
	}
    err = notification_set_launch_option(notiHandle, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, (void *)handle);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set apps [%d]", err);
		freeNotiData(notiHandle);
		app_control_destroy(handle);
		return;
	}
	if (app_control_destroy(handle) != APP_CONTROL_ERROR_NONE)
		DM_LOGE("Failed to destroy the app_control");

	err = notification_set_property(notiHandle,
			NOTIFICATION_PROP_DISABLE_TICKERNOTI |
			NOTIFICATION_PROP_DISABLE_AUTO_DELETE);

	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set property [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

    err = notification_post(notiHandle);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to insert [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	freeNotiData(notiHandle);
}

void DownloadNoti::deleteCompleteNoti()
{
	int err = NOTIFICATION_ERROR_NONE;
	if (!m_item) {
		DM_LOGE("m_item is NULL");
		return;
	}
    err = notification_delete(m_notiHandle);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to delete [%d]", err);
	}
	DM_LOGI("delete historyID[%d]", m_item->getHistoryId());
}

bool DownloadNoti::__app_control_extra_data_cb(app_control_h service, const char *key, void *)
{
    char *value;
    int ret;

    ret = app_control_get_extra_data(service, key, &value);
    if (ret) {
        DM_LOGE("app_control_get_extra_data: error get data(%d)\n", ret);
        return false;
    }

    DM_LOGD("extra data : %s, %s\n", key, value);
    free(value);

    return true;
}

void DownloadNoti::clearOngoingNoti()
{
	int err = NOTIFICATION_ERROR_NONE;
    err = notification_delete_all(NOTIFICATION_TYPE_ONGOING);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to clear notificaton [%d]", err);
	}
}
