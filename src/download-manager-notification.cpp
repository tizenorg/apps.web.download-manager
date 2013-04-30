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

#ifdef _CAPI_NOTI
#else
#include "appsvc.h"
#endif

#include "download-manager-notification.h"

DownloadNoti::DownloadNoti(Item *item)
#ifdef _CAPI_NOTI
	: m_notiHandle(NULL)
#else
	: m_noti_id(0)
#endif
	, m_item(item)
{
	DP_LOGV_FUNC();

	if (item) {
		m_aptr_observer = auto_ptr<Observer>(
			new Observer(updateCB, this, "notiObserver"));
		item->subscribe(m_aptr_observer.get());
	}
}

DownloadNoti::~DownloadNoti()
{
	DP_LOGD_FUNC();
#ifdef _CAPI_NOTI
	destoryNotiHandle();
#endif
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

	DP_LOGV("DownloadNoti::updateFromItem() ITEM::[%d]", m_item->state());
	switch(m_item->state()) {
	case ITEM::REQUESTING:
	case ITEM::REGISTERING_TO_SYSTEM:
		break;
	case ITEM::PREPARE_TO_RETRY:
		DP_LOGD("DownloadNoti:: ITEM::PREPARE_TO_RETRY");
		/* In case of retry, previous noti is deletead and
		 * will add ongoing notification */
		deleteCompleteNoti();
		break;
	case ITEM::DOWNLOADING:
		DP_LOGV("DownloadNoti::updateFromItem() ITEM::DOWNLOADING");
#ifdef _CAPI_NOTI
		if (!m_notiHandle)
			addOngoingNoti();
#else
		if (m_noti_id <= 0)
			addOngoingNoti();
#endif
		updateOngoingNoti();
		break;
	case ITEM::CANCEL:
		DP_LOGD("DownloadNoti::updateFromItem() ITEM::CANCEL");
#ifdef _BOX_NOTI_TYPE
		msg = S_("IDS_COM_POP_DOWNLOAD_FAILED");
#else
		msg = S_("IDS_COM_POP_CANCELLED");
#endif

		addCompleteNoti(msg, false);
		break;
	case ITEM::FAIL_TO_DOWNLOAD:
		DP_LOGD("DownloadNoti::updateFromItem() ITEM::FAIL_TO_DOWNLOAD");
		msg = S_("IDS_COM_POP_DOWNLOAD_FAILED");
		addCompleteNoti(msg, false);
		break;
	case ITEM::FINISH_DOWNLOAD:
		DP_LOGD("DownloadNoti::updateFromItem() ITEM::FINISH_DOWNLOAD");
		msg = __("IDS_RH_POP_DOWNLOAD_COMPLETE");
		addCompleteNoti(msg, true);
		break;
	case ITEM::DESTROY:
		DP_LOGD("DownloadNoti::updateFromItem() ITEM::DESTROY");
		if (m_item)
			m_item->deSubscribe(m_aptr_observer.get());
		m_aptr_observer->clear();
		break;
	default:
		break;
	}
}

#ifdef _CAPI_NOTI
#else
void DownloadNoti::freeNotiData(notification_h notiHandle)
{
	DP_LOGV_FUNC();
	notification_error_e err = NOTIFICATION_ERROR_NONE;

	if (notiHandle) {
		err = notification_free(notiHandle);
		if (err != NOTIFICATION_ERROR_NONE)
			DP_LOGE("Fail to free noti data [%d]",err);
			notiHandle = NULL;
	}
}
#endif

