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
 * @file	download-manager-downloadItem.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	download item class for interface of download agent
 */

#include <iostream>
#ifdef _ENABLE_OMA_DOWNLOAD
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "curl/curl.h"
#include "download-manager-network.h"
#endif
#include "download-manager-downloadItem.h"
#include "download-manager-common.h"
#include "download-manager-util.h"

#ifdef _ENABLE_OMA_DOWNLOAD
#define DD_MIME_STR "application/vnd.oma.dd+xml"
#endif

static Ecore_Pipe *ecore_pipe = NULL;
static void __ecore_cb_pipe_update(void *data, void *buffer, unsigned int nbyte);

#define CONV_STR(NAME) (#NAME)
static const char *__convertErrToString(int err)
{
	const char *str = NULL;
	switch(err) {
	case DOWNLOAD_ERROR_NONE:
		str = CONV_STR(DOWNLOAD_ERROR_NONE);
		break;
	case DOWNLOAD_ERROR_INVALID_PARAMETER:
		str = CONV_STR(DOWNLOAD_ERROR_INVALID_PARAMETER);
		break;
	case DOWNLOAD_ERROR_OUT_OF_MEMORY:
		str = CONV_STR(DOWNLOAD_ERROR_OUT_OF_MEMORY);
		break;
	case DOWNLOAD_ERROR_NETWORK_UNREACHABLE:
		str = CONV_STR(DOWNLOAD_ERROR_NETWORK_UNREACHABLE);
		break;
	case DOWNLOAD_ERROR_CONNECTION_TIMED_OUT:
		str = CONV_STR(DOWNLOAD_ERROR_CONNECTION_TIMED_OUT);
		break;
	case DOWNLOAD_ERROR_NO_SPACE:
		str = CONV_STR(DOWNLOAD_ERROR_NO_SPACE);
		break;
	case DOWNLOAD_ERROR_FIELD_NOT_FOUND:
		str = CONV_STR(DOWNLOAD_ERROR_FIELD_NOT_FOUND);
		break;
	case DOWNLOAD_ERROR_INVALID_STATE:
		str = CONV_STR(DOWNLOAD_ERROR_INVALID_STATE);
		break;
	case DOWNLOAD_ERROR_CONNECTION_FAILED:
		str = CONV_STR(DOWNLOAD_ERROR_CONNECTION_FAILED);
		break;
	case DOWNLOAD_ERROR_INVALID_URL:
		str = CONV_STR(DOWNLOAD_ERROR_INVALID_URL);
		break;
	case DOWNLOAD_ERROR_INVALID_DESTINATION:
		str = CONV_STR(DOWNLOAD_ERROR_INVALID_DESTINATION);
		break;
	case DOWNLOAD_ERROR_TOO_MANY_DOWNLOADS:
		str = CONV_STR(DOWNLOAD_ERROR_TOO_MANY_DOWNLOADS);
		break;
	case DOWNLOAD_ERROR_QUEUE_FULL:
		str = CONV_STR(DOWNLOAD_ERROR_QUEUE_FULL);
		break;
	case DOWNLOAD_ERROR_ALREADY_COMPLETED:
		str = CONV_STR(DOWNLOAD_ERROR_ALREADY_COMPLETED);
		break;
	case DOWNLOAD_ERROR_FILE_ALREADY_EXISTS:
		str = CONV_STR(DOWNLOAD_ERROR_FILE_ALREADY_EXISTS);
		break;
	case DOWNLOAD_ERROR_CANNOT_RESUME:
		str = CONV_STR(DOWNLOAD_ERROR_CANNOT_RESUME);
		break;
	case DOWNLOAD_ERROR_TOO_MANY_REDIRECTS:
		str = CONV_STR(DOWNLOAD_ERROR_TOO_MANY_REDIRECTS);
		break;
	case DOWNLOAD_ERROR_UNHANDLED_HTTP_CODE:
		str = CONV_STR(DOWNLOAD_ERROR_UNHANDLED_HTTP_CODE);
		break;
	case DOWNLOAD_ERROR_REQUEST_TIMEOUT:
		str = CONV_STR(DOWNLOAD_ERROR_REQUEST_TIMEOUT);
		break;
	case DOWNLOAD_ERROR_RESPONSE_TIMEOUT:
		str = CONV_STR(DOWNLOAD_ERROR_RESPONSE_TIMEOUT);
		break;
	case DOWNLOAD_ERROR_SYSTEM_DOWN:
		str = CONV_STR(DOWNLOAD_ERROR_SYSTEM_DOWN);
		break;
	case DOWNLOAD_ERROR_ID_NOT_FOUND:
		str = CONV_STR(DOWNLOAD_ERROR_ID_NOT_FOUND);
		break;
	case DOWNLOAD_ERROR_NO_DATA:
		str = CONV_STR(DOWNLOAD_ERROR_NO_DATA);
		break;
	case DOWNLOAD_ERROR_IO_ERROR:
		str = CONV_STR(DOWNLOAD_ERROR_IO_ERROR);
		break;
	default:
		str = "Unknown Error";
		break;
	}
	return str;
}

