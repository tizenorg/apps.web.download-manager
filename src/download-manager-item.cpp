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

Item::Item()
	: m_state(ITEM::IDLE)
	, m_errorCode(ERROR::NONE)
	, m_historyId(-1)
	, m_contentType(DP_CONTENT_UNKOWN)
	, m_finishedTime(0)
	, m_downloadType(DL_TYPE::TYPE_NONE)
	, m_gotFirstData(false)
{
// FIXME Later : init private members
}

Item::Item(DownloadRequest &rRequest)
	: m_state(ITEM::IDLE)
	, m_errorCode(ERROR::NONE)
	, m_historyId(-1)
	, m_contentType(DP_CONTENT_UNKOWN)
	, m_finishedTime(0)
	, m_downloadType(DL_TYPE::TYPE_NONE)
	, m_gotFirstData(false)
{
	m_title = S_("IDS_COM_BODY_NO_NAME");
	m_iconPath = DP_UNKNOWN_ICON_PATH;
	m_aptr_request = auto_ptr<DownloadRequest>(new DownloadRequest(rRequest));	// FIXME ???
}

Item::~Item()
{
	DP_LOGD_FUNC();
}

void Item::create(DownloadRequest &rRequest)
{
//	DP_LOGD_FUNC();

	Item *newItem = new Item(rRequest);

	Items &items = Items::getInstance();
	items.attachItem(newItem);

	ViewItem::create(newItem);
	DP_LOGD("newItem[%p]",newItem);

	newItem->download();
}

Item *Item::createHistoryItem()
{
	string url = string();
	string cookie = string();
	DownloadRequest request(url,cookie);
	Item *newItem = new Item(request);
//	DP_LOGD_FUNC();

	DP_LOGD("new History Item[%p]",newItem);

	return newItem;
}

void Item::attachHistoryItem()
{
	Items &items = Items::getInstance();

	DP_LOGD("attach History Item[%p]",this);
	items.attachItem(this);
	ViewItem::create(this);
	extractIconPath();
}

void Item::destroy()
{
//	DP_LOG_FUNC();
	// FIXME prohibit to destroy if downloading
	if (!isFinished()) {
		DP_LOGE("Cannot delete this item. State[%d]",m_state);
		return;
	}
	DP_LOGD("Item::destroy() notify()");

	setState(ITEM::DESTROY);
	notify();
//	DP_LOG("Item::destroy() notify()... END");
	m_aptr_downloadItem->deSubscribe(m_aptr_downloadObserver.get());
	if (m_aptr_downloadObserver.get())
		m_aptr_downloadObserver->clear();
	else
		DP_LOGE("download observer pointer is NULL");
	/* When deleting item after download is failed */
	if (m_aptr_netEventObserver.get()) {
		NetMgr &netMgrInstance = NetMgr::getInstance();
		netMgrInstance.deSubscribe(m_aptr_netEventObserver.get());
	}
	Items::getInstance().detachItem(this);
}

void Item::deleteFromDB()
{
	DownloadHistoryDB::deleteItem(m_historyId);
}

void Item::download()
{
	NetMgr &netMgrInstance = NetMgr::getInstance();

//	DP_LOGD_FUNC();

	setState(ITEM::REQUESTING);

	createSubscribeData();

	netMgrInstance.subscribe(m_aptr_netEventObserver.get());

	m_aptr_downloadItem->start(false);

	DP_LOG("Item::download() notify()");
	notify();
}

void Item::createSubscribeData() // autoptr's variable of this class.
{
	m_aptr_downloadObserver = auto_ptr<Observer>(
		new Observer(updateCBForDownloadObserver, this, "downloadItemObserver"));
	m_aptr_netEventObserver = auto_ptr<Observer>(
		new Observer(netEventCBObserver, this, "netMgrObserver"));
	m_aptr_downloadItem = auto_ptr<DownloadItem>(new DownloadItem(m_aptr_request));
	if (!m_aptr_downloadItem.get()) {
		DP_LOGE("Fail to create download item");
		return;
	}
	m_aptr_downloadItem->subscribe(m_aptr_downloadObserver.get());
}

