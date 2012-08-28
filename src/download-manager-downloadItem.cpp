/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file	download-manager-downloadItem.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	download item class for interface of download agent
 */

#include <Ecore.h>
#include <iostream>
#include "download-manager-downloadItem.h"
#include "download-manager-common.h"
#include "app_service.h"

static Ecore_Pipe *ecore_pipe = NULL;
static void __ecore_cb_pipe_update(void *data, void *buffer, unsigned int nbyte);

namespace DA_CB {
enum TYPE {
	STARTED = 1,
	PROGRESS,
	PAUSED,
	COMPLETED,
	STOPPED
};
}

class CbData {
public:
	CbData() {}
	~CbData() {}

	void updateDownloadItem();

	inline void setType(DA_CB::TYPE type) { m_type = type; }
	inline void setUserData(void *userData) { m_userData = userData; }
	inline void setDownloadHandle(url_download_h handle) { m_download_handle = handle; }
	inline void setReceivedFileSize(unsigned long int size) { m_receivedFileSize = size; }
	inline void setFileSize(unsigned long int size) { m_fileSize = size; }
	inline void setContentName(const char *name) { if (name) m_contentName = name; }
	inline void setRegisteredFilePath(const char *path) { if (path) m_registeredFilePath = path; }
	inline void setMimeType(const char *mime) { m_mimeType = mime; }
	inline void setErrorCode(int err) { m_error = err;	}

private:
	DA_CB::TYPE m_type;
	void *m_userData;
	url_download_h m_download_handle;
	int m_error;
	unsigned long int m_receivedFileSize;
	unsigned long int m_fileSize;
	string m_contentName;
	string m_registeredFilePath;
	string m_mimeType;
};

struct pipe_data_t {
	CbData *cbData;
};

DownloadEngine::DownloadEngine()
{
}

DownloadEngine::~DownloadEngine()
{
	DP_LOG_FUNC();
}

void DownloadEngine::initEngine(void)
{
	ecore_pipe = ecore_pipe_add(__ecore_cb_pipe_update, NULL);
}

void DownloadEngine::deinitEngine(void)
{
	DP_LOG_FUNC();
	if (ecore_pipe) {
		ecore_pipe_del(ecore_pipe);
		ecore_pipe = NULL;
	}
}

void CbData::updateDownloadItem()
{
//	DP_LOGD_FUNC();

	if (!m_userData) {
		DP_LOGE("download item is NULL");
		return;
	}

	DownloadItem *downloadItem = static_cast<DownloadItem*>(m_userData);
	if (downloadItem->state() == DL_ITEM::FAILED) {
		DP_LOGE("download item is already failed");
		return;
	}
	downloadItem->setDownloadHandle(m_download_handle);

	switch(m_type) {
	case DA_CB::STARTED:
		downloadItem->setState(DL_ITEM::STARTED);
		//downloadItem->setFileSize(m_fileSize);
		if (!m_contentName.empty())
			downloadItem->setContentName(m_contentName);
		if (!m_mimeType.empty())
			downloadItem->setMimeType(m_mimeType);
		break;
	case DA_CB::PROGRESS:
		downloadItem->setState(DL_ITEM::UPDATING);
		downloadItem->setFileSize(m_fileSize);
		downloadItem->setReceivedFileSize(m_receivedFileSize);
		break;
	case DA_CB::PAUSED:
		downloadItem->setState(DL_ITEM::SUSPENDED);
		downloadItem->setFileSize(m_fileSize);
		downloadItem->setReceivedFileSize(m_receivedFileSize);
		break;
	case DA_CB::COMPLETED:
		downloadItem->setState(DL_ITEM::FINISHED);
		if (!m_registeredFilePath.empty()) {
			DP_LOGD("registeredFilePath[%s]", m_registeredFilePath.c_str());
			downloadItem->setRegisteredFilePath(m_registeredFilePath);
		}
		downloadItem->destroyHandle();
		break;
	case DA_CB::STOPPED:
		if (m_error != URL_DOWNLOAD_ERROR_NONE) {
			downloadItem->setState(DL_ITEM::FAILED);
			downloadItem->setErrorCode(downloadItem->_convert_error(m_error));
		} else {
			downloadItem->setState(DL_ITEM::CANCELED);
		}
		downloadItem->destroyHandle();
		break;
	default:
		break;
	}
	downloadItem->notify();
}

void __ecore_cb_pipe_update(void *data, void *buffer, unsigned int nbyte)
{
//	DP_LOGD_FUNC();

	if (!buffer)
		return;
	pipe_data_t *pipe_data = static_cast<pipe_data_t *>(buffer);
	CbData *cbData = pipe_data->cbData;
	if (!cbData)
		return;

	cbData->updateDownloadItem();
	delete cbData;
}