namespace DA_CB {
enum TYPE {
	STARTED = 1,
	PROGRESS,
	PAUSED,
	COMPLETED,
	CANCELED,
	FAILED
};
}

class CbData {
public:
	CbData();
	~CbData() {}

	void updateDownloadItem();

	inline void setType(DA_CB::TYPE type) { m_type = type; }
	inline void setUserData(void *userData) { m_userData = userData; }
	inline void setDownloadId(int id) { m_download_id = id; }
	inline void setReceivedFileSize(unsigned long long size) { m_receivedFileSize = size; }

	DA_CB::TYPE m_type;
private:
	void *m_userData;
	int m_download_id;
	unsigned long long m_receivedFileSize;
};

struct pipe_data_t {
	CbData *cbData;
};

CbData::CbData()
	:m_type(DA_CB::STARTED)
	,m_userData(NULL)
	,m_download_id(-1)
	,m_receivedFileSize(0)
{
}

DownloadEngine::DownloadEngine()
{
}

DownloadEngine::~DownloadEngine()
{
	DM_LOGD("");
}

void DownloadEngine::initEngine(void)
{
	ecore_pipe = ecore_pipe_add(__ecore_cb_pipe_update, NULL);
}

void DownloadEngine::deinitEngine(void)
{
	DM_LOGD("");
	if (ecore_pipe) {
		ecore_pipe_del(ecore_pipe);
		ecore_pipe = NULL;
	}
}

