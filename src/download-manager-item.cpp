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
 * @file	download-manager-item.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	item class for saving download data
 */

#include <iostream>
#include <time.h>
#include "download-manager-item.h"
#include "download-manager-common.h"
#include "download-manager-items.h"
#include "download-manager-viewItem.h"
#include "download-manager-history-db.h"
#include "download-manager-network.h"
#include "download-manager-notification.h"

Item::Item()
	: m_state(ITEM::IDLE)
	, m_errorCode(ERROR::NONE)
	, m_historyId(-1)
	, m_id(-1)
	, m_contentType(DM_CONTENT_UNKOWN)
	, m_fileSize(0)
	, m_finishedTime(0)
	, m_downloadType(DL_TYPE::TYPE_NONE)
#ifdef _ENABLE_OMA_DOWNLOAD
	, m_notifyIdler(NULL)
#endif
#ifdef _ENABLE_WAITING_RO
	, m_waitingRoTimer(NULL)
	, m_timerCount(0)
#endif
	, m_gotFirstData(false)
{
// FIXME Later : init private members
}

Item::Item(DownloadRequest &rRequest)
	: m_state(ITEM::IDLE)
	, m_errorCode(ERROR::NONE)
	, m_historyId(-1)
	, m_contentType(DM_CONTENT_UNKOWN)
	, m_fileSize(0)
	, m_finishedTime(0)
	, m_downloadType(DL_TYPE::TYPE_NONE)
#ifdef _ENABLE_OMA_DOWNLOAD
	, m_notifyIdler(NULL)
#endif
#ifdef _ENABLE_WAITING_RO
	, m_waitingRoTimer(NULL)
	, m_timerCount(0)
#endif
	, m_gotFirstData(false)
{
	m_title = string();
	m_iconPath = DM_UNKNOWN_ICON_PATH;
	m_aptr_request = auto_ptr<DownloadRequest>(new DownloadRequest(rRequest));
	m_aptr_noti = auto_ptr<DownloadNoti>(new DownloadNoti(this));
#ifdef _ENABLE_OMA_DOWNLOAD
	m_installNotifyUrl = string();
#endif
}

Item::~Item()
{
	DM_LOGD("");
}

void Item::create(DownloadRequest &rRequest)
{
	Item *newItem = new Item(rRequest);
	newItem->createHistoryId();
	Items &items = Items::getInstance();
	items.attachItem(newItem);

	ViewItem::create(newItem);
	DM_LOGV("newItem[%p]",newItem);

	newItem->download();
	newItem->notify();
}

Item *Item::createHistoryItem()
{
	string url = string();
	string fileName = string();
	string reqHeaderField = string();
	string reqHeaderValue = string();
	string installDir = string();
	DownloadRequest request(url, reqHeaderField, reqHeaderValue,
			installDir, fileName);

	Item *newItem = new Item(request);

	DM_LOGV("new History Item[%p]",newItem);

	return newItem;
}

void Item::attachHistoryItem()
{
	Items &items = Items::getInstance();

	DM_LOGI("attach History Item[%p] state[%d]",this, getState());
	items.attachItem(this);
	if (!isFinished()) {
		DM_LOGI("Change downloading state to fail state");
		setState(ITEM::FAIL_TO_DOWNLOAD);
		setErrorCode(ERROR::ENGINE_FAIL);
#ifdef _ENABLE_OMA_DOWNLOAD
		if (!m_installNotifyUrl.empty()) {
			OmaItem *omaItem = new OmaItem();
			omaItem->sendInstallNotification(952, m_installNotifyUrl);
		}
#endif
		//for notification
		notify();
	}
	ViewItem::create(this);
	extractIconPath();
}

