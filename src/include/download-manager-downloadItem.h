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
#ifdef _ENABLE_OMA_DOWNLOAD
	REQUEST_USER_CONFIRM,
#endif
	UPDATING,
	COMPLETE_DOWNLOAD,
	INSTALL_NOTIFY,
	WAITING_RO,
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
	void sendInstallNotify(void);
	void goNextUrl(void);
	void getIconFromUri(void);
	string getUserMessage(void);
	string getObjectUri(void) { return objectUri; }
	string getName(void) { return name; }
	string getVersion(void) { return version; }
	string getVendor(void) { return vendor; }
	string getDescription(void) { return description; }
	string getIconPath(void) { return iconPath; }
	string getInstallUri(void) { return installUri; }
	int getStatus(void) { return status; }
	void sendInstallNotification(int status);
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
	unsigned long int size;
	int status;
	int retryCount;
	string proxyAddr;
	Ecore_Idler *idler;
	string getBytesStr(unsigned long int bytes);
	static Eina_Bool sendInstallNotifyCB(void *data);
	bool notifyFinished;
};
#endif

class DownloadItem {
public:
	DownloadItem();	/* FIXME remove after cleanup ecore_pipe */
	DownloadItem(auto_ptr<DownloadRequest> request);
	~DownloadItem();

	void start(int id);
	void cancel(void);
	void retry(int id);
	void suspend(void);
	void resume(void);
	bool createId(int id);
	void destroyId(void);

	inline int downloadId(void) { return m_download_id;}
	inline void setDownloadId(int id) { m_download_id = id; }

	inline unsigned long int receivedFileSize(void) { return m_receivedFileSize; }
	inline void setReceivedFileSize(unsigned long int size) { m_receivedFileSize = size; }

	inline unsigned long int fileSize(void) { return m_fileSize; }
	inline void setFileSize(unsigned long int size) { m_fileSize = size; }

	inline string &filePath(void) { return m_filePath; }
	inline void setFilePath(const char *path) { if (path) m_filePath = path; }
	inline void setFilePath(string &path) { m_filePath = path; }

	inline string &contentName(void) { return m_contentName; }
	inline void setContentName(string &name) { m_contentName = name; }

	inline string &registeredFilePath(void) { return m_registeredFilePath; }
	inline void setRegisteredFilePath(string &path) { m_registeredFilePath = path; }

	inline string &mimeType(void) { return m_mimeType; }
	inline void setMimeType(const char *mime) { m_mimeType = mime; }
	inline void setMimeType(string &mime) { m_mimeType = mime; }

	inline DL_ITEM::STATE state(void) { return m_state; }
	inline void setState(DL_ITEM::STATE state) { m_state = state; }

	inline ERROR::CODE errorCode(void) { return m_errorCode; }
	inline void setErrorCode(ERROR::CODE err) { m_errorCode = err;	}
	inline DL_TYPE::TYPE downloadType(void) { return m_downloadType; }
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
	inline bool isOMADownloadCase()
	{
		if (m_oma_item.get())
			return true;
		else
			return false;
	}
	bool isNotifyFiinished(void);
#endif
	inline string &url(void) { return m_aptr_request->getUrl(); }
	inline string &cookie(void) { return m_aptr_request->getCookie(); }
	inline string sender(void) { return m_aptr_request->getSender(); }


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
	unsigned long int m_receivedFileSize;
	unsigned long int m_fileSize;
	string m_filePath;
	string m_contentName;
	string m_registeredFilePath;
	string m_mimeType;
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