void CbData::updateDownloadItem()
{
	int ret = 0;

	if (!m_userData) {
		DM_LOGE("NULL Check:download item");
		return;
	}

	DownloadItem *downloadItem = static_cast<DownloadItem*>(m_userData);
	if (downloadItem->state() == DL_ITEM::FAILED) {
		DM_LOGE("Fail to get download item");
		return;
	}
	downloadItem->setDownloadId(m_download_id);

	switch(m_type) {
	case DA_CB::STARTED:
	{
		downloadItem->setState(DL_ITEM::STARTED);
		unsigned long long contentSize = 0;
		char *contentName = NULL;
		char *mimeType = NULL;
		char *tempPath = NULL;
		string name;
		ret = download_get_content_size(m_download_id, &contentSize);
		if (ret != DOWNLOAD_ERROR_NONE) {
			DM_LOGE("Fail to get content size:id[%d] err[%s]",
				m_download_id, __convertErrToString(ret));
			downloadItem->setState(DL_ITEM::FAILED);
			downloadItem->setErrorCode(ERROR::ENGINE_FAIL);
			download_cancel(m_download_id);
			break;
		}
		DM_SLOGI("content size[%llu]", contentSize);
		/* In case of second download of OMA download, the size form server may be not set
		 * At that case, the size from dd file should not changed.*/
		if (contentSize > 0)
			downloadItem->setFileSize((unsigned long long)contentSize);
		ret = download_get_content_name(m_download_id, &contentName);
		if (ret != DOWNLOAD_ERROR_NONE) {
			if (ret == DOWNLOAD_ERROR_NO_DATA) {
				DM_LOGI("No content name. Set default name.");
				name = string(S_("IDS_COM_BODY_NO_NAME"));
			} else {
				DM_LOGE("Fail to get content name:id[%d] err[%s]",
					m_download_id, __convertErrToString(ret));
				downloadItem->setState(DL_ITEM::FAILED);
				downloadItem->setErrorCode(ERROR::ENGINE_FAIL);
				download_cancel(m_download_id);
				free(contentName);
				break;
			}
		}
		if (contentName)
			name = string(contentName);
		DM_SLOGI("content name[%s]", contentName);
		downloadItem->setContentName(name);
		free(contentName);
		ret = download_get_mime_type(m_download_id, &mimeType);
		if (ret != DOWNLOAD_ERROR_NONE) {
			if (ret == DOWNLOAD_ERROR_NO_DATA) {
				DM_LOGI("Allow empty content type");
			} else {
				DM_LOGE("Fail to get mime type:id[%d] err[%s]",
					m_download_id, __convertErrToString(ret));
				downloadItem->setState(DL_ITEM::FAILED);
				downloadItem->setErrorCode(ERROR::ENGINE_FAIL);
				download_cancel(m_download_id);
				free(mimeType);
			}
			break;
		}
		if (mimeType) {
			string mime = string(mimeType);
			DM_SLOGI("mime type[%s]", mimeType);
			downloadItem->setMimeType(mime);
#ifdef _ENABLE_OMA_DOWNLOAD
			if (downloadItem->isOMADownloadCase()) {
				DM_SLOGI("mime type from dd[%s]",
						downloadItem->getMimeFromOmaItem().c_str());
				if (downloadItem->isNeededTocheckMimeTypeFromDD(mimeType)) {
					if (downloadItem->getMimeFromOmaItem().compare(mimeType))
						downloadItem->cancelWithcontentTypeErr();
				}
			}
#endif
			free(mimeType);
		}
		ret = download_get_temp_path(m_download_id, &tempPath);
		if (ret != DOWNLOAD_ERROR_NONE) {
			DM_LOGE("Fail to get temp path:id[%d] err[%s]",
				m_download_id, __convertErrToString(ret));
			downloadItem->setState(DL_ITEM::FAILED);
			downloadItem->setErrorCode(ERROR::ENGINE_FAIL);
			download_cancel(m_download_id);
			free(tempPath);
			break;
		}
		if (tempPath) {
			DM_SLOGI("temp path[%s]", tempPath);
			downloadItem->setFilePath(tempPath);
			free(tempPath);
		}
#ifdef _ENABLE_OMA_DOWNLOAD
		if (downloadItem->isOMAMime())
			return;
#endif
		break;
	}
	case DA_CB::PROGRESS:
		downloadItem->setState(DL_ITEM::UPDATING);
		downloadItem->setReceivedFileSize(m_receivedFileSize);
#ifdef _ENABLE_OMA_DOWNLOAD
		if (downloadItem->isOMAMime())
			return;
#endif
		break;
	case DA_CB::PAUSED:
		downloadItem->setState(DL_ITEM::SUSPENDED);
		break;
	case DA_CB::COMPLETED:
	{
		int status = 0;
		char *savedPath = NULL;
		downloadItem->setState(DL_ITEM::FINISHED);
		ret = download_get_downloaded_file_path(m_download_id, &savedPath);
		if (ret != DOWNLOAD_ERROR_NONE) {
			DM_LOGE("Fail to get downloaded file path:id[%d] err[%s]",
				m_download_id, __convertErrToString(ret));
			downloadItem->setState(DL_ITEM::FAILED);
			downloadItem->setErrorCode(ERROR::ENGINE_FAIL);
			if (savedPath)
				free(savedPath);
			break;
		}
		ret = download_get_http_status(m_download_id, &status);
		if (ret != DOWNLOAD_ERROR_NONE) {
			DM_LOGE("Fail to get downloaded status:id[%d] err[%s]",
				m_download_id, __convertErrToString(ret));
			downloadItem->setState(DL_ITEM::FAILED);
			downloadItem->setErrorCode(ERROR::ENGINE_FAIL);
			if (savedPath)
				free(savedPath);
			break;
		}
		DM_SLOGI("http status code [%d]", status);
		if (savedPath) {
			string path = string(savedPath);
			DM_SLOGI("saved path[%s]", savedPath);
			downloadItem->setRegisteredFilePath(path);
			free(savedPath);
		}
#ifdef _ENABLE_OMA_DOWNLOAD
		if (downloadItem->isOMAMime()) {
			int ret = 0;
			int err = 0;
			dd_oma1_t *dd_info = NULL;
			ret = op_parse_dd_file(downloadItem->registeredFilePath().c_str(),
					(void **)&dd_info, &err);
			if (ret != OP_TRUE || dd_info == NULL) {
				downloadItem->setState(DL_ITEM::FAILED);
				downloadItem->setErrorCode(ERROR::PARSING_FAIL);
				DM_LOGE("Fail to parsing dd:err[%d]", err);
				if (dd_info) {
					/* When parsing dd is failed, if the install notify url is existed,
					 *	it shoud send the result to the server*/
					if (dd_info->install_notify_uri) {
						auto_ptr<OmaItem> omaItem;
						omaItem = auto_ptr<OmaItem> (new OmaItem(dd_info));
						downloadItem->setOmaItem(omaItem);
						downloadItem->sendInstallNotification(906);
					}
					op_free_dd_info(dd_info);
				}
				if (unlink(downloadItem->registeredFilePath().c_str()) < 0)
					DM_LOGE("Fail to unlink the dd file:err[%s]",
							strerror(errno));
				downloadItem->destroyId();
			} else {
				if (dd_info->major_version > 1) {
					downloadItem->setState(DL_ITEM::FAILED);
					downloadItem->setErrorCode(ERROR::PARSING_FAIL);
					if (dd_info->install_notify_uri) {
						auto_ptr<OmaItem> omaItem;
						omaItem = auto_ptr<OmaItem> (new OmaItem(dd_info));
						downloadItem->setOmaItem(omaItem);
						downloadItem->sendInstallNotification(951);
					}
					op_free_dd_info(dd_info);
					break;
				}
				downloadItem->setState(DL_ITEM::REQUEST_USER_CONFIRM);
				if (strlen(dd_info->type) > 0)
					downloadItem->setMimeType(dd_info->type);
				if (strlen(dd_info->name) > 0) {
					string name = string(dd_info->name);
					downloadItem->setContentName(name);
				}
				downloadItem->setFileSize(dd_info->size);
				auto_ptr<OmaItem> omaItem;
				omaItem = auto_ptr<OmaItem> (new OmaItem(dd_info));
				downloadItem->setOmaItem(omaItem);
				/* for extracting title */
				downloadItem->setReceivedFileSize(0);
				if (unlink(downloadItem->registeredFilePath().c_str()) < 0)
					DM_LOGE("Fail to unlink the dd file:err[%s]",
							strerror(errno));
				string emptyStr = string();
				downloadItem->setRegisteredFilePath(emptyStr);
				op_free_dd_info(dd_info);
			}
		} else {
			if (downloadItem->isOMADownloadCase()) {
				downloadItem->setState(DL_ITEM::NOTIFYING);
			}
		}
#endif
		downloadItem->destroyId();
		break;
	}
	case DA_CB::CANCELED:
	{
		download_error_e error = DOWNLOAD_ERROR_NONE;
#ifdef _ENABLE_OMA_DOWNLOAD
		if (downloadItem->isOMADownloadCase()) {
			downloadItem->sendInstallNotification(902);
		}
#endif
		ret = download_get_error(m_download_id, &error);
		if (ret != DOWNLOAD_ERROR_NONE) {
			DM_LOGE("Fail to get downloaded error:id[%d] err[%s]",
				m_download_id, __convertErrToString(ret));
			downloadItem->setState(DL_ITEM::FAILED);
			downloadItem->setErrorCode(ERROR::ENGINE_FAIL);
			break;
		}
		if (downloadItem->state() == DL_ITEM::CANCELED) {
			DM_LOGI("Already update cancel UI");
			downloadItem->destroyId();
			return;
		}
		if (error != DOWNLOAD_ERROR_NONE) {
			DM_LOGE("Cancel error:err[%d]", error);
			downloadItem->setState(DL_ITEM::FAILED);
			downloadItem->setErrorCode(ERROR::ENGINE_FAIL);
		} else {
			/* In case the content server request cancel */
			downloadItem->setState(DL_ITEM::CANCELED);
		}
		downloadItem->destroyId();
		break;
	}
	case DA_CB::FAILED:
	{
		download_error_e error = DOWNLOAD_ERROR_NONE;
		ret = download_get_error(m_download_id, &error);
		if (ret != DOWNLOAD_ERROR_NONE) {
			DM_LOGE("Fail to get download error:id[%d] err[%s]",
				m_download_id, __convertErrToString(ret));
			downloadItem->setState(DL_ITEM::FAILED);
			downloadItem->setErrorCode(ERROR::ENGINE_FAIL);
			break;
		}

		if (error != DOWNLOAD_ERROR_NONE) {
			downloadItem->setState(DL_ITEM::FAILED);
			downloadItem->setErrorCode(downloadItem->_convert_error(error));
#ifdef _ENABLE_OMA_DOWNLOAD
			if (downloadItem->isOMADownloadCase()) {
				int status = downloadItem->convertInstallStatus(error);
				downloadItem->sendInstallNotification(status);
			}
#endif
		} else {
			DM_LOGE("Cannot enter here!!:err[%d]", error);
		}
		downloadItem->destroyId();
		break;
	}
	default:
		break;
	}
	downloadItem->notify();
}