void Item::destroy()
{
	DM_LOGD("");
	FileUtility fileObj;
	// FIXME prohibit to destroy if downloading
	if (!isFinished()) {
		DM_LOGE("Cannot delete this item:state[%d]",m_state);
		return;
	}

	if (isFinishedWithErr()) {
		DM_LOGI("Delete failed notification message");
		deleteCompleteNoti();
	}

	fileObj.deleteFile(getTempPath());
	setState(ITEM::DESTROY);
	notify();

	m_aptr_downloadItem->deSubscribe(m_aptr_downloadObserver.get());
	if (m_aptr_downloadObserver.get())
		m_aptr_downloadObserver->clear();
	else
		DM_LOGE("NULL Check:download observer");

#ifdef _ENABLE_OMA_DOWNLOAD
	if (m_notifyIdler) {
		ecore_idler_del(m_notifyIdler);
		m_notifyIdler = NULL;
	}
#endif

#ifdef _ENABLE_WAITING_RO
	if (m_waitingRoTimer) {
		ecore_timer_del(m_waitingRoTimer);
		m_waitingRoTimer = NULL;
	}
#endif

	Items::getInstance().detachItem(this);
}

void Item::deleteFromDB()
{
	DownloadHistoryDB::deleteItem(m_historyId);
}

void Item::download()
{
	setState(ITEM::REQUESTING);

	createSubscribeData();

	DownloadHistoryDB::createItemToDB(this);

	m_aptr_downloadItem->start(-1);
	m_id = getId();
	DownloadHistoryDB::updateDownloadIdToDB(this);
}

void Item::downloadFromQueuedState()
{
	m_aptr_downloadItem->start(-1);
	m_id = getId();
	DownloadHistoryDB::updateDownloadIdToDB(this);
}

void Item::createSubscribeData() // autoptr's variable of this class.
{
	m_aptr_downloadObserver = auto_ptr<Observer>(
		new Observer(updateCBForDownloadObserver, this, "downloadItemObserver"));
	m_aptr_netEventObserver = auto_ptr<Observer>(
		new Observer(netEventCBObserver, this, "netMgrObserver"));
	m_aptr_downloadItem = auto_ptr<DownloadItem>(new DownloadItem(m_aptr_request));
	if (!m_aptr_downloadItem.get()) {
		DM_LOGE("Fail to create download item");
		return;
	}
	m_aptr_downloadItem->subscribe(m_aptr_downloadObserver.get());
}

void Item::startUpdate(void)
{
	if (m_gotFirstData) {
		setState(ITEM::DOWNLOADING);
		if (!getRegisteredFilePath().empty())
			/* need to parse title again, because installed path can be changed */
			extractTitle();
		return;
	}

	DM_LOGD("");

	if (!m_aptr_downloadItem.get()) {
		DM_LOGE("Fail to get download item");
		return;
	}
	m_gotFirstData = true;
	setFileSize(m_aptr_downloadItem->getFileSize());
//	setState(ITEM::DOWNLOADING);
	setState(ITEM::RECEIVING_DOWNLOAD_INFO);

	extractTitle();
	DownloadUtil &util = DownloadUtil::getInstance();
	m_contentType = util.getContentType(
		m_aptr_downloadItem->getMimeType().c_str(), getFilePath().c_str());
	extractIconPath();
	DownloadHistoryDB::updateDownloadInfoToDB(this);
}

void Item::failCaseUpdate(void)
{
	FileUtility fileObj;
	DM_LOGD("");
	if (m_title.empty() && !getContentName().empty())
		extractTitle();
	extractIconPath();
	if (getErrorCode() == ERROR::NOT_ENOUGH_MEMORY) {
		if(fileObj.deleteFile(getFilePath()))
			DM_SLOGE("File %s is removed", getFilePath().c_str());
	}
	if (fileObj.deleteFile(getRegisteredFilePath()))
		DM_SLOGE("File %s is removed", getRegisteredFilePath().c_str());
}

#ifdef _ENABLE_OMA_DOWNLOAD
#ifdef _ENABLE_WAITING_RO
void Item::sendInstallNotification(int status)
{
	if (m_aptr_downloadItem.get())
		m_aptr_downloadItem->sendInstallNotification(status);
	return;
}
Eina_Bool Item::waitingRoForOmaCB(void *data)
{
	Item *item = (Item*)data;
	return waitingRo(item, true);
}
#endif
#endif