void DownloadNoti::addOngoingNoti()
{
#ifdef _CAPI_NOTI
	service_h serviceHandle;
#else
	notification_h notiHandle = NULL;
	notification_error_e err = NOTIFICATION_ERROR_NONE;
	int privId = 0;
#endif

	if (!m_item) {
		DP_LOGE("m_item is NULL");
		return;
	}
	DP_LOGD("DownloadNoti::addOngoingNoti downloadId[%d] title[%s]",
			m_item->id(), m_item->title().c_str());
#ifdef _CAPI_NOTI
	if (ui_notification_create(true, &m_notiHandle) < 0) {
		DP_LOGE("Fail to create notification handle");
		return;
	}
	if (m_item->title().empty()) {
		if (ui_notification_set_title(m_notiHandle,
				S_("IDS_COM_BODY_NO_NAME")) < 0) {
			DP_LOGE("Fail to set title at notification handle");
			destoryNotiHandle();
			return;
		}
	} else {
		if (ui_notification_set_title(m_notiHandle, m_item->title().c_str()) < 0) {
			DP_LOGE("Fail to set title at notification handle");
			destoryNotiHandle();
			return;
		}
	}

	if (ui_notification_set_icon(m_notiHandle, DP_NOTIFICATION_ICON_PATH) < 0) {
		DP_LOGE("Fail to set icon at notification handle");
		destoryNotiHandle();
		return;
	}

	serviceHandle = getServiceHandle(false);
	if (!serviceHandle) {
		DP_LOGE("Fail to get service handle data");
		destoryNotiHandle();
		return;
	}

	if (ui_notification_set_service(m_notiHandle, serviceHandle) < 0) {
		DP_LOGE("Fail to set service handle at notification handle");
		service_destroy(serviceHandle);
		destoryNotiHandle();
		return;
	}

	if (ui_notification_post(m_notiHandle) < 0) {
		DP_LOGE("Fail to post notification");
		service_destroy(serviceHandle);
		destoryNotiHandle();
		return;
	}

	if (service_destroy(serviceHandle) < 0) {
		DP_LOGE("Fail to set service uri");
	}

	DP_LOGD("noti handle [%p]",m_notiHandle);
#else

	notiHandle = notification_create(NOTIFICATION_TYPE_ONGOING);

	if (!notiHandle) {
		DP_LOGE("Fail to create notification handle");
		return;
	}
	if (m_item->title().empty())
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
				S_("IDS_COM_BODY_NO_NAME"), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
				m_item->title().c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set title [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON,
			DP_NOTIFICATION_ICON_PATH);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set icon [%d]", err);
		freeNotiData(notiHandle);
		return;
	}
#if 0
	/* Test code */
	b = bundle_create();
	if (!b) {
		DP_LOGE("Fail to create bundle");
		freeNotiData(notiHandle);
		return;
	}

//	if (appsvc_set_pkgname(b, PACKAGE_NAME) !=
	if (appsvc_set_pkgname(b, "com.samsung.download-manager") !=
			APPSVC_RET_OK) {
		DP_LOGE("Fail to appsvc set pkgname");
		bundle_free(b);
		freeNotiData(notiHandle);
		return;
	}
	if (appsvc_add_data(b,"mode","view") != APPSVC_RET_OK) {
		DP_LOGE("Fail to appsvc add data");
		bundle_free(b);
		freeNotiData(notiHandle);
		return;
	}
	/* End Test code */

	err = notification_set_execute_option(notiHandle,
			NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH, "Launch", NULL, b);
//	err = notification_set_execute_option(notiHandle, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, b);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set args [%d]", err);
		bundle_free(b);
		freeNotiData(notiHandle);
		return;
	}
	bundle_free(b);