void __ecore_cb_pipe_update(void *data, void *buffer, unsigned int nbyte)
{
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
	: m_download_id(-1)
	, m_state(DL_ITEM::IGNORE)
	, m_errorCode(ERROR::NONE)
	, m_receivedFileSize(0)
	, m_fileSize(0)
	, m_downloadType(DL_TYPE::HTTP_DOWNLOAD)
{
}

DownloadItem::DownloadItem(auto_ptr<DownloadRequest> request)
	: m_aptr_request(request)
	, m_download_id(-1)
	, m_state(DL_ITEM::IGNORE)
	, m_errorCode(ERROR::NONE)
	, m_receivedFileSize(0)
	, m_fileSize(0)
	, m_downloadType(DL_TYPE::HTTP_DOWNLOAD)
{
}

int DownloadItem::createId(int id)
{
	FileUtility fileObj;
	int ret = download_create(&m_download_id);
	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to create download id:[%s]", __convertErrToString(ret));
		return ret;
	}
	DM_LOGV("URL download id : [%d]", m_download_id);

	ret = download_set_state_changed_cb(m_download_id, state_changed_cb, static_cast<void*>(this));
	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to set state changed cb:[%s]", __convertErrToString(ret));
		return ret;
	}

	ret = download_set_progress_cb(m_download_id, progress_cb, static_cast<void*>(this));
	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to set progress cb:[%s]", __convertErrToString(ret));
		return ret;
	}
	if (!fileObj.checkTempDir(m_aptr_request->getInstallDir()))
		return ret;

	return ret;
}

