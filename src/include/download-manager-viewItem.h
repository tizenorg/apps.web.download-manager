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
 * @file 	download-manager-viewItem.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Item class for Download view
 */

#ifndef DOWNLOAD_MANAGER_VIEW_ITEM_H
#define DOWNLOAD_MANAGER_VIEW_ITEM_H

#include "download-manager-event.h"
#include "download-manager-item.h"
#include <Elementary.h>
#include <memory>

using namespace std;

namespace VIEWITEM_GROUP {
enum GROUP {
	TODAY = 0,
	YESTERDAY,
	OLDER,
	NONE,
};
}

class ViewItem {
public:
	ViewItem(VIEWITEM_GROUP::GROUP type);
	~ViewItem();
	static void create(Item *item);
	void destroy(void);
	void cancel(void);
	inline void deleteFromDB(void) {
		if (m_item)
			m_item->deleteFromDB();
	}

	inline void setItem(Item *item) { m_item = item; }
	static void updateCB(void *);


	static char *getGenlistLabelCB(void *data, Evas_Object *obj,
		const char *part);
	char *getGenlistLabel(Evas_Object *obj, const char *part);

	static Evas_Object *getGenlistIconCB(void *data, Evas_Object *obj,
		const char *part);
	Evas_Object *getGenlistIcon(Evas_Object *obj, const char *part);
#ifdef _TIZEN_2_3_UX
	static void checkBoxChangedCB(void *data, Evas_Object *obj,
		void *event_info);
#endif
	static void deleteGenlistData(void *data, Evas_Object *obj);

	const char *getGroupTitle();
	const char *getMessage(void);
	const char *getBytesStr(void);
	void getHumanFriendlyBytesStr(unsigned long long bytes,
		bool progressOption, char **buff);

	Elm_Genlist_Item_Class *elmGenlistStyle(void);

	inline Elm_Genlist_Item_Class *elmGenlistItemClass(void)
		{ return &dldGenlistStyle; }
	inline Elm_Genlist_Item_Class *elmGenlistHistoryItemClass(void)
		{ return &dldHistoryGenlistStyle; }
	inline ITEM::STATE getState(void) {
		if (m_item)
			return m_item->getState();
		else
			return ITEM::IDLE;
	}
	inline const char* stateStr(void) {
		if (m_item)
			return m_item->stateStr();
		else
			return NULL;
	}
	inline bool isFinished(void) {
		if (m_item)
			return m_item->isFinished();
		else
			return false;
	}
	inline bool isFinishedWithErr(void) {
		if (m_item)
			return m_item->isFinishedWithErr();
		else
			return false;
	}
	inline bool isPreparingDownload(void) {
		if (m_item)
			return m_item->isPreparingDownload();
		else
			return false;
	}
	inline bool isCompletedDownload(void) {
		if (m_item)
			return m_item->isCompletedDownload();
		else
			return false;
	}
	inline void setInsertAtFirst(bool insertAtFirst) {
		m_insertAtFirst = insertAtFirst;
	}
	inline bool getInsertAtFirst() { return m_insertAtFirst; }
	inline bool isGroupTitle(void) { return m_isGroupTitle; }

	unsigned long long getReceivedFileSize(void);
	unsigned long long getFileSize(void);
	const char *getTitle(void);
	const char *getIconPath(void) { return m_item->getIconPath().c_str(); }

	inline Elm_Object_Item *genlistItem(void) { return m_glItem; }
	inline void setGenlistItem(Elm_Object_Item *glItem) { m_glItem = glItem; }
	inline string senderName(void) { return m_item->getSender(); }

	void clickedCancelButton(void);
	void clickedCanceledRetryButton(void);
	void clickedRetryButton(void);
	void clickedGenlistItem(void);
	inline Eina_Bool checkedValue(void) { return m_checked; }
	void setCheckedValue(Eina_Bool b) { m_checked = b; }
	inline Evas_Object *checkedBtn(void) { return m_checkedBtn; }
	void setCheckedBtn(Evas_Object *e) { m_checkedBtn = e; }

	inline double getFinishedTime(void) { return m_item->getFinishedTime();}
	inline VIEWITEM_GROUP::GROUP getItemGroup(void) { return m_group; }
	inline void setItemGroup(VIEWITEM_GROUP::GROUP group) { m_group = group; }
	void extractDateGroupType(void);

	inline unsigned int getHistoryId(void) { return m_item->getHistoryId(); }

	void setIsClickedFromNoti(bool b) { m_isClickedFromNoti = b; }
	bool isClickedFromNoti(void) { return m_isClickedFromNoti; }

#ifdef _ENABLE_OMA_DOWNLOAD
	void responseUserConfirm(bool res);
#endif
private:
	ViewItem(Item *item);

	void updateFromItem(void);
	Evas_Object *createProgressBar(Evas_Object *parent);
	Evas_Object *createCancelBtn(Evas_Object *parent);
	void retryViewItem(void);

	static void cancelBtnClickedCB(void *data, Evas_Object *obj,
		void *event_info);

	auto_ptr<Observer> m_aptr_observer;
	Item *m_item;

	static Elm_Genlist_Item_Class dldGenlistStyle;
	static Elm_Genlist_Item_Class dldHistoryGenlistStyle;
	static Elm_Genlist_Item_Class dldGroupTitleGenlistStyle;
	Elm_Object_Item *m_glItem;
	Evas_Object *m_checkedBtn;
	Eina_Bool m_checked;
	bool m_isRetryCase;
	bool m_isClickedFromNoti;
	bool m_isGroupTitle;
	bool m_insertAtFirst;
	VIEWITEM_GROUP::GROUP m_group;

    static const unsigned int GENLIST_ICON_SIZE = 46;
};

#endif /* DOWNLOAD_MANAGER_VIEW_ITEM_H */
