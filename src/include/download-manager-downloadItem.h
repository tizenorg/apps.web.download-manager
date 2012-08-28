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
 * @file 	download-manager-downloadItem.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	download item class
 */

#ifndef DOWNLOAD_MANAGER_DOWNLOAD_ITEM_H
#define DOWNLOAD_MANAGER_DOWNLOAD_ITEM_H

#include <memory>
#include "url_download.h"
#include "download-manager-common.h"
#include "download-manager-downloadRequest.h"
#include "download-manager-event.h"

namespace DL_ITEM {
enum STATE {
	IGNORE,
	STARTED,
	UPDATING,
	COMPLETE_DOWNLOAD,
	INSTALL_NOTIFY,
	WAITING_RO,
	SUSPENDED,
	RESUMED,
	FINISHED,
	CANCELED,
	FAILED
};
}

class DownloadItem {
public:
	DownloadItem();	/* FIXME remove after cleanup ecore_pipe */
	DownloadItem(auto_ptr<DownloadRequest> request);
	~DownloadItem();

	void start(bool isRetry);
	void cancel(void);
	void retry(void);
	void suspend(void);
	void resume(void);
	void createHandle(void);
	void destroyHandle(void);

	inline void *downloadHandle(void) { return (void *)m_download_handle;}
	inline void setDownloadHandle(url_download_h handle) { m_download_handle = handle; }

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
	inline string &url(void) { return m_aptr_request->getUrl(); }
	inline string &cookie(void) { return m_aptr_request->getCookie(); }

	ERROR::CODE _convert_error(int err);

	static void started_cb(url_download_h download, const char *name,
		const char *mime, void *user_data);
	static void paused_cb(url_download_h download, void *user_data);
	static void completed_cb(url_download_h download, const char *path,
		void *user_data);
	static void stopped_cb(url_download_h download, url_download_error_e error,
		void *user_data);
	static void progress_cb(url_download_h download, unsigned long long received,
		unsigned long long total, void *user_data);

private:
	auto_ptr<DownloadRequest> m_aptr_request;
	Subject m_subject;
	url_download_h m_download_handle;
	DL_ITEM::STATE m_state;
	ERROR::CODE m_errorCode;
	unsigned long int m_receivedFileSize;
	unsigned long int m_fileSize;
	string m_filePath;
	string m_contentName;
	string m_registeredFilePath;
	string m_mimeType;
	DL_TYPE::TYPE m_downloadType;
	int m_download_id;
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