DownloadItem::~DownloadItem()
{
	destroyId();
}

void DownloadItem::destroyId()
{
	if (m_download_id < 0)
		return;
	DM_LOGI("download id[%d]", m_download_id);
	download_unset_state_changed_cb(m_download_id);
	download_unset_progress_cb(m_download_id);
	download_destroy(m_download_id);
	m_download_id = -1;
}

void DownloadItem::state_changed_cb(int download_id, download_state_e state,
	void *user_data)
{
	CbData *cbData = new CbData();
	cbData->setDownloadId(download_id);
	cbData->setUserData(user_data);
	switch(state) {
	/* The state callback is called when the state value is changed.
	   So, it means that the download is started when the downloading state is received */
	case DOWNLOAD_STATE_DOWNLOADING:
		cbData->setType(DA_CB::STARTED);
		break;
	case DOWNLOAD_STATE_PAUSED:
		cbData->setType(DA_CB::PAUSED);
		break;
	case DOWNLOAD_STATE_COMPLETED:
		cbData->setType(DA_CB::COMPLETED);
		break;
	case DOWNLOAD_STATE_FAILED:
		cbData->setType(DA_CB::FAILED);
		break;
	case DOWNLOAD_STATE_CANCELED:
		cbData->setType(DA_CB::CANCELED);
		break;
	default:
		DM_LOGI("Ignore state:[%d]", state);
		delete cbData;
		return;
	}
	pipe_data_t pipe_data;
	pipe_data.cbData = cbData;
	ecore_pipe_write(ecore_pipe, &pipe_data, sizeof(pipe_data_t));
}
void DownloadItem::progress_cb(int download_id, unsigned long long received,
	void *user_data)
{
	CbData *cbData = new CbData();
	cbData->setType(DA_CB::PROGRESS);
	cbData->setDownloadId(download_id);
	cbData->setUserData(user_data);
	cbData->setReceivedFileSize(received);

	pipe_data_t pipe_data;
	pipe_data.cbData = cbData;
	ecore_pipe_write(ecore_pipe, &pipe_data, sizeof(pipe_data_t));
}

void DownloadItem::start(int id)
{
	int ret = 0;
	string url;
	DM_LOGD("");
	if (m_download_id > 0) {
		destroyId();
	}
	if ((ret = createId(id)) != DOWNLOAD_ERROR_NONE) {
		if (ret == DOWNLOAD_ERROR_TOO_MANY_DOWNLOADS) {
			m_state = DL_ITEM::QUEUED;
			m_errorCode = ERROR::NONE;
		} else {
			m_state = DL_ITEM::FAILED;
			m_errorCode = ERROR::ENGINE_FAIL;
		}
		notify();
		return;
	}

#ifdef _ENABLE_OMA_DOWNLOAD
	if (m_oma_item.get())
		url = m_oma_item.get()->getObjectUri();
	else
		url = m_aptr_request->getUrl();
#else
	url = m_aptr_request->getUrl();
#endif
	ret = download_set_url(m_download_id, url.c_str());
	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to set url:[%s]", __convertErrToString(ret));
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
		return;
	}
	if (!m_aptr_request->getCookie().empty()) {
		ret = download_add_http_header_field(m_download_id,
				"Cookie", m_aptr_request->getCookie().c_str());
		if (ret != DOWNLOAD_ERROR_NONE) {
			DM_LOGE("Fail to set cookie:[%s]", __convertErrToString(ret));
			m_state = DL_ITEM::FAILED;
			m_errorCode = ERROR::ENGINE_FAIL;
			notify();
			return;
		}
	}
	if (!m_aptr_request->getReqHeaderField().empty() &&
			!m_aptr_request->getReqHeaderValue().empty()) {
		ret = download_add_http_header_field(m_download_id,
				m_aptr_request->getReqHeaderField().c_str(),
				m_aptr_request->getReqHeaderValue().c_str());
		if (ret != DOWNLOAD_ERROR_NONE) {
			DM_LOGE("Fail to set request header:[%s]", __convertErrToString(ret));
			m_state = DL_ITEM::FAILED;
			m_errorCode = ERROR::ENGINE_FAIL;
			notify();
			return;
		}
	}
	if (!m_aptr_request->getInstallDir().empty()) {
		string tempDir = m_aptr_request->getInstallDir();
		tempDir.append(DM_TEMP_DIR_NAME);
		ret = download_set_destination(m_download_id, tempDir.c_str());
	} else {
		string dirPath = FileUtility::getDefaultPath(true);
		ret = download_set_destination(m_download_id, dirPath.c_str());
	}
	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to set destination[%s]", __convertErrToString(ret));
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
		return;
	}

	ret = download_start(m_download_id);
	DM_LOGI("URL download id:[%d] ret[%d]", m_download_id, ret);
	

	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to start:[%s]", __convertErrToString(ret));
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
	}
}