void Item::startUpdate(void)
{
	if (m_gotFirstData) {
		setState(ITEM::DOWNLOADING);
		if (!registeredFilePath().empty())
			/* need to parse title again, because installed path can be changed */
			extractTitle();
		return;
	}

	DP_LOGD_FUNC();

	if (!m_aptr_downloadItem.get()) {
		DP_LOGE("Fail to get download item");
		return;
	}
	m_gotFirstData = true;
//	setState(ITEM::DOWNLOADING);
	setState(ITEM::RECEIVING_DOWNLOAD_INFO);

	extractTitle();
	DownloadUtil &util = DownloadUtil::getInstance();
	m_contentType = util.getContentType(
		m_aptr_downloadItem->mimeType().c_str(), filePath().c_str());
	extractIconPath();
}

void Item::updateFromDownloadItem(void)
{
//	DP_LOGD_FUNC();
	DownloadUtil &util = DownloadUtil::getInstance();

	switch (m_aptr_downloadItem->state()) {
	case DL_ITEM::STARTED:
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
		break;
	case DL_ITEM::RESUMED:
		//setState(ITEM::RESUMED);
		break;
	case DL_ITEM::FINISHED:
		setState(ITEM::FINISH_DOWNLOAD);
		/* need to parse title again, because installed path can be changed */
		extractTitle();
		handleFinishedItem();
		util.registerContent(registeredFilePath());
		break;
	case DL_ITEM::CANCELED:
		setState(ITEM::CANCEL);
		handleFinishedItem();
		break;
	case DL_ITEM::FAILED:
		setState(ITEM::FAIL_TO_DOWNLOAD);
		setErrorCode(m_aptr_downloadItem->errorCode());
		handleFinishedItem();
		break;
	default:
		break;
	}

	DP_LOGD("Item[%p]::updateFromDownloadItem() notify() dl_state[%d]state[%d]", this, m_aptr_downloadItem->state(), state());
	notify();
}
void Item::handleFinishedItem()
{
	createHistoryId();
	m_finishedTime = time(NULL);
	DownloadHistoryDB::addToHistoryDB(this);
	/* If download is finished, it is not need to get network event */
	if (m_aptr_netEventObserver.get()) {
		NetMgr &netMgrInstance = NetMgr::getInstance();
		netMgrInstance.deSubscribe(m_aptr_netEventObserver.get());
	}
}

const char *Item::getErrorMessage(void)
{
	switch (m_errorCode) {
	case ERROR::NETWORK_FAIL:
		return S_("IDS_COM_POP_CONNECTION_FAILED");

	case ERROR::INVALID_URL:
		return S_("IDS_COM_POP_INVALID_URL");

	case ERROR::NOT_ENOUGH_MEMORY:
		return S_("IDS_COM_POP_NOT_ENOUGH_MEMORY");

	case ERROR::FAIL_TO_INSTALL:
		return S_("IDS_COM_POP_SAVING_FAILED");
	case ERROR::UNKNOWN:
	default:
		return S_("IDS_COM_POP_FAILED") ;
	}
}

void Item::extractTitle(void)
{
	if (!m_aptr_downloadItem.get()) {
		DP_LOGE("Fail to get download item");
		return;
	}
	size_t found = 0;
	string path;
	if (!registeredFilePath().empty()) {
		path = registeredFilePath();
		found = path.find_last_of("/");
		if (found != string::npos)
			m_title = path.substr(found+1);
	} else if (!contentName().empty()){
		m_title = contentName();
	}
	DP_LOGD("title [%s] contentName [%s]", m_title.c_str(),contentName().c_str());
}

void Item::extractIconPath()
{
	// FIXME Later : change 2 dimension array??
	switch(m_contentType) {
	case DP_CONTENT_IMAGE :
		m_iconPath = DP_IMAGE_ICON_PATH;
		break;
	case DP_CONTENT_VIDEO :
		m_iconPath = DP_VIDEO_ICON_PATH;
		break;
	case DP_CONTENT_MUSIC:
		m_iconPath = DP_MUSIC_ICON_PATH;
		break;
	case DP_CONTENT_PDF:
		m_iconPath = DP_PDF_ICON_PATH;
		break;
	case DP_CONTENT_WORD:
		m_iconPath = DP_WORD_ICON_PATH;
		break;
	case DP_CONTENT_PPT:
		m_iconPath = DP_PPT_ICON_PATH;
		break;
	case DP_CONTENT_EXCEL:
		m_iconPath = DP_EXCEL_ICON_PATH;
		break;
	case DP_CONTENT_HTML:
		m_iconPath = DP_HTML_ICON_PATH;
		break;
	case DP_CONTENT_TEXT:
		m_iconPath = DP_TEXT_ICON_PATH;
		break;
	case DP_CONTENT_RINGTONE:
		m_iconPath = DP_RINGTONE_ICON_PATH;
		break;
	case DP_CONTENT_DRM:
		m_iconPath = DP_DRM_ICON_PATH;
		break;
	case DP_CONTENT_JAVA:
		m_iconPath = DP_JAVA_ICON_PATH;
		break;
	case DP_CONTENT_UNKOWN:
	default:
		m_iconPath = DP_UNKNOWN_ICON_PATH;
		break;
	}
}

