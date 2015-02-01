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
 * @file 	download-manager-item.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Item class
 */
#ifndef DOWNLOAD_MANAGER_ITEM_H
#define DOWNLOAD_MANAGER_ITEM_H

#include <string>
#include <memory>
#include "download-manager-event.h"
#include "download-manager-downloadRequest.h"
#include "download-manager-downloadItem.h"
#include "download-manager-util.h"
#ifdef _ENABLE_OMA_DOWNLOAD
#include "Ecore.h"
#endif
using namespace std;

namespace ITEM {
enum STATE {
	IDLE = 0,
	PREPARE_TO_RETRY,
	REQUESTING,
	QUEUED,
	RECEIVING_DOWNLOAD_INFO,
#ifdef _ENABLE_OMA_DOWNLOAD
	REQUEST_USER_CONFIRM,
#endif
	DOWNLOADING,
	REGISTERING_TO_SYSTEM,
	SUSPENDED,
#ifdef _ENABLE_OMA_DOWNLOAD
	NOTIFYING,
#endif
#ifdef _ENABLE_WAITING_RO
	WAITING_RO,
#endif
	FINISH_DOWNLOAD,
	FAIL_TO_DOWNLOAD,
	CANCEL,
	PLAY,
	DESTROY
};
}

class DownloadNoti;

class Item {
public:
	static void create(DownloadRequest &rRequest);
	static Item *createHistoryItem(void);
	~Item(void);

	void attachHistoryItem(void);
	void destroy(void);
	/* SHOULD call this before destrying an item*/
	void deleteFromDB(void);
	void download(void);
	void downloadFromQueuedState(void);
	inline void cancel(void)
	{
		if (m_aptr_downloadItem.get())
			m_aptr_downloadItem->cancel();
		return;
	}
	void clearForRetry(void);
	bool retry(void);
	bool play(void);
	bool isExistedFile(void);
#ifdef _ENABLE_OMA_DOWNLOAD
	void handleUserConfirm(bool res);
	void doneNotifyFinished(void);
#endif
	void deleteCompleteNoti(void);
	inline void subscribe(Observer *o) { m_subjectForView.attach(o); }
	inline void deSubscribe(Observer *o) { m_subjectForView.detach(o); }

	static void updateCBForDownloadObserver(void *data);
	static void netEventCBObserver(void *data);
	void updateFromDownloadItem(void);
	void suspend(void);

	inline int getId(void) {
		if (m_aptr_downloadItem.get())
			return (int)(m_aptr_downloadItem->getDownloadId());

		return -1;
	} 	// FIXME create Item's own id

	inline unsigned long long getReceivedFileSize(void) {
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->getReceivedFileSize();
		return 0;
	}

	inline unsigned long long getFileSize(void) {
		if (m_aptr_downloadItem.get()) {
			return m_aptr_downloadItem->getFileSize();
		}
		return 0;
	}

	inline string &getFilePath(void) {
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->getFilePath();
		return m_emptyString;
	}


	inline string &getContentName(void) {
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->getContentName();
		return m_emptyString;
	}

	inline string getSender(void) {
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->getSender();
		return m_emptyString;
	}

	inline string getEtag(void) {
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->getEtag();
		return m_emptyString;
	}

	inline string getTempPath(void) {
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->getFilePath();
		return m_emptyString;
	}

	inline void setId(int id) { m_id = id; }
	inline void setHistoryId(unsigned int i) { m_historyId = i; }
	inline unsigned int getHistoryId(void) { return m_historyId; }	// FIXME duplicated with m_id
	inline string &getTitle(void) {return m_title;}
	inline void setTitle(string &title) { m_title = title; }
	inline void setFileSize(unsigned long long fileSize) { m_fileSize = fileSize; }
	string &getRegisteredFilePath(void);
	inline void setRegisteredFilePath(string &r) { m_registeredFilePath = r; }
	string getUrl(void);
	string getReqHeaderField(void);
	string getReqHeaderValue(void);
	string getInstallDir(void);
	string getFileName(void);
	void setRetryData(string url,  string reqHeaderField, string reqHeaderValue,
			string installDir, string fileName, string tempFilePath, string etag);
	int getContentType(void) { return m_contentType; }
	inline void setContentType(int t) { m_contentType = t; }
	DL_TYPE::TYPE getDownloadType(void);
	inline void setDownloadType(DL_TYPE::TYPE t) { m_downloadType = t; }
	string &getThumbnailPath(void) { return m_thumbnailPath; }
	inline string &getIconPath(void) { return m_iconPath; }