ERROR::CODE DownloadItem::_convert_error(int err)
{
	DM_LOGI("Error from download service:[%d]", err);

	switch (err) {
	case DOWNLOAD_ERROR_NETWORK_UNREACHABLE:
	case DOWNLOAD_ERROR_CONNECTION_TIMED_OUT:
	case DOWNLOAD_ERROR_CONNECTION_FAILED:
		return ERROR::NETWORK_FAIL;

	case DOWNLOAD_ERROR_INVALID_URL:
		return ERROR::INVALID_URL;

	case DOWNLOAD_ERROR_NO_SPACE:
		return ERROR::NOT_ENOUGH_MEMORY;
	default :
		return ERROR::UNKNOWN;
	}

}
#ifdef _ENABLE_OMA_DOWNLOAD
int DownloadItem::convertInstallStatus(int err)
{
	DM_LOGI("Error from download service:[%d]", err);

	switch (err) {
	case DOWNLOAD_ERROR_NETWORK_UNREACHABLE:
	case DOWNLOAD_ERROR_CONNECTION_TIMED_OUT:
	case DOWNLOAD_ERROR_CONNECTION_FAILED:
	case DOWNLOAD_ERROR_INVALID_URL:
		return 954;
	case DOWNLOAD_ERROR_NO_SPACE:
		return 901;
	case DOWNLOAD_ERROR_IO_ERROR:
	default :
		return 952;
	}
}
#endif

void DownloadItem::cancel()
{
	DM_LOGI("");
	if (m_state == DL_ITEM::CANCELED) {
		DM_LOGI("It is already canceled");
		notify();
		return;
	}
	int ret = download_cancel(m_download_id);
	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to cancel download:id[%d] reason[%s]",
			m_download_id, __convertErrToString(ret));
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
	}
	m_state = DL_ITEM::CANCELED;
	notify();
	return;
}

#ifdef _ENABLE_OMA_DOWNLOAD
bool DownloadItem::isNeededTocheckMimeTypeFromDD(const char *mimeType)
{
	DM_LOGD("");
	if (getMimeFromOmaItem().empty())
		return false;

	if (strncmp(mimeType, DM_DRM_MIME_STR, strlen(DM_DRM_MIME_STR)) == 0 ||
			strncmp(mimeType, DM_DCF_MIME_STR, strlen(DM_DCF_MIME_STR)) == 0)
		return false;
	return true;
}

void DownloadItem::cancelWithcontentTypeErr()
{
	DM_LOGI("");
	if (m_state == DL_ITEM::CANCELED) {
		DM_LOGI("It is already canceled");
		notify();
		return;
	}
	int ret = download_cancel(m_download_id);
	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to cancel download:id[%d] reason[%s]",
			m_download_id, __convertErrToString(ret));
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
	}
	sendInstallNotification(905);
	m_state = DL_ITEM::FAILED;
	m_errorCode = ERROR::PARSING_FAIL;
	notify();
	return;
}
#endif

void DownloadItem::retry(int id)
{
	DM_LOGD("");
	m_state = DL_ITEM::IGNORE;
	m_errorCode = ERROR::NONE;
	m_receivedFileSize = 0;
	m_fileSize = 0;
	m_downloadType = DL_TYPE::HTTP_DOWNLOAD;
	if (!m_registeredFilePath.empty())
		m_registeredFilePath.clear();
#ifdef _ENABLE_OMA_DOWNLOAD
	if (m_oma_item.get())
		m_oma_item.release();
#endif
	start(id);
}

void DownloadItem::suspend()
{
	int ret = download_pause(m_download_id);
	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to suspend download:id[%d] err[%s]",
			m_download_id, __convertErrToString(ret));
		download_cancel(m_download_id);
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
	}
}

void DownloadItem::resume()
{
	int ret = DOWNLOAD_ERROR_NONE;

	ret = download_start(m_download_id);
	if (ret != DOWNLOAD_ERROR_NONE) {
		DM_LOGE("Fail to resume download:id[%d] err[%s]",
			m_download_id, __convertErrToString(ret));
		download_cancel(m_download_id);
		m_state = DL_ITEM::FAILED;
		m_errorCode = ERROR::ENGINE_FAIL;
		notify();
	}
}

