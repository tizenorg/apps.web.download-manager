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

#include "appsvc.h"
#include "download-manager-notification.h"

DownloadNoti::DownloadNoti(Item *item)
	: m_notiId(0)
	, m_notiHandle(NULL)
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

	DM_LOGV("state:[%d]", m_item->state());
	switch(m_item->state()) {
	case ITEM::REQUESTING:
	case ITEM::QUEUED:
		DM_LOGD("REQUESTING or QUEUED");
		if (m_notiId <= 0)
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
		msg = S_("IDS_COM_POP_DOWNLOAD_FAILED");
		if (m_notiHandle) {
			freeNotiData(m_notiHandle);
			m_notiHandle = NULL;
		}
		addCompleteNoti(msg, false);
		break;
	case ITEM::REGISTERING_TO_SYSTEM:
		DM_LOGD("REGISTERING_TO_SYSTEM");
		break;
	case ITEM::FAIL_TO_DOWNLOAD:
		DM_LOGD("FAIL_TO_DOWNLOAD");
		msg = S_("IDS_COM_POP_DOWNLOAD_FAILED");
		if (m_notiHandle) {
			freeNotiData(m_notiHandle);
			m_notiHandle = NULL;
		}
		addCompleteNoti(msg, false);
		break;
	case ITEM::FINISH_DOWNLOAD:
		DM_LOGD("FINISH_DOWNLOAD");
		msg = __("IDS_DM_HEADER_DOWNLOAD_COMPLETE");
		if (m_notiHandle) {
			freeNotiData(m_notiHandle);
			m_notiHandle = NULL;
		}
		addCompleteNoti(msg, true);
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
	notification_error_e err = NOTIFICATION_ERROR_NONE;

	if (notiHandle) {
		err = notification_free(notiHandle);
		if (err != NOTIFICATION_ERROR_NONE)
			DM_LOGE("Fail to free noti data [%d]",err);
			notiHandle = NULL;
	}
}

void DownloadNoti::addOngoingNoti()
{
	notification_h notiHandle = NULL;
	notification_error_e err = NOTIFICATION_ERROR_NONE;
	int privId = 0;

	if (!m_item) {
		DM_LOGE("NULL Check:item");
		return;
	}
	DM_LOGV("downloadId[%d]", m_item->id());
	DM_SLOGD("title[%s]", m_item->title().c_str());

	notiHandle = notification_create(NOTIFICATION_TYPE_ONGOING);

	if (!notiHandle) {
		DM_LOGE("Fail to create notification handle");
		return;
	}
	if (m_item->title().empty())
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
				S_("IDS_COM_BODY_NO_NAME"), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
				m_item->title().c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
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

	err = notification_insert(notiHandle, &privId);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to insert [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	m_notiId = privId;
	m_notiHandle = notiHandle;
	DM_LOGI("m_notiId [%d]", m_notiId);

	return;
}

void DownloadNoti::updateTitleOngoingNoti()
{
	DM_LOGD("");
	if (m_notiHandle) {
		notification_error_e err = NOTIFICATION_ERROR_NONE;
		DM_SLOGD("title[%s]", m_item->title().c_str());
		err = notification_set_text(m_notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
						m_item->title().c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to set title [%d]", err);
			freeNotiData(m_notiHandle);
			return;
		}
		err = notification_update(m_notiHandle);
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to update [%d]", err);
			freeNotiData(m_notiHandle);
			return;
		}
	}
}