#ifdef _ENABLE_WAITING_RO
Eina_Bool Item::waitingRo(Item *item, bool isOmaCase)
{
	DownloadDrm &drmObj = DownloadDrm::getInstance();

	if (!item) {
		DM_LOGE("[CRITICAL]NULL Check:item");
		return ECORE_CALLBACK_CANCEL;
	}

	if(drmObj.validRo(item->getRegisteredFilePath().c_str()) ||
			item->timerCount() >= WAITING_RO_MAX_SECONDS) {
		DownloadUtil &util = DownloadUtil::getInstance();
		DM_LOGI("Valid RO or Timeout");
		if (!util.registerContent(item->getRegisteredFilePath(),
				item->getThumbnailPath())) {
			item->setState(ITEM::FAIL_TO_DOWNLOAD);
			item->setErrorCode(ERROR::FAIL_TO_INSTALL);
			if (isOmaCase)
				item->sendInstallNotification(952);
			item->failCaseUpdate();
			item->handleFinishedItem();
		} else {
			if (isOmaCase) {
				if (!item->isExistedInstallNotifyUri()) {
					item->setState(ITEM::FINISH_DOWNLOAD);
					item->handleFinishedItem();
				} else {
					item->setState(ITEM::NOTIFYING);
					item->sendInstallNotification(900);
				}
			} else {
				item->setState(ITEM::FINISH_DOWNLOAD);
				item->handleFinishedItem();
			}
		}
		if (isOmaCase && item->isExistedInstallNotifyUri()) {
			Ecore_Idler *idler = NULL;
			idler = ecore_idler_add(checkInstallNotifyCB, item);
			item->setNotifyIdler(idler);
		} else {
			item->setNotifyIdler(NULL);
		}
		DM_LOGI("Item[%p]::updateAfterCheckRo() notify() state[%d]",
				item, item->getState());
		item->notify();
		item->initTimerCount();
		return ECORE_CALLBACK_CANCEL;
	}

	item->increaseTimerCount();
	DM_LOGI("Check again RO Timer count[%d]", item->timerCount());
	item->setState(ITEM::WAITING_RO);
	item->notify();
	return ECORE_CALLBACK_RENEW;
}

Eina_Bool Item::waitingRoCB(void *data)
{
	Item *item = (Item*)data;
	return waitingRo(item, false);
}
#endif