DownloadItem::DownloadItem()
	: m_download_handle(NULL)
	, m_state(DL_ITEM::IGNORE)
	, m_errorCode(ERROR::NONE)
	, m_receivedFileSize(0)
	, m_fileSize(0)
	, m_downloadType(DL_TYPE::HTTP_DOWNLOAD)
{
}

DownloadItem::DownloadItem(auto_ptr<DownloadRequest> request)
	: m_aptr_request(request)
	, m_download_handle(NULL)
	, m_state(DL_ITEM::IGNORE)
	, m_errorCode(ERROR::NONE)
	, m_receivedFileSize(0)
	, m_fileSize(0)
	, m_downloadType(DL_TYPE::HTTP_DOWNLOAD)
{
}

void DownloadItem::createHandle()
{
	int ret = url_download_create(&m_download_handle);
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to create download handle : [%d]", ret);
		return;
	}
	DP_LOGD("URL download handle : [%p]", m_download_handle);
	ret = url_download_set_started_cb(m_download_handle, started_cb, static_cast<void*>(this));
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to set started callback : [%d]", ret);
		return;
	}

	ret = url_download_set_completed_cb(m_download_handle, completed_cb, static_cast<void*>(this));
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to set completed cb : [%d]", ret);
		return;
	}

	ret = url_download_set_paused_cb(m_download_handle, paused_cb, static_cast<void*>(this));
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to set paused cb : [%d]", ret);
		return;
	}

	ret = url_download_set_stopped_cb(m_download_handle, stopped_cb, static_cast<void*>(this));
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to set stopped cb : [%d]", ret);
		return;
	}

	ret = url_download_set_progress_cb(m_download_handle, progress_cb, static_cast<void*>(this));
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to set progress cb : [%d]", ret);
		return;
	}

	service_h service_handle;
	ret = service_create(&service_handle);
	if (ret < 0) {
		DP_LOGE("Fail to create service handle");
		return;
	}

	ret = service_set_package(service_handle, PACKAGE_NAME);
	if (ret < 0) {
		DP_LOGE("Fail to set package name");
		return;
	}
	ret = url_download_set_notification(m_download_handle, service_handle);
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to set notification : [%d]", ret);
		return;
	}
	ret = service_destroy(service_handle);
	if (ret < 0) {
		DP_LOGE("Fail to create service handle");
		return;
	}
}

DownloadItem::~DownloadItem()
{
	DP_LOGD_FUNC();
	destroyHandle();
}

void DownloadItem::destroyHandle()
{
	if (!m_download_handle)
		return;
	DP_LOGD("download handle[%p]", m_download_handle);
	url_download_unset_started_cb(m_download_handle);
	url_download_unset_completed_cb(m_download_handle);
	url_download_unset_paused_cb(m_download_handle);
	url_download_unset_stopped_cb(m_download_handle);
	url_download_unset_progress_cb(m_download_handle);
	url_download_destroy(m_download_handle);
	m_download_handle = NULL;
}

void DownloadItem::started_cb(url_download_h download, const char *name,
	const char *mime, void *user_data)
{

	CbData *cbData = new CbData();
	cbData->setType(DA_CB::STARTED);
	cbData->setDownloadHandle(download);
	cbData->setUserData(user_data);
	if (name)
		cbData->setContentName(name);
	if (mime)
		cbData->setMimeType(mime);

	pipe_data_t pipe_data;
	pipe_data.cbData = cbData;
	ecore_pipe_write(ecore_pipe, &pipe_data, sizeof(pipe_data_t));
}

void DownloadItem::paused_cb(url_download_h download, void *user_data)
{
	CbData *cbData = new CbData();
	cbData->setType(DA_CB::PAUSED);
	cbData->setDownloadHandle(download);
	cbData->setUserData(user_data);

	pipe_data_t pipe_data;
	pipe_data.cbData = cbData;
	ecore_pipe_write(ecore_pipe, &pipe_data, sizeof(pipe_data_t));
}

void DownloadItem::completed_cb(url_download_h download, const char *path,
	void *user_data)
{
	CbData *cbData = new CbData();
	cbData->setType(DA_CB::COMPLETED);
	cbData->setDownloadHandle(download);
	cbData->setUserData(user_data);
	cbData->setRegisteredFilePath(path);

	pipe_data_t pipe_data;
	pipe_data.cbData = cbData;
	ecore_pipe_write(ecore_pipe, &pipe_data, sizeof(pipe_data_t));
}

