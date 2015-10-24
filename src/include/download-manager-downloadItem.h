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
 * @file 	download-manager-downloadItem.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	download item class
 */

#ifndef DOWNLOAD_MANAGER_DOWNLOAD_ITEM_H
#define DOWNLOAD_MANAGER_DOWNLOAD_ITEM_H

#include <memory>
#include "download_product.h"
#include "download.h"
#include "download-manager-common.h"
#include "download-manager-downloadRequest.h"
#include "download-manager-event.h"
#ifdef _ENABLE_OMA_DOWNLOAD
#include "oma-parser-interface.h"
#include "Eina.h"
#endif
#include "Ecore.h"

namespace DL_ITEM {
enum STATE {
	IGNORE,
	STARTED,
	QUEUED,
#ifdef _ENABLE_OMA_DOWNLOAD
	REQUEST_USER_CONFIRM,
#endif
	UPDATING,
	COMPLETE_DOWNLOAD,
	INSTALL_NOTIFY,
	SUSPENDED,
	RESUMED,
#ifdef _ENABLE_OMA_DOWNLOAD
	NOTIFYING,
#endif
	FINISHED,
	CANCELED,
	FAILED
};
}

#ifdef _ENABLE_OMA_DOWNLOAD
class OmaItem {
public:
	OmaItem(void);
	OmaItem(dd_oma1_t *dd_info);
	~OmaItem(void);
	string getUserMessage(void);
	string getObjectUri(void) { return objectUri; }
	string getName(void) { return name; }
	string getVersion(void) { return version; }
	string getVendor(void) { return vendor; }
	string getDescription(void) { return description; }
	string getIconPath(void) { return iconPath; }
	string getContentType(void) { return contentType; }
	string getInstallUri(void) { return installUri; }
	void setThreadData(Ecore_Thread *t) { th = t; }
	int getStatus(void) { return status; }
	void sendInstallNotification(int status);
	void sendInstallNotification(int status, string url);
	string getMessageForInstallNotification(int statusCode);
	bool isNotifyFiinished(void) { return notifyFinished; }
	void setNotifyFinished(bool b) { notifyFinished = b; }
private:
	string objectUri;
	string name;
	string version;
	string vendor;
	string installUri;
	string description;
	string iconUri;
	string nextUri;
	string installParam;
	string iconPath;
	string contentType;
	unsigned long long size;
	int status;
	int retryCount;
	string proxyAddr;
	Ecore_Thread *th;
	string getBytesStr(unsigned long long bytes);
	static void sendInstallNotifyCB(void *data, Ecore_Thread *thread);
	static void threadEndCB(void *data, Ecore_Thread *thread);
	static void threadCancelCB(void *data, Ecore_Thread *thread);
	bool notifyFinished;
};
#endif

class DownloadItem {
public:
	DownloadItem();	/* FIXME remove after cleanup ecore_pipe */
	DownloadItem(auto_ptr<DownloadRequest> request);
	~DownloadItem();

	bool start(int id);
	void cancel(void);
#ifdef _ENABLE_OMA_DOWNLOAD
	bool isNeededTocheckMimeTypeFromDD(const char *mimeType);
	void cancelWithcontentTypeErr(void);
#endif
	bool retry(int id, unsigned long long fileSize);
	void suspend(void);
	void resume(void);
	int createId(int id);
	void destroyId(void);

	inline int getDownloadId(void) { return m_download_id;}
	inline void setDownloadId(int id) { m_download_id = id; }

	inline unsigned long long getReceivedFileSize(void) { return m_receivedFileSize; }
	inline void setReceivedFileSize(unsigned long long size) { m_receivedFileSize = size; }

	inline unsigned long long getFileSize(void) { return m_fileSize; }
	inline void setFileSize(unsigned long long size) { m_fileSize = size; }

	inline string &getFilePath(void) { return m_filePath; }
	inline void setFilePath(const char *path) {
		if (path) {
			m_filePath = path;
			m_aptr_request->setTempFilePath(path);
		}
	}
	inline void setFilePath(string &path) {
		m_filePath = path;
		m_aptr_request->setTempFilePath(path);
	}

	inline string &getContentName(void) { return m_contentName; }
	inline void setContentName(string &name) { m_contentName = name; }

	inline string &getRegisteredFilePath(void) { return m_registeredFilePath; }
	inline void setRegisteredFilePath(string &path) { m_registeredFilePath = path; }