void Item::updateFromDownloadItem(void)
{
	string finalPath;
	DownloadUtil &util = DownloadUtil::getInstance();

	if (isFinished()) {
		/* When user canceled item,
		 * the cancel operation is not operated in download daemon at once due to timing issue
		 * The canceled state is updated later.
		 * It is necessary to check queued time for that case.*/
		if (m_aptr_downloadItem->getState() == DL_ITEM::CANCELED) {
			checkQueuedItem();
		}
		DM_LOGI("Already finished. Ignored. dl_state[%d], state[%d]",
				m_aptr_downloadItem->getState(), getState());
		return;
	}

	switch (m_aptr_downloadItem->getState()) {
	case DL_ITEM::STARTED:
#ifdef _ENABLE_OMA_DOWNLOAD
		if (getState() == ITEM::REQUEST_USER_CONFIRM)
			setState(ITEM::REQUESTING);
#endif
		DownloadHistoryDB::updateStateToDB(this);
		break;
	case DL_ITEM::UPDATING:
		startUpdate();
		break;
	case DL_ITEM::COMPLETE_DOWNLOAD:
		setState(ITEM::REGISTERING_TO_SYSTEM);
		break;
	case DL_ITEM::SUSPENDED:
		setState(ITEM::SUSPENDED);
		m_aptr_downloadItem->resume();
		DownloadHistoryDB::updateStateToDB(this);
		break;
	case DL_ITEM::RESUMED:
		//setState(ITEM::RESUMED);
		break;
	case DL_ITEM::FINISHED:
		setState(ITEM::FINISH_DOWNLOAD);
		finalPath = util.saveContent(getRegisteredFilePath(), getInstallDir());
		if (finalPath.empty()) {
			setState(ITEM::FAIL_TO_DOWNLOAD);
			setErrorCode(ERROR::FAIL_TO_INSTALL);
			failCaseUpdate();
			handleFinishedItem();
			break;
		}
		setRegisteredFilePath(finalPath);
		/* need to parse title again, because installed path can be changed */
		extractTitle();
#ifdef _ENABLE_WAITING_RO
		if (getContentType() == DM_CONTENT_SD_DRM) {
			if (m_waitingRoTimer)
				ecore_timer_del(m_waitingRoTimer);
			m_waitingRoTimer = ecore_timer_add(1, waitingRoCB, this);
			return;
		}
#endif
		if (!util.registerContent(getRegisteredFilePath(), getThumbnailPath())) {
			setState(ITEM::FAIL_TO_DOWNLOAD);
			setErrorCode(ERROR::FAIL_TO_INSTALL);
			failCaseUpdate();
			handleFinishedItem();
			break;
		}
		handleFinishedItem();
		break;
	case DL_ITEM::CANCELED:
		/* Due to async callback,
		 * the content name and mime are received when the cancel event is received */
		setState(ITEM::CANCEL);
		failCaseUpdate();
		handleFinishedItem();
		break;
	case DL_ITEM::FAILED:
		/* Due to async callback,
		 * the content name and mime are received when the fail event is received */
		setState(ITEM::FAIL_TO_DOWNLOAD);
		setErrorCode(m_aptr_downloadItem->getErrorCode());
		failCaseUpdate();
		handleFinishedItem();
		break;
#ifdef _ENABLE_OMA_DOWNLOAD
	case DL_ITEM::NOTIFYING:
		setState(ITEM::NOTIFYING);
		finalPath = util.saveContent(getRegisteredFilePath(), getInstallDir());
		if (finalPath.empty()) {
			setState(ITEM::FAIL_TO_DOWNLOAD);
			setErrorCode(ERROR::FAIL_TO_INSTALL);
			m_aptr_downloadItem->sendInstallNotification(952);
			failCaseUpdate();
			handleFinishedItem();
			break;
		}
		setRegisteredFilePath(finalPath);
		extractTitle();
#ifdef _ENABLE_WAITING_RO
		if (getContentType() == DM_CONTENT_SD_DRM) {
			if (m_waitingRoTimer)
				ecore_timer_del(m_waitingRoTimer);
			// this is necessary in OMA download case. Because there are two download event.
			initTimerCount();
			m_waitingRoTimer = ecore_timer_add(1, waitingRoForOmaCB, this);
			return;
		}
#endif
		if (!util.registerContent(getRegisteredFilePath(), getThumbnailPath())) {
			setState(ITEM::FAIL_TO_DOWNLOAD);
			setErrorCode(ERROR::FAIL_TO_INSTALL);
			m_aptr_downloadItem->sendInstallNotification(952);
			failCaseUpdate();
			handleFinishedItem();
			break;
		}
		if (!m_aptr_downloadItem->isExistedInstallNotifyUri()) {
			DM_LOGI("No install notify URI");
			setState(ITEM::FINISH_DOWNLOAD);
			handleFinishedItem();
			notify();
		} else {
			m_aptr_downloadItem->sendInstallNotification(900);
			m_notifyIdler = ecore_idler_add(checkInstallNotifyCB, this);
		}
		return;
//		break;
	case DL_ITEM::REQUEST_USER_CONFIRM:
		DM_LOGD("DL_ITEM:REQUEST_USER_CONFIRM");
		DownloadHistoryDB::updateNotiUrlToHistoryDB(this);
		setState(ITEM::REQUEST_USER_CONFIRM);
		break;
#endif
	case DL_ITEM::QUEUED:
		DM_LOGD("DL_ITEM:QUEUED");
		setState(ITEM::QUEUED);
		break;
	default:
		break;
	}
	DM_LOGV("Item[%p]::updateFromDownloadItem() notify() dl_state[%d]state[%d]",
			this, m_aptr_downloadItem->getState(), getState());
	notify();
}

#ifdef _ENABLE_OMA_DOWNLOAD
void Item::doneNotifyFinished()
{
	/* For destroying item */
	DM_LOGD("");
	setState(ITEM::FINISH_DOWNLOAD);
	handleFinishedItem();
	notify();
}

Eina_Bool Item::checkInstallNotifyCB(void *data)
{
	Item *item = (Item *)data;
	if (!data) {
		DM_LOGE("[CRITICAL]NULL Check:item");
		return ECORE_CALLBACK_CANCEL;
	}
	if (item->isNotifyFiinished()) {
		item->doneNotifyFinished();
		item->setNotifyIdler(NULL);
		return ECORE_CALLBACK_CANCEL;
	} else {
		return ECORE_CALLBACK_RENEW;
	}

}
#endif