void DownloadNoti::updateOngoingNoti(void)
{
	notification_error_e err = NOTIFICATION_ERROR_NONE;
	if (m_item->fileSize() > 0) {
		double progress = (double)m_item->receivedFileSize() / (double)m_item->fileSize();
		err = notification_update_progress(NULL, m_notiId, progress);
		if (err != NOTIFICATION_ERROR_NONE)
			DM_LOGE("Fail to update noti progress[%d]", err);
	} else {
		err = notification_update_size(NULL, m_notiId, m_item->receivedFileSize());
		if (err != NOTIFICATION_ERROR_NONE)
			DM_LOGE("Fail to update noti progress[%d]", err);
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

void DownloadNoti::addCompleteNoti(string &msg, bool isSuccess)
{
	notification_h notiHandle = NULL;
	notification_error_e err = NOTIFICATION_ERROR_NONE;
	int privId = 0;
	bundle *b = NULL;

	if (!m_item) {
		DM_LOGE("m_item is NULL");
		return;
	}

	DM_LOGI("historyId [%d], downloadId[%d]",
			m_item->historyId(), m_item->id());
	DM_SLOGD("title[%s]", m_item->title().c_str());

	/* Remove ongoing noti */
	err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_ONGOING,
			m_notiId);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to delete ongoing noti [%d]", err);
	}
	m_notiId = 0;

	notiHandle = notification_create(NOTIFICATION_TYPE_NOTI);

	if (!notiHandle) {
		DM_LOGE("Fail to insert notification");
		return;
	}
	if (m_item->title().empty())
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_CONTENT,
				S_("IDS_COM_BODY_NO_NAME"), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_CONTENT,
				m_item->title().c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set content name [%d]", err);
		freeNotiData(notiHandle);
		return;
	}
	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
			msg.c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set status [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	if (isSuccess) {
		string sizeStr = convertSizeStr(m_item->receivedFileSize());
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_INFO_1,
				sizeStr.c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to set size [%d]", err);
			freeNotiData(notiHandle);
			return;
		}
		if (!m_item->thumbnailPath().empty()) {
			err = notification_set_image(notiHandle,
					NOTIFICATION_IMAGE_TYPE_BACKGROUND,
					m_item->thumbnailPath().c_str());
			if (err != NOTIFICATION_ERROR_NONE) {
				DM_LOGE("Fail to set bg thumbnail image [%d]", err);
				freeNotiData(notiHandle);
				return;
			}
		} else {
			err = notification_set_image(notiHandle,
					NOTIFICATION_IMAGE_TYPE_ICON, DM_NOTI_COMPLETED_ICON_PATH);
			if (err != NOTIFICATION_ERROR_NONE) {
				DM_LOGE("Fail to set icon [%d]", err);
				freeNotiData(notiHandle);
				return;
			}
		}
		err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
				DM_NOTI_COMPLETED_ICON_PATH);
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to set icon [%d]", err);
			freeNotiData(notiHandle);
			return;
		}
	} else {
		err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON,
				DM_NOTI_FAILED_ICON_PATH);
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to set icon [%d]", err);
			freeNotiData(notiHandle);
			return;
		}
		err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR,
				DM_NOTI_FAILED_ICON_PATH);
		if (err != NOTIFICATION_ERROR_NONE) {
			DM_LOGE("Fail to set icon [%d]", err);
			freeNotiData(notiHandle);
			return;
		}
	}

	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_INFO_2,
			m_item->sender().c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set sender [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	err = notification_set_time_to_text(notiHandle,
			NOTIFICATION_TEXT_TYPE_INFO_SUB_1, (time_t)(m_item->finishedTime()));

	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set time [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	b = bundle_create();
	if (!b) {
		DM_LOGE("Fail to create bundle");
		freeNotiData(notiHandle);
		return;
	}

	if (isSuccess) {
		if (appsvc_set_operation(b, APPSVC_OPERATION_VIEW) != APPSVC_RET_OK) {
			DM_LOGE("Fail to appsvc set operation");
			bundle_free(b);
			freeNotiData(notiHandle);
			return;
		}
		if (appsvc_set_uri(b, m_item->registeredFilePath().c_str()) !=
				APPSVC_RET_OK) {
			DM_LOGE("Fail to appsvc set uri");
			bundle_free(b);
			freeNotiData(notiHandle);
			return;
		}
		err = notification_set_execute_option(notiHandle,
				NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, "View", NULL, b);
	} else {
			char buff[MAX_BUF_LEN] = {0,};
			if (appsvc_set_pkgname(b, PACKAGE_NAME) !=
					APPSVC_RET_OK) {
				DM_LOGE("Fail to appsvc set pkgname");
				bundle_free(b);
				freeNotiData(notiHandle);
				return;
			}
			snprintf(buff, MAX_BUF_LEN, "%u", m_item->historyId());
			if (appsvc_add_data(b, KEY_FAILED_HISTORYID,
					(const char *)buff) != APPSVC_RET_OK) {
				DM_LOGE("Fail to appsvc set extra param data");
				bundle_free(b);
				freeNotiData(notiHandle);
				return;
			}

			err = notification_set_execute_option(notiHandle,
					NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, "Launch", NULL, b);
	}
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set apps [%d]", err);
		freeNotiData(notiHandle);
		bundle_free(b);
		return;
	}

	bundle_free(b);

	if (isSuccess) {
		err = notification_set_property(notiHandle,
				NOTIFICATION_PROP_DISABLE_TICKERNOTI);
	} else {
		err = notification_set_property(notiHandle,
				NOTIFICATION_PROP_DISABLE_TICKERNOTI |
				NOTIFICATION_PROP_DISABLE_AUTO_DELETE);
	}

	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to set property [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	err = notification_insert(notiHandle, &privId);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to insert [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	DM_LOGI("priv id [%d]", privId);
	m_notiId = privId;

	freeNotiData(notiHandle);
}

void DownloadNoti::deleteCompleteNoti()
{
	notification_error_e err = NOTIFICATION_ERROR_NONE;
	if (!m_item) {
		DM_LOGE("m_item is NULL");
		return;
	}
	err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI,
			m_notiId);

	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to delete [%d]", err);
	}
	DM_LOGI("delete historyID[%d] m_id[%d]", m_item->historyId(), m_notiId);
	m_notiId = 0;
}

void DownloadNoti::clearOngoingNoti()
{
	DM_LOGD("");

	/* If the application was terminated abnormaly before or
	    when the application is terminated
	   NULL (first param) means caller process */
	notification_error_e err = NOTIFICATION_ERROR_NONE;
	err = notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_ONGOING);
	if (err != NOTIFICATION_ERROR_NONE) {
		DM_LOGE("Fail to clear notificaton [%d]", err);
	}
}