#ifdef _ENABLE_OMA_DOWNLOAD
string DownloadItem::getUserMessage(void)
{
	if (m_oma_item.get())
		return m_oma_item->getUserMessage();
	else
		return string();
}

bool DownloadItem::isNotifyFiinished(void)
{
	if (m_oma_item.get())
			return m_oma_item->isNotifyFiinished();
		else
			return true;
}

bool DownloadItem::isOMAMime()
{
	if (m_mimeType.empty())
		return false;
	if (m_mimeType.compare(DD_MIME_STR) == 0)
		return true;
	return false;
}

OmaItem::OmaItem()
	: size(0)
	, status(900)
	, retryCount(0)
	, th(NULL)
	, notifyFinished(false)
{
}

OmaItem::OmaItem(dd_oma1_t *dd_info)
{
	retryCount = 0;
	th = NULL;
	notifyFinished = false;
	if (dd_info) {
		if (strlen(dd_info->name)>0)
			name = string(dd_info->name);
		if (dd_info->major_version>0 || dd_info->minor_version) {
			char buf[256] = {0,};
			sprintf(buf, "%d.%d", dd_info->major_version, dd_info->minor_version);
			buf[255] = '\0';
			version = string(buf);
		}
		if (strlen(dd_info->object_uri) > 0)
			objectUri = string(dd_info->object_uri);
		if (dd_info->vendor)
			vendor = string(dd_info->vendor);
		if (dd_info->install_notify_uri)
			installUri = string(dd_info->install_notify_uri);
		if (dd_info->description)
			description = string(dd_info->description);
		if (dd_info->icon_uri)
			iconUri = string(dd_info->icon_uri);
		if (dd_info->next_url)
			nextUri = string(dd_info->next_url);
		if (strlen(dd_info->type) > 1)
			contentType = string(dd_info->type);
		if (dd_info->install_param)
			installParam = string(dd_info->install_param);
		if (dd_info->size > 0)
			size = dd_info->size;
	}
	status = 900;
}

OmaItem::~OmaItem()
{
	if (th)
		ecore_thread_cancel(th);
}

void OmaItem::sendInstallNotify()
{
}

void OmaItem::goNextUrl()
{
}

void OmaItem::getIconFromUri()
{
}

string OmaItem::getUserMessage()
{
	string buf = string();
	if (!name.empty()) {
		buf.append("Name : ");
		buf.append(name);
		buf.append("<br>");
	}

	buf.append("Size : ");
	buf.append(getBytesStr(size));

	if (!contentType.empty()) {
		buf.append("<br>");
		buf.append("Mime : ");
		buf.append(contentType);
	}

	if (!version.empty()) {
		buf.append("<br>");
		buf.append("Version : ");
		buf.append(version);
	}
	if (!vendor.empty()) {
		buf.append("<br>");
		buf.append("Vendor : ");
		buf.append(vendor);
	}
	if (!description.empty()) {
		buf.append("<br>");
		buf.append("Description : ");
		buf.append(description);
		buf.append("<br>");
	}
	return buf;
}

/* FIXME later : this function is same to getHumanFriendlyBytesStr() method
   This should move to util class */
string OmaItem::getBytesStr(unsigned long long bytes)
{
	double doubleTypeBytes = 0.0;
	const char *unitStr[4] = {"B", "KB", "MB", "GB"};
	int unit = 0;
	unsigned long long unitBytes = bytes;

	/* using bit operation to avoid floating point arithmetic */
	for (unit = 0; (unitBytes > 1024 && unit < 4) ; unit++) {
		unitBytes = unitBytes >> 10;
	}

	unitBytes = 1 << (10 * unit);
	doubleTypeBytes = ((double)bytes / (double)(unitBytes));
	// FIXME following code should be broken into another function, but leave it now to save function call time.s
	char str[64] = {0};

	if (unit > 3)
		unit = 3;

	if (unit == 0)
		snprintf(str, sizeof(str)-1, "%llu %s", bytes, unitStr[unit]);
	else
		snprintf(str, sizeof(str)-1, "%.2f %s", doubleTypeBytes, unitStr[unit]);
	str[63] = '\0';
	string temp = string(str);
	return temp;
}

