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

class ViewItem {
public:
	~ViewItem();
	static void create(Item *item);
	void destroy(void);
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

	static void checkChangedCB(void *data, Evas_Object *obj, void *event_info);

#ifndef _TIZEN_PUBLIC
	void sweepRight(void);
	void sweepLeft(void);
#endif

	const char *getMessage(void);
	const char *getBytesStr(void);
	const char *getHumanFriendlyBytesStr(unsigned long int bytes,
		bool progressOption);

	Elm_Genlist_Item_Class *elmGenlistStyle(void);

	inline Elm_Genlist_Item_Class *elmGenlistItemClass(void)
		{ return &dldGenlistStyle; }
	inline Elm_Genlist_Item_Class *elmGenlistHistoryItemClass(void)
		{ return &dldHistoryGenlistStyle; }
	inline Elm_Genlist_Item_Class *elmGenlistFailedItemClass(void)
		{ return &dldGenlistSlideStyle; }
	inline ITEM::STATE state(void) {
		if (m_item)
			return m_item->state();
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

	unsigned long int receivedFileSize(void);
	unsigned long int fileSize(void);
	const char *getTitle(void);
	inline const char *getErrMsg(void) { return m_item->getErrorMessage(); }
	const char *getIconPath(void) { return m_item->iconPath().c_str(); }

	inline Elm_Object_Item *genlistItem(void) { return m_glItem; }
	inline void setGenlistItem(Elm_Object_Item *glItem) { m_glItem = glItem; }
	inline void setProgressBar(Evas_Object *p) { m_progressBar = p; }

	void clickedDeleteButton(void);
	void clickedCancelButton(void);
	void clickedRetryButton(void);
	void clickedGenlistItem(void);
	void requestCancel(void);
	inline Eina_Bool checkedValue(void) { return m_checked; }
	void setCheckedValue(Eina_Bool b) { m_checked = b; }
	inline Evas_Object *checkedBtn(void) { return m_checkedBtn; }
	void setCheckedBtn(Evas_Object *e) { m_checkedBtn = e; }

	void updateCheckedBtn(void);

	inline int dateGroupType(void) { return m_dateGroupType; }
	void setDateGroupType (int t) { m_dateGroupType = t; }

	inline double finishedTime(void) { return m_item->finishedTime();}
	void extractDateGroupType(void);

	inline unsigned int historyId(void) { return m_item->historyId(); }
private:
	ViewItem(Item *item);

	void updateFromItem(void);
	Evas_Object *createProgressBar(Evas_Object *parent);
#ifndef _TIZEN_PUBLIC
	Evas_Object *createDeleteBtn(Evas_Object *parent);
	Evas_Object *createRetryBtn(Evas_Object *parent);
#endif
	Evas_Object *createCancelBtn(Evas_Object *parent);
	void retryViewItem(void);

	static void deleteBtnClickedCB(void *data, Evas_Object *obj,
		void *event_info);
	static void cancelBtnClickedCB(void *data, Evas_Object *obj,
		void *event_info);
	static void retryBtnClickedCB(void *data, Evas_Object *obj,
		void *event_info);

	auto_ptr<Observer> m_aptr_observer;
	Item *m_item;

	Elm_Genlist_Item_Class dldGenlistStyle;
	Elm_Genlist_Item_Class dldHistoryGenlistStyle;
	Elm_Genlist_Item_Class dldGenlistSlideStyle;
	Elm_Object_Item *m_glItem;
	Evas_Object *m_progressBar;
	Evas_Object *m_checkedBtn;
	Eina_Bool m_checked;
	bool m_isRetryCase;
	int m_dateGroupType;
};

#endif /* DOWNLOAD_MANAGER_VIEW_ITEM_H */
