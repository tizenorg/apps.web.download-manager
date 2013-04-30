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
 * @file 	download-manager-view.h
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	Download UI View manager
 */

#ifndef DOWNLOAD_MANAGER_VIEW_H
#define DOWNLOAD_MANAGER_VIEW_H

#include <Elementary.h>
#include <libintl.h>

#include <vector>
#include "download-manager-common.h"
#include "download-manager-viewItem.h"
#include "download-manager-dateTime.h"

enum {
	POPUP_EVENT_EXIT = 0,
	POPUP_EVENT_ERR,
};

class DownloadView {
public:
	static DownloadView& getInstance(void) {
		static DownloadView inst;
		return inst;
	}

	Evas_Object *create(void);
	void destroy(void);
	void createView(void);
	void activateWindow(void);
#ifdef _ENABLE_ROTATE
	void rotateWindow(int angle);
#endif
	void show(void);
	void hide(void);
	void pause(void);

	void attachViewItem(ViewItem *viewItem);
	void detachViewItem(ViewItem *viewItem);
	void changedRegion(void);
	void showErrPopup(string &desc);
	void showRetryPopup(ViewItem *viewItem, string msg);
	void update(void);
	void update(ViewItem *viewItem);
	void update(Elm_Object_Item *glItem);
	void updateLang(void);
	void showViewItem(int id, const char *title);
	void playContent(int id, const char *title);
	void handleChangedAllCheckedState(void);
	void handleCheckedState(void);
	bool isGenlistEditMode(void);
	void handleGenlistGroupItem(int type);
	void moveRetryItem(ViewItem *viewItem);
#ifdef _ENABLE_OMA_DOWNLOAD
	void showOMAPopup(string msg, ViewItem *viewItem);
#endif
	void setSilentMode(bool value) { m_silentMode = value; }

private:
	static void showNotifyInfoCB(void *data, Evas *evas, Evas_Object *obj, void *event);
	static void hideNotifyInfoCB(void *data, Evas *evas, Evas_Object *obj, void *event);
	static void backBtnCB(void *data, Evas_Object *obj, void *event_info);
	static void cbDeleteBtnCB(void *data, Evas_Object *obj, void *event_info);
	static void cbItemCancelCB(void *data, Evas_Object *obj, void *event_info);
	static void selectAllClickedCB(void *data, Evas *evas, Evas_Object *obj,
		void *event_info);
	static void selectAllChangedCB(void *data, Evas_Object *obj,
		void *event_info);
	static void genlistClickCB(void *data, Evas_Object *obj, void *event_info);
	static void cancelClickCB(void *data, Evas_Object *obj, void *event_info);
	static void errPopupResponseCB(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool deletedNotifyTimerCB(void *data);
	static void retryPopupCancelCB(void *data, Evas_Object *obj, void *event_info);
	static void retryPopupRetryCB(void *data, Evas_Object *obj, void *event_info);

#ifdef _ENABLE_OMA_DOWNLOAD
	static void omaPopupResponseOKCB(void *data, Evas_Object *obj, void *event_info);
	static void omaPopupResponseCancelCB(void *data, Evas_Object *obj, void *event_info);
#endif

private:
	DownloadView();
	~DownloadView();

	inline void destroyEvasObj(Evas_Object *e) { if(e) {evas_object_del(e); e = NULL;} }
	void setTheme(void);
	void setIndicator(Evas_Object *window);
	Evas_Object *createWindow(const char *windowName);
	Evas_Object *createBackground(Evas_Object *window);
	Evas_Object *createConformant(Evas_Object *window);
	Evas_Object *createLayout(Evas_Object *parent);
	void createTheme(void);
	void createNaviBar(void);
	void createBackBtn(void);
	void createControlBar(void);
	void createBox(void);
	void createList(void);

	void removeTheme(void);

	void addViewItemToGenlist(ViewItem *viewItem);
	void createGenlistItem(ViewItem *viewItem);
	void showEmptyView(void);
	void hideEmptyView(void);

	void removePopup(void);
#ifdef _ENABLE_OMA_DOWNLOAD
	void removeOnlyPopupObj(void);
#endif
	void showGenlistEditMode(void);
	void hideGenlistEditMode(void);
	void createSelectAllLayout(void);
	void changeAllCheckedValue(void);
	void destroyCheckedItem(void);
	void showNotifyInfo(int type, int selectedCount);
	void destroyNotifyInfo(void);
	void createNotifyInfo(void);
	void cleanGenlistData();

	Evas_Object *eoWindow;
	Evas_Object *eoBackground;
	Evas_Object *eoConform;
	Evas_Object *eoLayout;
	Evas_Object *eoEmptyNoContent;
	Evas_Object *eoNaviBar;
	Elm_Object_Item *eoNaviBarItem;
	Evas_Object *eoBackBtn;
	Evas_Object *eoControlBar;
	Evas_Object *eoDeleteBtn;
	Evas_Object *eoBoxLayout;
	Evas_Object *eoBox;
	Evas_Object *eoDldList;
	Evas_Object *eoPopup;
	Evas_Object *eoSelectAllLayout;
	Evas_Object *eoAllCheckedBox;
	Evas_Object *eoNotifyInfoLayout;
	Eina_Bool m_allChecked;
#ifdef _ENABLE_OMA_DOWNLOAD
	ViewItem *prevOmaViewItem;
#endif
	int m_viewItemCount;
	bool m_silentMode;
};

#endif /* DOWNLOAD_MANAGER_VIEW_H */