string OmaItem::getMessageForInstallNotification(int statusCode)
{
	string msg = string();
	switch(statusCode) {
	case 900:
		msg = "900 Success";
		break;
	case 901:
		msg = "901 Insufficient memory";
		break;
	case 902:
		msg = "902 User Cancelled";
		break;
	case 903:
		msg = "903 Loss of Service";
		break;
	case 905:
		msg = "905 Attribute mismatch";
		break;
	case 906:
		msg = "906 Invalid descriptor";
		break;
	case 951:
		msg = "951 Invalid DDVersion";
		break;
	case 952:
		msg = "952 Device Aborted";
		break;
	case 953:
		msg = "953 Non-Acceptable Content";
		break;
	case 954:
		msg = "954 Loader Error";
		break;
	default :
		msg = string();
		break;
	}
	return msg;
}

int myTrace(CURL *handle, curl_infotype type, char *data, size_t size, void *user)
{
	switch(type) {
	case CURLINFO_TEXT:
		if (data)
			DM_LOGV("[curl] Info:%s", data);
		break;
	case CURLINFO_HEADER_OUT:
		DM_LOGV("[curl] Send header");
		if (data)
			DM_LOGV("[curl] %s", data);
		break;
	case CURLINFO_DATA_OUT:
		DM_LOGD("[curl] Send data");
		if (data)
			DM_SLOGD("[curl] %s", data);
		break;
	case CURLINFO_SSL_DATA_OUT:
		DM_SLOGD("[curl] Send SSL data");
		break;
	case CURLINFO_HEADER_IN:
		DM_SLOGD("[curl] Recv header");
		if (data)
			DM_SLOGD("[curl] %s", data);
		break;
	case CURLINFO_DATA_IN:
		DM_SLOGD("[curl] Recv data");
		if (data)
			DM_SLOGD("[curl] %s", data);
		break;
	case CURLINFO_SSL_DATA_IN:
		DM_SLOGD("[curl] Recv SSL data");
		break;
	default:
		return 0;
	}
	return 0;
}

void OmaItem::sendInstallNotification(int s)
{
	status = s;
	th = ecore_thread_run(sendInstallNotifyCB, threadEndCB,
			threadCancelCB, this);
}

void OmaItem::sendInstallNotification(int s, string url)
{
	status = s;
	installUri = url;
	th = ecore_thread_run(sendInstallNotifyCB, threadEndCB,
			threadCancelCB, this);
}

void OmaItem::sendInstallNotifyCB(void *data, Ecore_Thread *th)
{
	CURL *curl;
	CURLcode res;
	string msg = string();
	OmaItem *item = (OmaItem *)data;
	struct curl_slist *header = NULL;
	long httpCode = 0;
	string proxyAddr = string();
	string userAgent;
	DownloadUtil &utilObj = DownloadUtil::getInstance();

	if (!data) {
		DM_LOGE("[CRITICAL]NULL Check: oma item");
		return;
	}

	item->retryCount++;
	DM_LOGI("try[%d]", item->retryCount);
	proxyAddr = NetMgr::getInstance().getProxy();
	userAgent = utilObj.getUserAgent();
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	msg = item->getMessageForInstallNotification(item->getStatus());
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, item->getInstallUri().c_str());
		DM_SLOGD("install notify url[%s]",item->getInstallUri().c_str());
		if (!userAgent.empty())
			curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, msg.length() + 1);
		if (!proxyAddr.empty())
			curl_easy_setopt(curl, CURLOPT_PROXY, proxyAddr.c_str());
		else
			DM_LOGI("proxy is not set");
		/* debug */
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, myTrace);
	}
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		DM_LOGE("Fail to send install notification[%s]", curl_easy_strerror(res));
		if (!ecore_thread_reschedule(th))
			DM_LOGE("Fail to ecore_thread_reschedule");
	} else {
		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		if (res != CURLE_OK) {
			if (!ecore_thread_reschedule(th))
				DM_LOGE("Fail to ecore_thread_reschedule");
		} else {
			DM_LOGI("Response Http Status code[%d]", (int)httpCode);
			if (httpCode != 200)
				if (!ecore_thread_reschedule(th))
					DM_LOGE("Fail to ecore_thread_reschedule");
		}
	}
	curl_easy_cleanup(curl);
	curl_slist_free_all(header);
	if (item->retryCount > 2) {
		ecore_thread_cancel(th);
	}
	return;
}
void OmaItem::threadEndCB(void *data, Ecore_Thread *th)
{
	OmaItem *item = (OmaItem *)data;
	if (!data) {
		DM_LOGE("[CRITICAL]NULL Check: oma item");
		return;
	}
	item->setNotifyFinished(true);
	item->setThreadData(NULL);
}

void OmaItem::threadCancelCB(void *data, Ecore_Thread *th)
{
	OmaItem *item = (OmaItem *)data;
	if (!data) {
		DM_LOGE("[CRITICAL]NULL Check: oma item");
		return;
	}
	item->setNotifyFinished(true);
	item->setThreadData(NULL);
}

#endif