#endif

	err = notification_set_property(notiHandle, NOTIFICATION_PROP_DISABLE_TICKERNOTI);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set property [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	err = notification_insert(notiHandle, &privId);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to insert [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	m_noti_id = privId;
	DP_LOGD("m_noti_id [%d]", m_noti_id);
	freeNotiData(notiHandle);

#endif
}

void DownloadNoti::updateOngoingNoti(void)
{
#ifdef _CAPI_NOTI
	if (!m_notiHandle) {
		DP_LOGE("noti handle is NULL");
		return;
	}
	if (m_item->fileSize() > 0) {
		double progress = (double)m_item->receivedFileSize() / (double)m_item->fileSize();
		if (ui_notification_update_progress(
				m_notiHandle, UI_NOTIFICATION_PROGRESS_TYPE_PERCENTAGE,
				progress) < 0) {
			DP_LOGE("Fail to update progress at notification handle");
			destoryNotiHandle();
		}
	} else {
		if (ui_notification_update_progress(
				m_notiHandle, UI_NOTIFICATION_PROGRESS_TYPE_SIZE,
				m_item->receivedFileSize()) < 0) {
			DP_LOGE("Fail to update progress at notification handle");
			destoryNotiHandle();
		}
	}
#else
	notification_error_e err = NOTIFICATION_ERROR_NONE;
	if (m_item->fileSize() > 0) {
		double progress = (double)m_item->receivedFileSize() / (double)m_item->fileSize();
		err = notification_update_progress(NULL, m_noti_id, progress);
		if (err != NOTIFICATION_ERROR_NONE)
			DP_LOGE("Fail to update noti progress[%d]", err);
	} else {
		err = notification_update_size(NULL, m_noti_id, m_item->receivedFileSize());
		if (err != NOTIFICATION_ERROR_NONE)
			DP_LOGE("Fail to update noti progress[%d]", err);
	}
#endif
}
#ifdef _BOX_NOTI_TYPE
string DownloadNoti::convertSizeStr(unsigned long int size)
{
	const char *unitStr[4] = {"B", "KB", "MB", "GB"};
	double doubleTypeBytes = 0.0;
	int unit = 0;
	unsigned long int bytes = size;
	unsigned long int unitBytes = bytes;
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
		snprintf(str, sizeof(str), "%lu %s", bytes, unitStr[unit]);
	} else {
		doubleTypeBytes = ((double)bytes / (double)unitBytes);
		snprintf(str, sizeof(str), "%.2f %s", doubleTypeBytes, unitStr[unit]);
	}

	str[63] = '\0';
	temp = string(str);
	DP_LOGV("size[%ld]",size);
	return temp;
}
#endif

void DownloadNoti::addCompleteNoti(string &msg, bool isFinished)
{
#ifdef _CAPI_NOTI
	service_h serviceHandle;

	if (m_notiHandle) {
		/* Remove ongoing noti */
		bool ongoing = false;
		if (ui_notification_is_ongoing(m_notiHandle, &ongoing) < 0)
			DP_LOGE("Fail to check ongoing noti[%p]", m_notiHandle);
		if (ongoing)
			if (ui_notification_cancel(m_notiHandle) < 0)
				DP_LOGE("Fail to cancel ongoing noti[%p]", m_notiHandle);
		destoryNotiHandle();
	}

	if (!m_item) {
		DP_LOGE("m_item is NULL");
		return;
	}
	DP_LOGD("DownloadNoti::addCompleteNoti historyId [%d], downloadId[%d], title[%s]",
		m_item->historyId(), m_item->id(), m_item->title().c_str());

	if (ui_notification_create(false, &m_notiHandle) < 0) {
		DP_LOGE("Fail to create notification handle");
		return;
	}
	if (m_item->title().empty()) {
		if (ui_notification_set_title(m_notiHandle,
				S_("IDS_COM_BODY_NO_NAME")) < 0) {
			DP_LOGE("Fail to set title at notification handle");
			destoryNotiHandle();
			return;
		}
	} else {
		if (ui_notification_set_title(m_notiHandle, m_item->title().c_str()) < 0) {
			DP_LOGE("Fail to set title at notification handle");
			destoryNotiHandle();
			return;
		}
	}

	if (ui_notification_set_content(m_notiHandle, msg.c_str()) < 0) {
		DP_LOGE("Fail to set content at notification handle");
		destoryNotiHandle();
		return;
	}

	if (ui_notification_set_icon(m_notiHandle, DP_NOTIFICATION_ICON_PATH) < 0) {
		DP_LOGE("Fail to set icon at notification handle");
		destoryNotiHandle();
		return;
	}

	time_t finishedTime = m_item->finishedTime();
	struct tm *localTime = localtime(&finishedTime);
	if (ui_notification_set_time(m_notiHandle, localTime) < 0) {
		DP_LOGE("Fail to set time at notification handle");
		destoryNotiHandle();
		return;
	}

	serviceHandle = getServiceHandle(isFinished);
	if (!serviceHandle) {
		DP_LOGE("Fail to get service handle data");
		destoryNotiHandle();
		return;
	}

	if (ui_notification_set_service(m_notiHandle, serviceHandle) < 0) {
		DP_LOGE("Fail to set service handle at notification handle");
		service_destroy(serviceHandle);
		destoryNotiHandle();
		return;
	}

	if (service_destroy(serviceHandle) < 0) {
		DP_LOGE("Fail to set service uri");
	}

	if (ui_notification_post(m_notiHandle) < 0) {
		DP_LOGE("Fail to post notification");
		destoryNotiHandle();
		return;
	}
	DP_LOGD("noti handle [%p]",m_notiHandle);
#else

	notification_h notiHandle = NULL;
	notification_error_e err = NOTIFICATION_ERROR_NONE;
	int privId = 0;
	bundle *b = NULL;

	if (!m_item) {
		DP_LOGE("m_item is NULL");
		return;
	}

	DP_LOGD("DownloadNoti::addNoti historyId [%d], downloadId[%d], title[%s]",
			m_item->historyId(), m_item->id(), m_item->title().c_str());

	/* Remove ongoing noti */
	err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_ONGOING,
			m_noti_id);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to delete ongoing noti [%d]", err);
	}
	m_noti_id = 0;


	notiHandle = notification_create(NOTIFICATION_TYPE_NOTI);

	if (!notiHandle) {
		DP_LOGE("Fail to insert notification");
		return;
	}
	if (m_item->title().empty())