	inline string &getMimeType(void) { return m_mimeType; }
	inline void setMimeType(const char *mime) { m_mimeType = mime; }
	inline void setMimeType(string &mime) { m_mimeType = mime; }

	inline string &getEtag(void) { return m_etag; }
	inline void setEtag(const char *etag)
	{
		m_etag = etag;
		m_aptr_request->setEtag(etag);
	}
	inline void setEtag(string &etag)
	{
		m_etag = etag;
		m_aptr_request->setEtag(etag);
	}

	inline bool getNetworkBonding(void)
	{
		if (m_aptr_request.get())
			return m_aptr_request->getNetworkBondingOption();
		return false;
	}

	inline DL_ITEM::STATE getState(void) { return m_state; }
	inline void setState(DL_ITEM::STATE state) { m_state = state; }

	inline ERROR::CODE getErrorCode(void) { return m_errorCode; }
	inline void setErrorCode(ERROR::CODE err) { m_errorCode = err;	}
	inline DL_TYPE::TYPE getDownloadType(void) { return m_downloadType; }
	inline void setDownloadType(DL_TYPE::TYPE t) { m_downloadType = t;}

	inline void notify(void) { m_subject.notify(); }
	inline void subscribe(Observer *o) { if (o) m_subject.attach(o); }
	inline void deSubscribe(Observer *o) { if (o) m_subject.detach(o); }
#ifdef _ENABLE_OMA_DOWNLOAD
	inline void setUrl(string url) { if (m_aptr_request.get()) m_aptr_request->setUrl(url); }
	inline void setOmaItem(auto_ptr<OmaItem> item) { m_oma_item = item; }
	string getUserMessage(void);
	inline void sendInstallNotification(int status)
	{
		if (m_oma_item.get() && !m_oma_item->getInstallUri().empty())
			m_oma_item->sendInstallNotification(status);
	}
	inline bool isExistedInstallNotifyUri()
	{
		if (m_oma_item.get() && !m_oma_item->getInstallUri().empty())
			return true;
		else
			return false;
	}
	inline bool isOMADownloadCase()
	{
		if (m_oma_item.get())
			return true;
		else
			return false;
	}
	inline string getMimeFromOmaItem(void)
	{
		string emptyStr = string();
		if (m_oma_item.get())
			return m_oma_item->getContentType();
		else
			return emptyStr;
	}
	inline string getInstallNotifyUri(void)
	{
		string emptyStr = string();
		if (m_oma_item.get())
			return m_oma_item->getInstallUri();
		else
			return emptyStr;
	}
	bool isNotifyFiinished(void);
	bool isOMAMime(void);
#endif
	inline string getUrl(void) { return m_aptr_request->getUrl(); }
	inline string getReqHeaderField(void) { return m_aptr_request->getReqHeaderField(); }
	inline string getReqHeaderValue(void) { return m_aptr_request->getReqHeaderValue(); }
	inline string getInstallDir(void) { return m_aptr_request->getInstallDir(); }
	inline string getFileName(void) { return m_aptr_request->getFileName(); }
	inline string getSender(void) { return m_aptr_request->getSender(); }

	ERROR::CODE _convert_error(int err);

	static void state_changed_cb(int download_id, download_state_e state,
		void *user_data);
	static void progress_cb(int download_id, unsigned long long received,
		void *user_data);
#ifdef _ENABLE_OMA_DOWNLOAD
	int convertInstallStatus(int err);
#endif

private:
	auto_ptr<DownloadRequest> m_aptr_request;
#ifdef _ENABLE_OMA_DOWNLOAD
	auto_ptr<OmaItem> m_oma_item;
#endif
	Subject m_subject;
	int m_download_id;
	DL_ITEM::STATE m_state;
	ERROR::CODE m_errorCode;
	unsigned long long m_receivedFileSize;
	unsigned long long m_fileSize;
	string m_filePath;
	string m_contentName;
	string m_registeredFilePath;
	string m_mimeType;
	string m_etag;
	string m_tempPath;
	DL_TYPE::TYPE m_downloadType;
};

class DownloadEngine {
public:
	static DownloadEngine &getInstance(void) {
		static DownloadEngine downloadEngine;
		return downloadEngine;
	}

	void initEngine(void);
	void deinitEngine(void);
private:
	DownloadEngine(void);
	~DownloadEngine(void);
};



#endif /* DOWNLOAD_MANAGER_DOWNLOAD_ITEM_H */
