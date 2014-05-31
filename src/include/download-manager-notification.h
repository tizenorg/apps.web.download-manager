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
 * @file	download-manager-notification.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief   Noti APIs and interface
 */

#ifndef DOWNLOAD_MANAGER_NOTIFICATION_H
#define DOWNLOAD_MANAGER_NOTIFICATION_H

#include "notification.h"

#include "download-manager-common.h"
#include "download-manager-event.h"
#include "download-manager-item.h"
#include <memory>

class DownloadNoti
{
public:
	DownloadNoti(Item *item);
	~DownloadNoti(void);

	void updateFromItem(void);
	static void updateCB(void *);
	static void clearOngoingNoti(void);
	void deleteCompleteNoti(void);
private:
	void addOngoingNoti(void);
	void updateTitleOngoingNoti();
	void updateOngoingNoti(void);
	string convertSizeStr(unsigned long long size);
	void addCompleteNoti(string &msg, bool isSuccess);
	void freeNotiData(notification_h notiHandle);
	int m_notiId;
	notification_h m_notiHandle;
	Item *m_item;
	auto_ptr<Observer> m_aptr_observer;
};

#endif//DOWNLOAD_MANAGER_NOTIFICATION_H