#ifdef _BOX_NOTI_TYPE
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_INFO_1,
				S_("IDS_COM_BODY_NO_NAME"), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
#else
	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
			S_("IDS_COM_BODY_NO_NAME"), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
#endif
	else
#ifdef _BOX_NOTI_TYPE
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_INFO_1,
				m_item->title().c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
#else
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
				m_item->title().c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
#endif
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set content name [%d]", err);
		freeNotiData(notiHandle);
		return;
	}
#ifdef _BOX_NOTI_TYPE
	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_TITLE,
			msg.c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
#else
	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_CONTENT,
			msg.c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
#endif
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set status [%d]", err);
		freeNotiData(notiHandle);
		return;
	}
#ifdef _BOX_NOTI_TYPE
	if (isFinished) {
		string sizeStr = convertSizeStr(m_item->receivedFileSize());
		err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_INFO_1,
				sizeStr.c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		if (err != NOTIFICATION_ERROR_NONE) {
			DP_LOGE("Fail to set size [%d]", err);
			freeNotiData(notiHandle);
			return;
		}
		if (!m_item->thumbnailPath().empty()) {
			err = notification_set_image(notiHandle,
					NOTIFICATION_IMAGE_TYPE_BACKGROUND,
					m_item->thumbnailPath().c_str());
			if (err != NOTIFICATION_ERROR_NONE) {
				DP_LOGE("Fail to set bg thumbnail image [%d]", err);
				freeNotiData(notiHandle);
				return;
			}
		} else {
			err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON,
						DP_NOTIFICATION_ICON_PATH);
			if (err != NOTIFICATION_ERROR_NONE) {
				DP_LOGE("Fail to set icon [%d]", err);
				freeNotiData(notiHandle);
				return;
			}
		}
	} else {
		err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON,
					DP_NOTIFICATION_ICON_PATH);
		if (err != NOTIFICATION_ERROR_NONE) {
			DP_LOGE("Fail to set icon [%d]", err);
			freeNotiData(notiHandle);
			return;
		}
	}

	err = notification_set_text(notiHandle, NOTIFICATION_TEXT_TYPE_INFO_2,
			m_item->sender().c_str(), NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set sender [%d]", err);
		freeNotiData(notiHandle);
		return;
	}
#else
	err = notification_set_image(notiHandle, NOTIFICATION_IMAGE_TYPE_ICON,
			DP_NOTIFICATION_ICON_PATH);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set icon [%d]", err);
		freeNotiData(notiHandle);
		return;
	}
#endif

#ifdef _BOX_NOTI_TYPE
	err = notification_set_time_to_text(notiHandle,
			NOTIFICATION_TEXT_TYPE_INFO_SUB_1, (time_t)(m_item->finishedTime()));
#else
	err = notification_set_time(notiHandle, (time_t)(m_item->finishedTime()));