	inline void setState(ITEM::STATE state) { m_state = state; }
	inline ITEM::STATE getState(void) { return m_state; }

	inline void setErrorCode(ERROR::CODE err) { m_errorCode = err; }
	inline ERROR::CODE getErrorCode(void) { return m_errorCode; }
	inline void setFinishedTime(double t) { m_finishedTime = t; }
	inline double getFinishedTime(void) { return m_finishedTime; }

	bool isFinished(void); /* include finish download state with error */
	bool isFinishedWithErr(void);
	bool isPreparingDownload(void); /* Before downloading start */
	bool isCompletedDownload(void); /* After stating installation */

#ifdef _ENABLE_OMA_DOWNLOAD
	inline bool isOMAMime(void)
	{
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->isOMAMime();
		return false;
	}
	inline string getUserMessage(void)
	{
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->getUserMessage();
		return string();
	}
	inline bool isNotifyFiinished(void)
	{
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->isNotifyFiinished();
		return true;
	}
	inline string installNotifyUrl(void)
	{
#ifdef _ENABLE_OMA_DOWNLOAD
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->getInstallNotifyUri();
#endif
		return m_installNotifyUrl;
	}
	inline void setInstallNotifyUrl(string url) { m_installNotifyUrl = url; }
	inline bool isExistedInstallNotifyUri(void)
	{
		if (m_aptr_downloadItem.get())
			return m_aptr_downloadItem->isExistedInstallNotifyUri();
		return false;
	}

	void setNotifyIdler(Ecore_Idler *idler) { m_notifyIdler = idler; }
#ifdef _ENABLE_WAITING_RO
	void sendInstallNotification(int status);
#endif
#endif
	/* Test code */
	const char *stateStr(void);

#ifdef _ENABLE_WAITING_RO
	void increaseTimerCount(void) { m_timerCount++; }
	int timerCount(void) { return m_timerCount; }
	void initTimerCount(void) { m_timerCount = 0; }
#endif

private:
	Item(void);
	Item(DownloadRequest &rRequest);

	inline void notify(void) { m_subjectForView.notify(); }

	void createSubscribeData(void);
	void extractTitle(void);
	void extractIconPath(void);

	void startUpdate(void);
	void failCaseUpdate(void);
	void createHistoryId(void);
	bool isExistedHistoryId(unsigned int id);
	void handleFinishedItem(void);
	void checkQueuedItem(void);

	auto_ptr<DownloadRequest> m_aptr_request;
	auto_ptr<DownloadItem> m_aptr_downloadItem;
	auto_ptr<DownloadNoti> m_aptr_noti;
	FileUtility m_fileObj;

	Subject m_subjectForView;
	auto_ptr<Observer> m_aptr_downloadObserver;
	auto_ptr<Observer> m_aptr_netEventObserver;

#ifdef _ENABLE_OMA_DOWNLOAD
	static Eina_Bool checkInstallNotifyCB(void *data);
#ifdef _ENABLE_WAITING_RO
	static Eina_Bool waitingRoForOmaCB(void *data);
#endif
#endif

#ifdef _ENABLE_WAITING_RO
	static Eina_Bool waitingRoCB(void *data);
	static Eina_Bool waitingRo(Item *item, bool isOmaCase);
#endif

	ITEM::STATE m_state;
	ERROR::CODE m_errorCode;
	string m_title;
	unsigned int m_historyId;
	int m_id;
	int m_contentType;
	unsigned long long m_fileSize;
	string m_iconPath; // FIXME Later:is it right to exist here? (ViewItem or not)
	string m_emptyString; // FIXME this is temporary to avoid crash when getFilePath() is called if m_aptr_downloaditem points nothing
	double m_finishedTime;
	DL_TYPE::TYPE m_downloadType;
	string m_registeredFilePath;
	string m_url;
	string m_fileName;
	string m_reqHeaderField;
	string m_reqHeaderValue;
	string m_installDir;
	string m_thumbnailPath;
	string m_tempFilePath;
	string m_etag;

#ifdef _ENABLE_OMA_DOWNLOAD
	Ecore_Idler *m_notifyIdler;
	string m_installNotifyUrl;
#endif

#ifdef _ENABLE_WAITING_RO
	Ecore_Timer *m_waitingRoTimer;
	int m_timerCount;
#endif

	bool m_gotFirstData;
};

#endif /* DOWNLOAD_MANAGER_ITEM_H */