void Item::updateCBForDownloadObserver(void *data)
{
	DP_LOGD_FUNC();
	if (data)
		static_cast<Item*>(data)->updateFromDownloadItem();
}

void Item::netEventCBObserver(void *data)
{
	/* It is only considerd that there is one network event which is suspend now.
	 * If other network evnet is added,
	 * it is needed to add function accroding to what kinds of network event is
	**/
	DP_LOG_FUNC();
	if (data)
		static_cast<Item*>(data)->suspend();
}

bool Item::play()
{
	return m_fileOpener.openFile(registeredFilePath(), m_contentType);
}

/* Test code */
const char *Item::stateStr(void)
{
	switch((int)state()) {
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
	case ITEM::FINISH_DOWNLOAD:
		return "FINISH_DOWNLOAD";
	case ITEM::FAIL_TO_DOWNLOAD:
		return "FAIL_TO_DOWNLOAD";
	case ITEM::CANCEL:
		return "CANCEL";
	case ITEM::PLAY:
		return "PLAY";
	case ITEM::DESTROY:
		return "DESTROY";
	}
	return "Unknown State";
}

DL_TYPE::TYPE Item::downloadType()
{
	if (m_downloadType == DL_TYPE::TYPE_NONE) {
		if (!m_aptr_downloadItem.get()) {
			DP_LOGE("Fail to get download item");
			return DL_TYPE::TYPE_NONE;
		}
		m_downloadType = m_aptr_downloadItem->downloadType();
	}
	return m_downloadType;
}

string &Item::registeredFilePath()
{
	if (m_registeredFilePath.empty()) {
		if (!m_aptr_downloadItem.get()) {
			DP_LOGE("Fail to get download item");
			return m_emptyString;
		}
		m_registeredFilePath = m_aptr_downloadItem->registeredFilePath();
	}
	return m_registeredFilePath;
}

string &Item::url()
{
	if (m_url.empty()) {
		if (!m_aptr_downloadItem.get()) {
			DP_LOGE("Fail to get download item");
			return m_emptyString;
		}
		m_url = m_aptr_downloadItem->url();
	}
	return m_url;
}

string &Item::cookie()
{
	if (m_cookie.empty()) {
		if (!m_aptr_downloadItem.get()) {
			DP_LOGE("Fail to get download item");
			return m_emptyString;
		}
		m_cookie = m_aptr_downloadItem->cookie();
	}
	return m_cookie;
}


void Item::createHistoryId()
{
	int count = 0;
	unsigned tempId = time(NULL);
	while (isExistedHistoryId(tempId)) {
		srand((unsigned)(time(NULL)));
		tempId = rand();
		count++;
		if (count > 100) {
			DP_LOGE("Fail to create unique ID");
			tempId = -1;
			break;
		}
		DP_LOGD("random historyId[%ld]", m_historyId);
	}
	m_historyId = tempId;
}

bool Item::isExistedHistoryId(unsigned int id)
{
	Items &items = Items::getInstance();
	return items.isExistedHistoryId(id);
}

void Item::setRetryData(string &url, string &cookie)
{

	m_url = url;
	m_cookie = cookie;
	m_aptr_request->setUrl(url);
	m_aptr_request->setCookie(cookie);

	createSubscribeData();

}

bool Item::retry()
{
	DP_LOG_FUNC();
	if (m_aptr_downloadItem.get()) {
		NetMgr &netMgrInstance = NetMgr::getInstance();
		setState(ITEM::PREPARE_TO_RETRY);
		notify();
		DownloadHistoryDB::deleteItem(m_historyId);
		netMgrInstance.subscribe(m_aptr_netEventObserver.get());
		m_historyId = -1;
		m_aptr_downloadItem->retry();
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
	m_contentType = DP_CONTENT_UNKOWN;
	m_finishedTime = 0;
	m_downloadType = DL_TYPE::TYPE_NONE;
	m_gotFirstData = false;
}

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
	if (isPreparingDownload() ||
			m_state == ITEM::DOWNLOADING)
		return false;
	else
		return true;
}