#endif
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set time [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	b = bundle_create();
	if (!b) {
		DP_LOGE("Fail to create bundle");
		freeNotiData(notiHandle);
		return;
	}

	if (isFinished) {
		if (appsvc_set_operation(b, APPSVC_OPERATION_VIEW) != APPSVC_RET_OK) {
			DP_LOGE("Fail to appsvc set operation");
			bundle_free(b);
			freeNotiData(notiHandle);
			return;
		}
		if (appsvc_set_uri(b, m_item->registeredFilePath().c_str()) !=
				APPSVC_RET_OK) {
			DP_LOGE("Fail to appsvc set uri");
			bundle_free(b);
			freeNotiData(notiHandle);
			return;
		}
		err = notification_set_execute_option(notiHandle,
				NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, "View", NULL, b);
	} else {
			if (appsvc_set_pkgname(b, PACKAGE_NAME) !=
					APPSVC_RET_OK) {
				DP_LOGE("Fail to appsvc set pkgname");
				bundle_free(b);
				freeNotiData(notiHandle);
				return;
			}
			err = notification_set_execute_option(notiHandle,
					NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH, "Launch", NULL, b);

	}
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set apps [%d]", err);
		freeNotiData(notiHandle);
		bundle_free(b);
		return;
	}

	bundle_free(b);

	err = notification_set_property(notiHandle, NOTIFICATION_PROP_DISABLE_TICKERNOTI);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to set property [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	err = notification_insert(notiHandle, &privId);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to insert [%d]", err);
		freeNotiData(notiHandle);
		return;
	}

	DP_LOGD("priv id [%d]", privId);
	m_noti_id = privId;

	freeNotiData(notiHandle);

#endif
}

#ifdef _CAPI_NOTI
service_h DownloadNoti::getServiceHandle(bool isFinished)
{
	service_h serviceHandle;

	if (service_create(&serviceHandle) < 0) {
		DP_LOGE("Fail to set service uri");
		return NULL;
	}

	if (isFinished) {
		if (service_set_operation(serviceHandle, SERVICE_OPERATION_VIEW) < 0) {
			DP_LOGE("Fail to set service operation");
			service_destroy(serviceHandle);
			return NULL;
		}

		if (service_set_uri(serviceHandle, m_item->registeredFilePath().c_str()) < 0) {
			DP_LOGE("Fail to set service uri");
			service_destroy(serviceHandle);
			return NULL;
		}
	} else {
		char *app_id = NULL;
		if (app_get_id(&app_id) < 0) {
			DP_LOGE("Fail to get app id");
			service_destroy(serviceHandle);
			return NULL;
		}
		DP_LOGD("app id [%s]", app_id);
		if (service_set_app_id(serviceHandle, app_id) < 0) {
			DP_LOGE("Fail to set service id");
			service_destroy(serviceHandle);
			if (app_id)
				free(app_id);
			return NULL;
		}
		if (app_id)
			free(app_id);
	}
	return serviceHandle;
}
#endif

void DownloadNoti::deleteCompleteNoti()
{
#ifdef _CAPI_NOTI
#else
	notification_error_e err = NOTIFICATION_ERROR_NONE;
#endif
	if (!m_item) {
		DP_LOGE("m_item is NULL");
		return;
	}
#ifdef _CAPI_NOTI
	if (ui_notification_cancel(m_notiHandle) < 0)
		DP_LOGE("Fail to cancel ongoing noti[%p]", m_notiHandle);
	destoryNotiHandle();

	DP_LOGD("delete historyID[%d] m_notiHandle[%d]",m_item->historyId(), m_notiHandle);
#else

	err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI,
			m_noti_id);
	m_noti_id = 0;

	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to delete [%d]", err);
	}
	DP_LOGD("delete historyID[%d] m_id[%d]", m_item->historyId(), m_noti_id);
#endif
}

void DownloadNoti::clearOngoingNoti()
{
	DP_LOGD_FUNC();

	/* If the application was terminated abnormaly before or
	    when the application is terminated
	   NULL (first param) means caller process */
#ifdef _CAPI_NOTI
	ui_notification_cancel_all_by_type(true);
#else
	notification_error_e err = NOTIFICATION_ERROR_NONE;
	err = notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_ONGOING);
	if (err != NOTIFICATION_ERROR_NONE) {
		DP_LOGE("Fail to clear notificaton [%d]", err);
	}
#endif

}

#ifdef _CAPI_NOTI
void DownloadNoti::destoryNotiHandle()
{
	DP_LOGD_FUNC();
	if (ui_notification_destroy(m_notiHandle) < 0)
		DP_LOGE("Fail to destory handle [%p]", m_notiHandle);
	m_notiHandle = 0;
}
#endif
