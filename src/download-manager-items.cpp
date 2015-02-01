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
 * @file	download-manager-items.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Interface API with item list
 */
#include "download-manager-items.h"

void Items::attachItem(Item *item)
{
	m_items.push_back(item);
}

void Items::detachItem(Item *item)
{
	vector <Item *>::iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it) {
		if (*it == item) {
			delete item;
			m_items.erase(it);
			return;
		}
	}
}

bool Items::isExistedHistoryId(unsigned int id)
{
	vector <Item *>::iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it) {
		if ((*it)->getHistoryId() == id ) {
			DM_SLOGD("historyId[%ld],title[%s]",
				(*it)->getHistoryId(), (*it)->getTitle().c_str());
			return true;
		}
	}
	return false;
}

int Items::checkQueuedItem()
{
	int count = 0;
	vector <Item *>::iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it) {
		if ((*it)->getState() == ITEM::QUEUED ) {
			(*it)->downloadFromQueuedState();
			count++;
		}
	}
	return count;
}