void Item::handleFinishedItem()
{
	//createHistoryId();
	m_finishedTime = time(NULL);
	if (this->m_state == ITEM::CANCEL)
		DownloadHistoryDB::updateCanceledItemToDB(this);
	else
		DownloadHistoryDB::updateHistoryToDB(this);
	/* Check QUEUED item and try to satrt downlaod it */
	checkQueuedItem();
}

void Item::checkQueuedItem()
{
	Items &items = Items::getInstance();
	int queudItemCount = 0;
	queudItemCount = items.checkQueuedItem();
	if (queudItemCount > 0) {
		DM_LOGI("Remained Queued Count[%d]", queudItemCount);
	}
}

void Item::extractTitle(void)
{
	if (!m_aptr_downloadItem.get()) {
		DM_LOGE("Fail to get download item");
		return;
	}
	size_t found = 0;
	string path = getRegisteredFilePath();
	if (!path.empty()) {
		found = path.find_last_of("/");
		if (found != string::npos)
			m_title = path.substr(found+1);
	} else if (!getContentName().empty()){
		m_title = getContentName();
	}
	DM_SLOGD("title [%s] contentName [%s]", m_title.c_str(),
			getContentName().c_str());
}

void Item::extractIconPath()
{
	if (isFinishedWithErr()) {
		m_iconPath = DM_FAILED_ICON_PATH;
		return;
	}
	// FIXME Later : change 2 dimension array??
	switch(m_contentType) {
	case DM_CONTENT_IMAGE :
		m_iconPath = DM_IMAGE_ICON_PATH;
		break;
	case DM_CONTENT_VIDEO :
		m_iconPath = DM_VIDEO_ICON_PATH;
		break;
	case DM_CONTENT_MUSIC:
		m_iconPath = DM_MUSIC_ICON_PATH;
		break;
	case DM_CONTENT_PDF:
		m_iconPath = DM_PDF_ICON_PATH;
		break;
	case DM_CONTENT_WORD:
		m_iconPath = DM_WORD_ICON_PATH;
		break;
	case DM_CONTENT_PPT:
		m_iconPath = DM_PPT_ICON_PATH;
		break;
	case DM_CONTENT_EXCEL:
		m_iconPath = DM_EXCEL_ICON_PATH;
		break;
	case DM_CONTENT_HTML:
		m_iconPath = DM_HTML_ICON_PATH;
		break;
	case DM_CONTENT_TEXT:
		m_iconPath = DM_TEXT_ICON_PATH;
		break;
	case DM_CONTENT_SD_DRM:
	case DM_CONTENT_DRM:
		m_iconPath = DM_DRM_ICON_PATH;
		break;
	case DM_CONTENT_FLASH:
		m_iconPath = DM_FLASH_ICON_PATH;
		break;
	case DM_CONTENT_TPK:
		m_iconPath = DM_TPK_ICON_PATH;
		break;
	case DM_CONTENT_VCAL:
		m_iconPath = DM_VCAL_ICON_PATH;
		break;
	case DM_CONTENT_UNKOWN:
	default:
		m_iconPath = DM_UNKNOWN_ICON_PATH;
		break;
	}
}

void Item::updateCBForDownloadObserver(void *data)
{
	if (data)
		static_cast<Item*>(data)->updateFromDownloadItem();
}

void Item::netEventCBObserver(void *data)
{
	/* It is only considerd that there is one network event which is suspend now.
	 * If other network evnet is added,
	 * it is needed to add function accroding to what kinds of network event is
	**/
	DM_LOGD("");
	if (data)
		static_cast<Item*>(data)->suspend();
}

bool Item::isExistedFile()
{
	return m_fileObj.isExistedFile(getRegisteredFilePath(), false);
}

bool Item::play()
{
	return m_fileObj.openFile(getRegisteredFilePath(), m_contentType);
}