void DownloadItem::stopped_cb(url_download_h download, url_download_error_e error,
	void *user_data)
{
	CbData *cbData = new CbData();
	cbData->setType(DA_CB::STOPPED);
	cbData->setDownloadHandle(download);
	cbData->setUserData(user_data);
	cbData->setErrorCode(error);

	pipe_data_t pipe_data;
	pipe_data.cbData = cbData;
	ecore_pipe_write(ecore_pipe, &pipe_data, sizeof(pipe_data_t));
}

void DownloadItem::progress_cb(url_download_h download, unsigned long long received,
	unsigned long long total, void *user_data)
{
	CbData *cbData = new CbData();
	cbData->setType(DA_CB::PROGRESS);
	cbData->setDownloadHandle(download);
	cbData->setUserData(user_data);
	cbData->setFileSize(total);
	cbData->setReceivedFileSize(received);
// need to tmp path??

	pipe_data_t pipe_data;
	pipe_data.cbData = cbData;
	ecore_pipe_write(ecore_pipe, &pipe_data, sizeof(pipe_data_t));
}

void DownloadItem::start(bool isRetry)
{
	int ret = 0;
	DP_LOGD_FUNC();
	if (m_download_handle) {
		destroyHandle();
	}
	createHandle();

	ret = url_download_set_url(m_download_handle,
			m_aptr_request->getUrl().c_str());
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to set url : [%d]", ret);
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
		return;
	}
	if (!m_aptr_request->getCookie().empty()) {
		ret = url_download_add_http_header_field(m_download_handle,
				"Cookie", m_aptr_request->getCookie().c_str());
		if (ret != URL_DOWNLOAD_ERROR_NONE) {
			DP_LOGE("Fail to set cookie : [%d]", ret);
			m_state = DL_ITEM::FAILED;
			m_errorCode = ERROR::ENGINE_FAIL;
			notify();
			return;
		}
	}
	ret = url_download_start(m_download_handle, &m_download_id);
	DP_LOGD("URL download handle : handle[%p]id[%d]", m_download_handle, m_download_id);

	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
	}
}

ERROR::CODE DownloadItem::_convert_error(int err)
{
	DP_LOGD("download module error[%d]", err);

	switch (err) {
	/*
	case URL_DOWNLOAD_ERROR_NONE:
	case URL_DOWNLOAD_ERROR_INVALID_PARAMETER:
	case URL_DOWNLOAD_ERROR_OUT_OF_MEMORY:
	case URL_DOWNLOAD_ERROR_IO_ERROR:
	case URL_DOWNLOAD_ERROR_FIELD_NOT_FOUND:
	case URL_DOWNLOAD_ERROR_INVALID_STATE:
	case URL_DOWNLOAD_ERROR_INVALID_DESTINATION:
	case URL_DOWNLOAD_ERROR_TOO_MANY_DOWNLOADS:
*/
	case URL_DOWNLOAD_ERROR_NETWORK_UNREACHABLE:
	case URL_DOWNLOAD_ERROR_CONNECTION_TIMED_OUT:
	case URL_DOWNLOAD_ERROR_CONNECTION_FAILED:
		return ERROR::NETWORK_FAIL;

	case URL_DOWNLOAD_ERROR_INVALID_URL:
		return ERROR::INVALID_URL;

	case URL_DOWNLOAD_ERROR_NO_SPACE:
		return ERROR::NOT_ENOUGH_MEMORY;

//		return ERROR::FAIL_TO_INSTALL;
	default :
		return ERROR::UNKNOWN;
	}

}

void DownloadItem::cancel()
{
	DP_LOGD("DownloadItem::cancel");
	if (m_state == DL_ITEM::CANCELED) {
		DP_LOGD("It is already canceled");
		notify();
		return;
	}
	int ret = url_download_stop(m_download_handle);
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to cancel download : handle[%p]  reason[%d]",
			m_download_handle, ret);
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
	}
	return;
}

void DownloadItem::retry()
{
	DP_LOGD_FUNC();
//	m_download_handle = NULL;
	m_state = DL_ITEM::IGNORE;
	m_errorCode = ERROR::NONE;
	m_receivedFileSize = 0;
	m_fileSize = 0;
	m_downloadType = DL_TYPE::HTTP_DOWNLOAD;
	start(true);
}

void DownloadItem::suspend()
{
	int ret = url_download_pause(m_download_handle);
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to suspend download : handle[%p] err[%d]",
			m_download_handle, ret);
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
	}
}

void DownloadItem::resume()
{
	int ret = url_download_start(m_download_handle, &m_download_id);
	if (ret != URL_DOWNLOAD_ERROR_NONE) {
		DP_LOGE("Fail to resume download : handle[%p] err[%d]",
			m_download_handle, ret);
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
	}
}
