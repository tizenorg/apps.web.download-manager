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
#include "vconf.h"

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
	void clickedItemFromNoti(unsigned int historyId);
	void show(void);
	void hide(void);
	void pause(void);
	void resume(void);
	static void lockStateChangedCB(keynode_t *node, void *user_data);

	void attachViewItem(ViewItem *viewItem);
	void detachViewItem(ViewItem *viewItem);
	void changedRegion(void);
	void showErrPopup(string &desc);
	void showMemoryFullPopup(void);
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
	void setActivatedLockScreen(bool value) { m_activatedLockScreen = value; }

private:
	static void hideNotifyInfoCB(void *data, Evas *evas, Evas_Object *obj, void *event);
	static void selectAllChangedCB(void *data, Evas_Object *obj,
			void *event_info);
	static void genlistClickCB(void *data, Evas_Object *obj, void *event_info);
	static void cancelClickCB(void *data, Evas_Object *obj, void *event_info);
	static void errPopupResponseCB(void *data, Evas_Object *obj, void *event_info);
	static void retryPopupCancelCB(void *data, Evas_Object *obj, void *event_info);
	static void retryPopupRetryCB(void *data, Evas_Object *obj, void *event_info);
	static void memoryFullPopupCancelCB(void *data, Evas_Object *obj, void *event_info);
	static void memoryFullPopupMyFilesCB(void *data, Evas_Object *obj, void *event_info);
#ifdef _HW_BACK_KEY
	static void deleteBtnCB(void *data, Evas_Object *obj, void *event_info);
	static void popupBackCB(void *data, Evas_Object *obj, void *event_info);
	static void cancelToolBarBtnCB(void *data, Evas_Object *obj, void *event_info);
	static void deleteToolBarBtnCB(void *data, Evas_Object *obj, void *event_info);
	static void deletePopupDeleteCB(void *data, Evas_Object *obj, void *event_info);
#else
	static void cbDeleteBtnCB(void *data, Evas_Object *obj, void *event_info);
#endif
	static Eina_Bool popCB(void *data, Elm_Object_Item *it);
#ifdef _ENABLE_OMA_DOWNLOAD
	static void omaPopupResponseOKCB(void *data, Evas_Object *obj, void *event_info);
	static void omaPopupResponseCancelCB(void *data, Evas_Object *obj, void *event_info);
#endif

private:
	DownloadView();
	~DownloadView();

	void setTheme(void);
	void setIndicator(Evas_Object *window);
	Evas_Object *createWindow(const char *windowName);
	Evas_Object *createBackground(Evas_Object *window);
	Evas_Object *createConformant(Evas_Object *window);
	Evas_Object *createLayout(Evas_Object *parent);
	void createTheme(void);
	void createNaviBar(void);
#ifdef	_HW_BACK_KEY
	void createToolBar(void);
	void destroyToolBar(void);
	void createDeleteBtn(void);
	void destroyDeleteBtn(void);
	void showDeletePopup(void);
#else
	void createBackBtn(void);
	void createControlBar(void);
#endif
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
	void createSelectAll(void);
	void destroyCheckedItem(void);
	void showDeletedNotify(void);
	void showSelectedNotify(int selectedCount);
	void cleanGenlistData();

	Evas_Object *eoWindow;
	Evas_Object *eoBackground;
	Evas_Object *eoConform;
	Evas_Object *eoLayout;
	Evas_Object *eoEmptyNoContent;
	Evas_Object *eoNaviBar;
	Elm_Object_Item *eoNaviBarItem;
#ifdef	_HW_BACK_KEY
	Evas_Object *eoToolBar;
	Elm_Object_Item *eoToolBarItem;
#else
	Evas_Object *eoBackBtn;
	Evas_Object *eoDeleteBtn;
#endif
	Evas_Object *eoBox;
	Evas_Object *eoDldList;
	Evas_Object *eoPopup;
	Evas_Object *eoAllCheckedBox;
	Eina_Bool m_allChecked;
#ifdef _ENABLE_OMA_DOWNLOAD
	ViewItem *prevOmaViewItem;
#endif
	int m_viewItemCount;
	bool m_silentMode;
	bool m_activatedLockScreen;
};

#endif /* DOWNLOAD_MANAGER_VIEW_H */