/* Test code */
const char *Item::stateStr(void)
{
	switch((int)getState()) {
	case ITEM::IDLE:
		return "IDLE";
	case ITEM::REQUESTING:
		return "REQUESTING";
	case ITEM::PREPARE_TO_RETRY:
		return "PREPARE_TO_RETRY";
	case ITEM::RECEIVING_DOWNLOAD_INFO:
		return "RECEIVING_DOWNLOAD_INFO";
	case ITEM::DOWNLOADING:
		return "DOWNLOADING";
	case ITEM::REGISTERING_TO_SYSTEM:
		return "REGISTERING_TO_SYSTEM";
	case ITEM::SUSPENDED:
		return "SUSPENDED";
#ifdef _ENABLE_WAITING_RO
	case ITEM::WAITING_RO:
		return "WAITING_RO";
#endif
	case ITEM::FINISH_DOWNLOAD:
		return "FINISH_DOWNLOAD";
	case ITEM::FAIL_TO_DOWNLOAD:
		return "FAIL_TO_DOWNLOAD";
#ifdef _ENABLE_OMA_DOWNLOAD
	case ITEM::REQUEST_USER_CONFIRM:
		return "REQUEST_USER_CONFIRM";
	case ITEM::NOTIFYING:
		return "NOTIFYING";
#endif
	case ITEM::CANCEL:
		return "CANCEL";
	case ITEM::PLAY:
		return "PLAY";
	case ITEM::DESTROY:
		return "DESTROY";
	}
	return "Unknown State";
}

DL_TYPE::TYPE Item::getDownloadType()
{
	if (m_downloadType == DL_TYPE::TYPE_NONE) {
		if (!m_aptr_downloadItem.get()) {
			DM_LOGE("Fail to get download item");
			return DL_TYPE::TYPE_NONE;
		}
		m_downloadType = m_aptr_downloadItem->getDownloadType();
	}
	return m_downloadType;
}

string &Item::getRegisteredFilePath()
{
	if (m_registeredFilePath.empty()) {
		if (!m_aptr_downloadItem.get()) {
			DM_LOGE("Fail to get download item");
			return m_emptyString;
		}
		m_registeredFilePath = m_aptr_downloadItem->getRegisteredFilePath();
	}
	return m_registeredFilePath;
}

string Item::getUrl()
{
	if (m_url.empty()) {
		if (!m_aptr_downloadItem.get()) {
			DM_LOGE("Fail to get download item");
			return m_emptyString;
		}
		m_url = m_aptr_downloadItem->getUrl();
	}
	return m_url;
}

string Item::getReqHeaderField()
{
	if (m_reqHeaderField.empty()) {
		if (!m_aptr_downloadItem.get()) {
			DM_LOGE("Fail to get download item");
			return m_emptyString;
		}
		m_reqHeaderField = m_aptr_downloadItem->getReqHeaderField();
	}
	return m_reqHeaderField;
}

string Item::getReqHeaderValue()
{
	if (m_reqHeaderValue.empty()) {
		if (!m_aptr_downloadItem.get()) {
			DM_LOGE("Fail to get download item");
			return m_emptyString;
		}
		m_reqHeaderValue = m_aptr_downloadItem->getReqHeaderValue();
	}
	return m_reqHeaderValue;
}

string Item::getInstallDir()
{
	if (m_installDir.empty()) {
		if (!m_aptr_downloadItem.get()) {
			DM_LOGE("Fail to get download item");
			return m_emptyString;
		}
		m_installDir = m_aptr_downloadItem->getInstallDir();
	}
	return m_installDir;
}

string Item::getFileName()
{
	if (m_fileName.empty()) {
		if (!m_aptr_downloadItem.get()) {
			DM_LOGE("Fail to get download item");
			return m_emptyString;
		}
		m_fileName = m_aptr_downloadItem->getFileName();
	}
	return m_fileName;
}

void Item::createHistoryId()
{
	int count = 0;
	unsigned int tempId = time(NULL);

	while (isExistedHistoryId(tempId)) {
		tempId = rand_r(&tempId);
		count++;
		if (count > 100) {
			DM_LOGE("Fail to create unique ID");
			tempId = -1;
			break;
		}
		DM_LOGD("Random historyId[%ld]", m_historyId);
	}
	m_historyId = tempId;
}

bool Item::isExistedHistoryId(unsigned int id)
{
	Items &items = Items::getInstance();
	return items.isExistedHistoryId(id);
}

void Item::setRetryData(string url, string reqHeaderField, string reqHeaderValue,
		string installDir, string fileName, string tempFilePath, string etag)
{
	m_url = url;
	m_reqHeaderField = reqHeaderField;
	m_reqHeaderValue = reqHeaderValue;
	m_installDir = installDir;
	m_fileName = fileName;
	m_tempFilePath = tempFilePath;
	m_etag = etag;
	m_aptr_request->setUrl(url);
	m_aptr_request->setReqHeaderField(reqHeaderField);
	m_aptr_request->setReqHeaderValue(reqHeaderValue);
	m_aptr_request->setInstallDir(installDir);
	m_aptr_request->setFileName(fileName);
	m_aptr_request->setTempFilePath(tempFilePath);
	m_aptr_request->setEtag(etag);
	createSubscribeData();
}

void Item::deleteCompleteNoti()
{
	if (m_aptr_noti.get())
		m_aptr_noti->deleteCompleteNoti();
}

void Item::suspend()
{
	if (!isCompletedDownload() && !isPreparingDownload())
		m_aptr_downloadItem->suspend();
	else
		DM_LOGI("Cannot suspend due to invalid state[%d]", getState());
}

bool Item::retry()
{
	DM_LOGD("");
	if (m_aptr_downloadItem.get()) {
		setState(ITEM::PREPARE_TO_RETRY);
		if (!m_aptr_noti.get()) {
			m_aptr_noti = auto_ptr<DownloadNoti>(new DownloadNoti(this));
		}
		notify();
		/* Donot delete db, just update db. */
		if (m_aptr_downloadItem->retry(m_id, m_fileSize))
			setState(ITEM::REQUESTING);
		return true;
	} else {
		m_state = ITEM::FAIL_TO_DOWNLOAD;
		return false;
	}
}

void Item::clearForRetry()
{
	m_state = ITEM::IDLE;
	m_errorCode = ERROR::NONE;
	m_contentType = DM_CONTENT_UNKOWN;
	m_finishedTime = 0;
	m_downloadType = DL_TYPE::TYPE_NONE;
	m_gotFirstData = false;
	m_iconPath = m_iconPath.assign(DM_UNKNOWN_ICON_PATH);
	if (!m_registeredFilePath.empty())
		m_registeredFilePath.clear();
}

#ifdef _ENABLE_OMA_DOWNLOAD
void Item::handleUserConfirm(bool res)
{
	if (res) {
		m_aptr_downloadItem->start(-1);
		m_id = getId();
		DownloadHistoryDB::updateDownloadIdToDB(this);
	} else {
		m_state = ITEM::CANCEL;
		failCaseUpdate();
		m_aptr_downloadItem->sendInstallNotification(902);
		handleFinishedItem();
		notify();

	}

}
#endif

bool Item::isFinished()
{
	bool ret = false;
	switch (m_state) {
	case ITEM::FINISH_DOWNLOAD:
	case ITEM::FAIL_TO_DOWNLOAD:
	case ITEM::CANCEL:
	case ITEM::PLAY:
	case ITEM::DESTROY:
		ret = true;
		break;
	default:
		ret = false;
		break;
	}
	return ret;
}

bool Item::isFinishedWithErr()
{
	bool ret = false;
	switch (m_state) {
	case ITEM::FAIL_TO_DOWNLOAD:
	case ITEM::CANCEL:
		ret = true;
		break;
	default:
		ret = false;
		break;
	}
	return ret;
}

bool Item::isPreparingDownload()
{
	bool ret = false;
	switch (m_state) {
	case ITEM::IDLE:
	case ITEM::REQUESTING:
	case ITEM::PREPARE_TO_RETRY:
#ifdef _ENABLE_OMA_DOWNLOAD
	case ITEM::REQUEST_USER_CONFIRM:
#endif
	case ITEM::QUEUED:
		ret = true;
		break;
	default:
		ret = false;
		break;
	}
	return ret;

}

bool Item::isCompletedDownload()
{
	if (isFinished())
		return true;
#ifdef _ENABLE_OMA_DOWNLOAD
	if (m_state == ITEM::NOTIFYING)
		return true;
#endif
#ifdef _ENABLE_WAITING_RO
	if (m_state == ITEM::WAITING_RO)
		return true;
#endif
	return false;
}

bool Item::getNetworkBonding(void) {
	if (m_aptr_downloadItem.get()) {
		return m_aptr_downloadItem->getNetworkBonding();
	}
	return false;
}
