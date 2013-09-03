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
 * @file	download-manager-view.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	UI manager class for download list view and delete view
 */
#include <sstream>
#include <queue>
#include "utilX.h"
#include "efl_assist.h"

#include "download-manager-view.h"
#include "download-manager-history-db.h"

static void destroy_window_cb(void *data, Evas_Object *obj, void *event);

enum {
	DOWNLOAD_NOTIFY_SELECTED,
	DOWNLOAD_NOTIFY_DELETED
};

DownloadView::DownloadView(void)
	: eoWindow(NULL)
	, eoBackground(NULL)
	, eoConform(NULL)
	, eoLayout(NULL)
	, eoEmptyNoContent(NULL)
	, eoNaviBar(NULL)
	, eoNaviBarItem(NULL)
	, eoToolBar(NULL)
	, eoToolBarItem(NULL)
	, eoBox(NULL)
	, eoDldList(NULL)
	, eoPopup(NULL)
	, eoAllCheckedBox(NULL)
	, m_allChecked(EINA_FALSE)
#ifdef _ENABLE_OMA_DOWNLOAD
	, prevOmaViewItem(NULL)
#endif
	, m_viewItemCount(0)
	, m_silentMode(false)
	, m_activatedLockScreen(false)
{
// FIXME Later : init private members
	DownloadEngine &engine = DownloadEngine::getInstance();
	engine.initEngine();
	DateUtil &inst = DateUtil::getInstance();
	inst.updateLocale();
}

DownloadView::~DownloadView()
{
	DM_LOGD("");
}

Evas_Object *DownloadView::create(void)
{
	Evas_Object *window = NULL;
	Evas_Object *conformant = NULL;
	FileUtility fileObj;
	window = createWindow(PACKAGE);
	if (!window)
		return NULL;

	if (elm_win_wm_rotation_supported_get(window)) {
		int rots[4] = {0, 90, 180, 270};
		elm_win_wm_rotation_available_rotations_set(window, rots, 4);
	}
	/* If there are some job when the rotation event is happened, please enable this.
	 * It is handled to rotate of a window by window manager. Please don't care it */
//	evas_object_smart_callback_add(window, "wm,rotation,changed", _rot_changed_cb, NULL);

	createBackground(window);
	conformant = createConformant(window);
	createLayout(conformant);
	setIndicator(window);
	createView();
	fileObj.cleanTempDir();
	return window;
}

void DownloadView::destroy()
{
	Elm_Object_Item *it = NULL;
	ViewItem *item = NULL;
	DownloadEngine &engine = DownloadEngine::getInstance();

	if (vconf_ignore_key_changed(
			VCONFKEY_IDLE_LOCK_STATE, lockStateChangedCB) != 0) {
		DM_LOGE("Fail to vconf_ignore_key_changed");
	}
	it = elm_genlist_first_item_get(eoDldList);

	while (it) {
		item = (ViewItem *)elm_object_item_data_get(it);
		if (item) {
			item->cancel();
//			item->destroy();
		}
		it = elm_genlist_item_next_get(it);
	}
	engine.deinitEngine();
	if (isGenlistEditMode()) {
		hideGenlistEditMode();
	}
	removePopup();
	if(eoAllCheckedBox) {
		evas_object_del(eoAllCheckedBox);
		eoAllCheckedBox = NULL;
	}
	if(eoEmptyNoContent) {
		evas_object_del(eoEmptyNoContent);
		eoEmptyNoContent = NULL;
	}
	if(eoDldList)
		elm_genlist_clear(eoDldList);
	if(eoDldList) {
		evas_object_del(eoDldList);
		eoDldList = NULL;
	}
	if(eoToolBar) {
		evas_object_del(eoToolBar);
		eoToolBar = NULL;
	}
	if(eoBox) {
		evas_object_del(eoBox);
		eoBox = NULL;
	}
	if(eoNaviBar) {
		evas_object_del(eoNaviBar);
		eoNaviBar = NULL;
	}
	if(eoLayout) {
		evas_object_del(eoLayout);
		eoLayout = NULL;
	}
	if(eoConform) {
		evas_object_del(eoConform);
		eoConform = NULL;
	}
	if(eoBackground) {
		evas_object_del(eoBackground);
		eoBackground = NULL;
	}
	if(eoWindow) {
		evas_object_del(eoWindow);
		eoWindow = NULL;
	}
}


void DownloadView::show()
{
	DM_LOGI("");
	if (!m_silentMode) {
		evas_object_show(eoWindow);
		elm_win_activate(eoWindow);
	}

	/* FIXME later : For performance, this is re-considered later
	 * This is added to update date information when system time is changed by user*/
	update();
}

void DownloadView::hide()
{
	DM_LOGI("");
	removePopup();
	if (isGenlistEditMode()) {
		hideGenlistEditMode();
	}
	evas_object_hide(eoWindow);
	elm_win_lower(eoWindow);
}

void DownloadView::lockStateChangedCB(keynode_t *node, void *user_data)
{
	DownloadView *view = NULL;
	if (!user_data) {
		DM_LOGE("NULL Check:user_data");
		return;
	}
	if (!node) {
		DM_LOGE("NULL Check:node");
		return;
	}
	if (node->keyname)
		DM_LOGV("keyname:%s", node->keyname);
	DM_LOGI("value:%d", node->value.i);
	if (node->value.i == VCONFKEY_IDLE_LOCK) {
		view = (DownloadView *)user_data;
		view->setActivatedLockScreen(true);
		DM_LOGV("Activated Lock Screen");
	}}

void DownloadView::pause()
{
	DM_LOGI("");
//	removePopup();

	if (vconf_notify_key_changed(
			VCONFKEY_IDLE_LOCK_STATE, lockStateChangedCB, this) != 0) {
		DM_LOGE("Fail to vconf_notify_key_changed");
	}
}


void DownloadView::resume()
{
	DM_LOGI("");
	if (vconf_ignore_key_changed(
			VCONFKEY_IDLE_LOCK_STATE, lockStateChangedCB) != 0) {
		DM_LOGD("Fail to vconf_ignore_key_changed");
	}
	if (isGenlistEditMode() && !m_activatedLockScreen) {
		DM_LOGI("Clear genlist mode");
		hideGenlistEditMode();
	}
	m_activatedLockScreen = false;
}

void DownloadView::activateWindow()
{
	if (!eoWindow)
		create();
	else
		resume();
	show();
}

void DownloadView::clickedItemFromNoti(unsigned id)
{
	Elm_Object_Item *it = NULL;
	ViewItem *item = NULL;

	it = elm_genlist_first_item_get(eoDldList);

	while (it) {
		item = (ViewItem *)elm_object_item_data_get(it);
		/* elm_genlist_item_select_mode_get is needed to check group item */
		if (item && item->historyId() == id) {
			elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
			string msg = __("IDS_DM_POP_FILE_NOT_RECEIVED_DOWNLOAD_AGAIN_Q");
			item->setIsClickedFromNoti(true);
			showRetryPopup(item, msg);
			break;
		}
		it = elm_genlist_item_next_get(it);
	}
}

void DownloadView::showViewItem(int id, const char *title)
{
	DM_LOGD("");
}

void DownloadView::setIndicator(Evas_Object *window)
{
	elm_win_indicator_mode_set(window, ELM_WIN_INDICATOR_SHOW);
}

Evas_Object *DownloadView::createWindow(const char *windowName)
{
	eoWindow = elm_win_add(NULL, windowName, ELM_WIN_BASIC);
	if (eoWindow) {
		elm_win_title_set(eoWindow, __("IDS_DM_HEADER_DOWNLOAD_MANAGER_ABB"));
		elm_win_conformant_set(eoWindow, 1);
		evas_object_smart_callback_add(eoWindow, "delete,request",
				destroy_window_cb,	static_cast<void*>(this));
	}
	return eoWindow;
}

Evas_Object *DownloadView::createBackground(Evas_Object *window)
{
	if (!window)
		return NULL;

	eoBackground = elm_bg_add(window);
	if (eoBackground) {
		evas_object_size_hint_weight_set(eoBackground, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(window, eoBackground);
		evas_object_show(eoBackground);
	} else {
		DM_LOGE("Fail to create bg object");
	}
	return eoBackground;
}

Evas_Object *DownloadView::createConformant(Evas_Object *window)
{
	DM_LOGD("");
	if (!window)
		return NULL;
	eoConform = elm_conformant_add(window);
	if (eoConform) {
		evas_object_size_hint_weight_set(eoConform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(window, eoConform);
	}
	if (!eoConform) {
		DM_LOGE("Fail to create conformant object");
		return NULL;
	}
	evas_object_show(eoConform);
	return eoConform;
}

Evas_Object *DownloadView::createLayout(Evas_Object *parent)
{
	if (!parent) {
		DM_LOGE("Invalid Paramter");
		return NULL;
	}

	eoLayout = elm_layout_add(parent);
	if (eoLayout) {
		if (!elm_layout_theme_set(eoLayout, "layout", "application", "default" ))
			DM_LOGE("Fail to set elm_layout_theme_set");

		evas_object_size_hint_weight_set(eoLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_content_set(parent, eoLayout);
		evas_object_show(eoLayout);
	} else {
		DM_LOGE("Fail to create layout");
	}
	return eoLayout;
}

void DownloadView::createView()
{
	DM_LOGD("");
	createNaviBar();
	createList();
	if (m_viewItemCount < 1)
		showEmptyView();
}

void DownloadView::createNaviBar()
{
	DM_LOGD("");
	eoNaviBar = elm_naviframe_add(eoLayout);
	if (!eoNaviBar) {
		DM_LOGE("Null Check:eoNaviBar");
	}

	elm_object_part_content_set(eoLayout, "elm.swallow.content", eoNaviBar);
	elm_naviframe_prev_btn_auto_pushed_set(eoNaviBar, EINA_FALSE);
	ea_object_event_callback_add(eoNaviBar, EA_CALLBACK_BACK,
			ea_naviframe_back_cb, NULL);
	createBox();
	eoNaviBarItem = elm_naviframe_item_push(eoNaviBar,
		__("IDS_DM_HEADER_DOWNLOAD_MANAGER_ABB"),NULL, NULL, eoBox, NULL);
	if (!eoNaviBarItem)
		DM_LOGE("Null Check:eoNaviBarItem");
	createDeleteBtn();
	elm_naviframe_item_pop_cb_set(eoNaviBarItem, popCB, NULL);
	evas_object_show(eoNaviBar);
}

void DownloadView::deletePopupDeleteCB(void *data, Evas_Object *obj, void *event_info)
{
	DownloadView& view = DownloadView::getInstance();
	view.destroyCheckedItem();
	view.removePopup();
}

void DownloadView::showDeletePopup()
{
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;

	DM_LOGI("");

	/* If another popup is shown, delete it*/
	removePopup();
	eoPopup = elm_popup_add(eoWindow);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoPopup, EVAS_HINT_FILL,
		EVAS_HINT_FILL);
	elm_object_text_set(eoPopup, S_("IDS_COM_POP_DELETE_Q"));
	btn1 = elm_button_add(eoPopup);
	elm_object_text_set(btn1, S_("IDS_COM_SK_CANCEL"));
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", popupBackCB,
			NULL);
	btn2 = elm_button_add(eoPopup);
	elm_object_text_set(btn2, S_("IDS_COM_OPT_DELETE"));
	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", deletePopupDeleteCB,
			NULL);
	ea_object_event_callback_add(eoPopup, EA_CALLBACK_BACK, popupBackCB, NULL);
	evas_object_show(eoPopup);
}

void DownloadView::cancelToolBarBtnCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGI("");
	DownloadView& view = DownloadView::getInstance();
	view.hideGenlistEditMode();
}

void DownloadView::deleteToolBarBtnCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGI("");
	DownloadView& view = DownloadView::getInstance();
	view.showDeletePopup();
}

void DownloadView::createToolBar()
{
	DM_LOGD("");
	eoToolBar = elm_toolbar_add(eoNaviBar);
	if(!eoToolBar) {
		DM_LOGE("Null Check:eoDeleteBtn");
		return;
	}
	elm_object_style_set(eoToolBar, "default");
	elm_toolbar_shrink_mode_set(eoToolBar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_select_mode_set(eoToolBar, ELM_OBJECT_SELECT_MODE_NONE);
	elm_toolbar_transverse_expanded_set(eoToolBar, EINA_TRUE);
	elm_toolbar_item_append(eoToolBar, NULL,
			S_("IDS_COM_SK_CANCEL"), cancelToolBarBtnCB, NULL);
	eoToolBarItem = elm_toolbar_item_append(eoToolBar, NULL,
			S_("IDS_COM_OPT_DELETE"), deleteToolBarBtnCB, NULL);
	if (eoNaviBarItem)
		elm_object_item_part_content_set(eoNaviBarItem, "toolbar", eoToolBar);
	evas_object_show(eoToolBar);
}

void DownloadView::destroyToolBar()
{
	Elm_Object_Item *objItem = NULL;
	objItem = elm_toolbar_first_item_get(eoToolBar);
	if(objItem)
		elm_object_item_del(objItem);
	if(eoToolBarItem) {
		elm_object_item_del(eoToolBarItem);
		eoToolBarItem = NULL;
	}
	if(eoToolBar) {
		evas_object_del(eoToolBar);
		eoToolBar = NULL;
	}
	if (eoNaviBarItem)
		elm_object_item_part_content_unset(eoNaviBarItem, "toolbar");
}

void DownloadView::deleteBtnCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGI("");
	DownloadView& view = DownloadView::getInstance();
	view.showGenlistEditMode();
}

void DownloadView::createDeleteBtn()
{
	Evas_Object *icon = NULL;
	Evas_Object *button = elm_button_add(eoNaviBar);
	string iconPath;
	if (!button) {
		DM_LOGE("Null Check:eoNaviBarItem");
		return;
	}
	elm_object_style_set(button, "naviframe/title_icon");
	icon = elm_image_add(eoNaviBar);
	iconPath.assign(IMAGEDIR);
	iconPath.append("/00_icon_delete.png");
	elm_image_file_set(icon, iconPath.c_str(), NULL);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(button, "icon", icon);
	evas_object_smart_callback_add(button, "clicked", deleteBtnCB, NULL);
	elm_object_item_part_content_set(eoNaviBarItem, "title_right_btn",	button);
}

void DownloadView::destroyDeleteBtn()
{
	Evas_Object *button = NULL;
	if (eoNaviBarItem) {
		button = elm_object_item_part_content_get(eoNaviBarItem, "title_right_btn");
		if(button) {
			evas_object_del(button);
		}
		elm_object_item_part_content_unset(eoNaviBarItem, "title_right_btn");
	}
}

void DownloadView::createBox()
{
	DM_LOGD("");

	eoBox = elm_box_add(eoNaviBar);
	evas_object_size_hint_weight_set(eoBox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoBox, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(eoBox);
}


void DownloadView::createList()
{
	eoDldList = elm_genlist_add(eoLayout);
/* When using ELM_LIST_LIMIT, the window size is broken at the landscape mode */
	evas_object_size_hint_weight_set(eoDldList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoDldList, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_homogeneous_set(eoDldList, EINA_TRUE);
	elm_genlist_block_count_set(eoDldList,8);
	elm_box_pack_end(eoBox, eoDldList);
	evas_object_show(eoDldList);
}

void destroy_window_cb(void *data, Evas_Object *obj, void *event)
{
	DM_LOGI("");
	elm_exit();
}

void DownloadView::changedRegion()
{
	DateUtil &inst = DateUtil::getInstance();
	inst.updateLocale();
	elm_genlist_realized_items_update(eoDldList);
}

void DownloadView::attachViewItem(ViewItem *viewItem)
{
	DM_LOGD("");
	if (m_viewItemCount < 1) {
		hideEmptyView();
		createList();
		if (eoNaviBarItem)
			if (!elm_object_item_part_content_get(eoNaviBarItem,
					"title_right_btn"))
				createDeleteBtn();
	}
	if (viewItem) {
		addViewItemToGenlist(viewItem);
		m_viewItemCount++;
	}
}

void DownloadView::detachViewItem(ViewItem *viewItem)
{
	DM_LOGI("delete viewItem[%p]",viewItem);
	if (viewItem) {
		delete viewItem;
		m_viewItemCount--;
	}
	if (!isGenlistEditMode() &&
			(m_viewItemCount < 1))
		showEmptyView();
}

void DownloadView::update()
{
	Elm_Object_Item *it = NULL;
	DM_LOGD("");
	if (!eoDldList) {
		DM_LOGI("download list is NULL");
		return;
	}

	it = elm_genlist_first_item_get(eoDldList);
	while (it) {
		DM_LOGD("glItem[%p]",it);
		elm_genlist_item_update(it);
		it = elm_genlist_item_next_get(it);
	}
}

void DownloadView::update(ViewItem *viewItem)
{
	if (!viewItem)
		return;

	DM_LOGI("viewItem [%p]", viewItem);
	elm_genlist_item_update(viewItem->genlistItem());
}

void DownloadView::update(Elm_Object_Item *glItem)
{
	if (!glItem)
		return;

	DM_LOGI("glItem [%p]", glItem);
	elm_genlist_item_update(glItem);
}

void DownloadView::updateLang()
{
	DM_LOGD("");
	if (eoNaviBarItem) {
		if (isGenlistEditMode()) {
			Elm_Object_Item *objItem = NULL;
			if (eoToolBarItem)
				elm_object_item_text_set(eoToolBarItem, S_("IDS_COM_OPT_DELETE"));
			objItem = elm_toolbar_first_item_get(eoToolBar);
			if(objItem)
				elm_object_item_text_set(objItem, S_("IDS_COM_SK_CANCEL"));
			elm_object_item_text_set(eoNaviBarItem, __("IDS_DM_HEADER_SELECT_ITEMS"));
		} else {
			Evas_Object *buttonObj = NULL;
			buttonObj = elm_object_item_part_content_get(eoNaviBarItem, "title_right_btn");
			elm_object_text_set(buttonObj, S_("IDS_COM_OPT_DELETE"));
			elm_object_item_text_set(eoNaviBarItem,
					__("IDS_DM_HEADER_DOWNLOAD_MANAGER_ABB"));
		}
	}
	if (eoEmptyNoContent) {
		elm_object_part_text_set(eoEmptyNoContent, "elm.text",
				__("IDS_DM_BODY_NO_DOWNLOADS"));
//		elm_object_part_text_set(eoEmptyNoContent, "elm.text",
//					__("IDS_DM_BODY_AFTER_YOU_DOWNLOAD_ITEMS_THEY_WILL_BE_SHOWN_HERE"));
	}
	update();
}


void DownloadView::addViewItemToGenlist(ViewItem *viewItem)
{
	DM_LOGD("");
	createGenlistItem(viewItem);
}

void DownloadView::createGenlistItem(ViewItem *viewItem)
{
	Elm_Object_Item *glItem = NULL;
	if (!viewItem->isFinished()) {
		glItem = elm_genlist_item_prepend(
			eoDldList,
			viewItem->elmGenlistStyle(),
			static_cast<const void*>(viewItem),
			NULL,
			ELM_GENLIST_ITEM_NONE,
			genlistClickCB,
			static_cast<const void*>(viewItem));
	} else {
		/* Download History Item */
		glItem = elm_genlist_item_append(
			eoDldList,
			viewItem->elmGenlistStyle(),
			static_cast<const void*>(viewItem),
			NULL,
			ELM_GENLIST_ITEM_NONE,
			genlistClickCB,
			static_cast<const void*>(viewItem));
	}
	if (!glItem)
		DM_LOGE("Fail to add a genlist item");

	viewItem->setGenlistItem(glItem);
	/* Move scrollbar to top.
	 * When groupItem means today group in case of addtion of download link item
	**/
	if (!viewItem->isFinished())
		elm_genlist_item_show(glItem, ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

void DownloadView::showEmptyView()
{
	DM_LOGD("");
	if (!eoEmptyNoContent) {
		eoEmptyNoContent = elm_layout_add(eoLayout);
		elm_layout_theme_set(eoEmptyNoContent, "layout", "nocontents", "text");
		evas_object_size_hint_weight_set(eoEmptyNoContent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(eoEmptyNoContent, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_part_text_set(eoEmptyNoContent, "elm.text",
			__("IDS_DM_BODY_NO_DOWNLOADS"));
//		elm_object_part_text_set(eoEmptyNoContent, "elm.text",
//					__("IDS_DM_BODY_AFTER_YOU_DOWNLOAD_ITEMS_THEY_WILL_BE_SHOWN_HERE"));
		if (eoDldList) {
			elm_box_unpack(eoBox,eoDldList);
			/* Detection code */
			DM_LOGI("del::eoDldList[%p]",eoDldList);
			evas_object_del(eoDldList);
			eoDldList = NULL;
		}
		elm_box_pack_start(eoBox, eoEmptyNoContent);
	}
	evas_object_show(eoEmptyNoContent);
	destroyDeleteBtn();
}

void DownloadView::hideEmptyView()
{
	DM_LOGI("");
	if(eoEmptyNoContent) {
		elm_box_unpack(eoBox, eoEmptyNoContent);
		evas_object_del(eoEmptyNoContent);
		eoEmptyNoContent = NULL;
	}
}

bool DownloadView::isGenlistEditMode()
{
	return (bool)elm_genlist_decorate_mode_get(eoDldList);
}


void DownloadView::createSelectAll()
{
	eoAllCheckedBox = elm_check_add(eoNaviBar);
	elm_check_state_pointer_set(eoAllCheckedBox, &m_allChecked);
	evas_object_size_hint_aspect_set(eoAllCheckedBox,
			EVAS_ASPECT_CONTROL_BOTH, 1, 1);
	evas_object_smart_callback_add(eoAllCheckedBox, "changed",
		selectAllChangedCB, NULL);
	evas_object_propagate_events_set(eoAllCheckedBox, EINA_FALSE);
	evas_object_show(eoAllCheckedBox);
	elm_object_item_part_content_set(eoNaviBarItem, "title_right_btn",
			eoAllCheckedBox);
	m_allChecked = EINA_FALSE;
}

void DownloadView::destroyCheckedItem()
{
	Eina_List *list = NULL;
	Elm_Object_Item *it = NULL;
	ViewItem *item = NULL;
	int checkedCount = 0;
	queue <unsigned int> deleteQueue;

	it = elm_genlist_first_item_get(eoDldList);

	while (it) {
		item = (ViewItem *)elm_object_item_data_get(it);
		/* elm_genlist_item_select_mode_get is needed to check group item */
		if (item && item->checkedValue()) {
			list = eina_list_append(list, it);
		}
		it = elm_genlist_item_next_get(it);
	}

	if (!list) {
		DM_LOGI("There is no delete item");
		return;
	}

	checkedCount = eina_list_count(list);
	if (checkedCount < 1)
		return;
	DM_LOGD("checkedCount[%d]", checkedCount);

	for (int i = 0; i < checkedCount; i++)
	{
		it = (Elm_Object_Item *)eina_list_data_get(list);
		if (it)
			item = (ViewItem *)elm_object_item_data_get(it);
		else
			DM_LOGE("NULL Check:genlist item");
		list = eina_list_next(list);
		if (item) {
			deleteQueue.push(item->historyId());
			item->destroy();
		} else {
			DM_LOGE("NULL Check:viewItem");
		}
	}
	if (list)
		eina_list_free(list);

	DownloadHistoryDB::deleteMultipleItem(deleteQueue);
	hideGenlistEditMode();
}

void DownloadView::showGenlistEditMode()
{
	DM_LOGD("");

	/* Initialize notify info widget */
	elm_object_item_text_set(eoNaviBarItem, __("IDS_DM_HEADER_SELECT_ITEMS"));
	/* Change layoutbackground color to edit mode color */
	elm_object_style_set(eoBackground, "edit_mode");

	createToolBar();
	destroyDeleteBtn();

	/* Append 'Select All' layout */
	createSelectAll();
	/* Set reorder end edit mode */
	elm_genlist_reorder_mode_set(eoDldList, EINA_TRUE);
	elm_genlist_decorate_mode_set(eoDldList, EINA_TRUE);
	/* This means even if the ouside of checked box is selected,
	   it is same to click a check box. */
	elm_genlist_select_mode_set(eoDldList, ELM_OBJECT_SELECT_MODE_ALWAYS);

	Elm_Object_Item *it = NULL;
	ViewItem *viewItem = NULL;
	int finishedCount = 0;
	it = elm_genlist_first_item_get(eoDldList);
	while (it) {
		viewItem = (ViewItem *)elm_object_item_data_get(it);
		if (viewItem) {
			if (!(viewItem->isFinished()))
				elm_object_item_disabled_set(it, EINA_TRUE);
			else
				finishedCount++;
		}
		it = elm_genlist_item_next_get(it);
	}
	elm_object_item_disabled_set(eoToolBarItem, EINA_TRUE);
	if (finishedCount == 0)
		elm_object_disabled_set(eoAllCheckedBox, EINA_TRUE);
	else
		elm_object_disabled_set(eoAllCheckedBox, EINA_FALSE);
}

void DownloadView::hideGenlistEditMode()
{
	DM_LOGD("");

	elm_object_item_text_set(eoNaviBarItem, __("IDS_DM_HEADER_DOWNLOAD_MANAGER_ABB"));
	elm_object_style_set(eoBackground, "default");

	elm_object_item_part_content_unset(eoNaviBarItem, "title_right_btn");
	if(eoAllCheckedBox) {
		evas_object_del(eoAllCheckedBox);
		eoAllCheckedBox = NULL;
	}

	elm_genlist_reorder_mode_set(eoDldList, EINA_FALSE);
	elm_genlist_decorate_mode_set(eoDldList, EINA_FALSE);
	elm_genlist_select_mode_set(eoDldList, ELM_OBJECT_SELECT_MODE_DEFAULT);

	Elm_Object_Item *it = NULL;
	ViewItem *viewItem = NULL;
	it = elm_genlist_first_item_get(eoDldList);
	while (it) {
		viewItem = (ViewItem *)elm_object_item_data_get(it);
		if (viewItem) {
			if (elm_object_item_disabled_get(it))
				elm_object_item_disabled_set(it, EINA_FALSE);
			viewItem->setCheckedValue(EINA_FALSE);
			viewItem->setCheckedBtn(NULL);
		}
		it = elm_genlist_item_next_get(it);
	}

	m_allChecked = EINA_FALSE;

	destroyToolBar();
	createDeleteBtn();

	if (m_viewItemCount < 1) {
		showEmptyView();
	}
}

void DownloadView::handleChangedAllCheckedState()
{
	int checkedCount = 0;
	Elm_Object_Item *it = NULL;
	ViewItem *viewItem = NULL;
	it = elm_genlist_first_item_get(eoDldList);
	while (it) {
		viewItem = (ViewItem *)elm_object_item_data_get(it);
		if (viewItem) {
			if (viewItem->isFinished()) {
				viewItem->setCheckedValue(m_allChecked);
				viewItem->updateCheckedBtn();
				checkedCount++;
			}
		}
		it = elm_genlist_item_next_get(it);
	}

	if (m_allChecked && checkedCount > 0) {
		elm_object_item_disabled_set(eoToolBarItem, EINA_FALSE);
		showSelectedNotify(checkedCount);
	} else {
		elm_object_item_disabled_set(eoToolBarItem, EINA_TRUE);
		showSelectedNotify(0);
	}
}

void DownloadView::handleCheckedState()
{
	int checkedCount = 0;
	int deleteAbleTotalCount = 0;

	DM_LOGD("");

	Elm_Object_Item *it = NULL;
	ViewItem *viewItem = NULL;
	it = elm_genlist_first_item_get(eoDldList);
	while (it) {
		viewItem = (ViewItem *)elm_object_item_data_get(it);
		if (viewItem) {
			if (viewItem->checkedValue())
				checkedCount++;
			if (viewItem->isFinished())
				deleteAbleTotalCount++;
		}
		it = elm_genlist_item_next_get(it);
	}

	if (checkedCount == deleteAbleTotalCount)
		m_allChecked = EINA_TRUE;
	else
		m_allChecked = EINA_FALSE;
	elm_check_state_pointer_set(eoAllCheckedBox, &m_allChecked);

	if (checkedCount == 0)
		elm_object_item_disabled_set(eoToolBarItem, EINA_TRUE);
	else
		elm_object_item_disabled_set(eoToolBarItem, EINA_FALSE);
	if (elm_object_disabled_get(eoAllCheckedBox) == EINA_TRUE)
		elm_object_disabled_set(eoAllCheckedBox, EINA_FALSE);
	showSelectedNotify(checkedCount);
}

void DownloadView::showSelectedNotify(int selectedCount)
{
	char buff[MAX_BUF_LEN] = {0,};

	DM_LOGD("");
	if (selectedCount <= 0) {
		elm_object_item_text_set(eoNaviBarItem, __("IDS_DM_HEADER_SELECT_ITEMS"));
		return;
	}

	snprintf(buff, MAX_BUF_LEN,
			S_("IDS_COM_BODY_PD_SELECTED"), selectedCount);

	elm_object_item_text_set(eoNaviBarItem, buff);
}

void DownloadView::hideNotifyInfoCB(void *data, Evas *evas, Evas_Object *obj,
	void *event)
{
	Evas_Object *layout = (Evas_Object *)data;
	if (!data) {
		DM_LOGE("NULL Check:data");
		return;
	}
	edje_object_signal_emit(_EDJ(layout), "elm,layout,content,default", "layout");
}

void DownloadView::selectAllChangedCB(void *data, Evas_Object *obj,
	void *event_info)
{
	DownloadView &view = DownloadView::getInstance();
	DM_LOGD("");
	view.handleChangedAllCheckedState();
}

Eina_Bool DownloadView::popCB(void *data, Elm_Object_Item *it)
{
	DownloadView& view = DownloadView::getInstance();
	DM_LOGI("");
	if (view.isGenlistEditMode()) {
		view.hideGenlistEditMode();
		return EINA_FALSE;
	} else {
		view.hide();
		return EINA_FALSE;
	}
}

void DownloadView::genlistClickCB(void *data, Evas_Object *obj, void *event_info)
{
	ViewItem *item = reinterpret_cast<ViewItem *>(data);
	DM_LOGD("");
	if (!data) {
		DM_LOGE("NULL Check:data");
		return;
	}
	item->clickedGenlistItem();
}

void DownloadView::cancelClickCB(void *data, Evas_Object *obj, void *event_info)
{
	ViewItem *item = NULL;

	DM_LOGI("");

	if (!data) {
		DM_LOGE("NULL Check:data");
		return;
	}
	item = reinterpret_cast<ViewItem *>(data);
	item->requestCancel();

}

void DownloadView::showRetryPopup(ViewItem *viewItem, string msg)
{
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;

	DM_LOGI("");
	if (!viewItem)
		return;
	/* If another popup is shown, delete it*/
	removePopup();
	eoPopup = elm_popup_add(eoWindow);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoPopup, EVAS_HINT_FILL,
		EVAS_HINT_FILL);

	elm_object_text_set(eoPopup, msg.c_str());
	btn1 = elm_button_add(eoPopup);
	elm_object_text_set(btn1, S_("IDS_COM_SK_CANCEL"));
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", retryPopupCancelCB,
			viewItem);
	btn2 = elm_button_add(eoPopup);
	elm_object_text_set(btn2, S_("IDS_COM_SK_RETRY"));

	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", retryPopupRetryCB,
			viewItem);
	ea_object_event_callback_add(eoPopup, EA_CALLBACK_BACK, popupBackCB, NULL);
	evas_object_show(eoPopup);
}

void DownloadView::showMemoryFullPopup()
{
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;

	DM_LOGI("");

	if (m_silentMode) {
		DM_LOGI("silent mode");
		evas_object_show(eoWindow);
		elm_win_activate(eoWindow);
	}

	/* If another popup is shown, delete it*/
	removePopup();
	eoPopup = elm_popup_add(eoWindow);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoPopup, EVAS_HINT_FILL,
		EVAS_HINT_FILL);

	elm_object_text_set(eoPopup, ERROR_POPUP_LOW_MEM);
	btn1 = elm_button_add(eoPopup);
	elm_object_text_set(btn1, S_("IDS_COM_BODY_MY_FILES"));
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", memoryFullPopupMyFilesCB,
			NULL);
	btn2 = elm_button_add(eoPopup);
	elm_object_text_set(btn2, S_("IDS_COM_POP_CLOSE"));

	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", memoryFullPopupCancelCB,
			NULL);
	ea_object_event_callback_add(eoPopup, EA_CALLBACK_BACK, popupBackCB, NULL);
	evas_object_show(eoPopup);
}

void DownloadView::showErrPopup(string &desc)
{
	if (m_silentMode) {
		DM_LOGI("silent mode");
		evas_object_show(eoWindow);
		elm_win_activate(eoWindow);
	}

	removePopup();

	eoPopup = elm_popup_add(eoWindow);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(eoPopup, desc.c_str());
	elm_popup_timeout_set(eoPopup, 2);
	evas_object_smart_callback_add(eoPopup, "response", errPopupResponseCB, NULL);
	ea_object_event_callback_add(eoPopup, EA_CALLBACK_BACK, popupBackCB, NULL);
	evas_object_show(eoPopup);
}

void DownloadView::errPopupResponseCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGI("");
	DownloadView& view = DownloadView::getInstance();
	view.removePopup();
}


void DownloadView::memoryFullPopupCancelCB(void *data, Evas_Object *obj, void *event_info)
{
	DownloadView& view = DownloadView::getInstance();
	view.removePopup();
}

void DownloadView::memoryFullPopupMyFilesCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGI("");
	DownloadView& view = DownloadView::getInstance();
	FileUtility fileObj;
	fileObj.openMyFilesApp();
	view.removePopup();
}

void DownloadView::retryPopupCancelCB(void *data, Evas_Object *obj, void *event_info)
{
	ViewItem *viewItem = (ViewItem *)data;
	DownloadView& view = DownloadView::getInstance();
	if (viewItem)
		viewItem->clickedCanceledRetryButton();
	else
		DM_LOGE("No viewItem");
	view.removePopup();
}

void DownloadView::retryPopupRetryCB(void *data, Evas_Object *obj, void *event_info)
{
	ViewItem *viewItem = (ViewItem *)data;
	DownloadView& view = DownloadView::getInstance();

	DM_LOGI("");
	if (viewItem)
		viewItem->clickedRetryButton();
	else
		DM_LOGE("No viewItem");
	view.removePopup();
}

void DownloadView::removePopup()
{
#ifdef _ENABLE_OMA_DOWNLOAD
	if (eoPopup && prevOmaViewItem) {
		DM_LOGI("Destroy oma popup and cancel oma download");
		prevOmaViewItem->responseUserConfirm(false);
		prevOmaViewItem = NULL;
	} else {
		prevOmaViewItem = NULL;
	}
#endif
	if(eoPopup) {
		DM_LOGD("Remove Popup");
		evas_object_del(eoPopup);
		eoPopup = NULL;
	}
}

void DownloadView::cleanGenlistData()
{
	DM_LOGI("");
	elm_genlist_clear(eoDldList);
}

void DownloadView::moveRetryItem(ViewItem *viewItem)
{

}

#ifdef _ENABLE_OMA_DOWNLOAD
void DownloadView::removeOnlyPopupObj()
{
	DM_LOGD("");
	prevOmaViewItem = NULL;
	if(eoPopup) {
		evas_object_del(eoPopup);
		eoPopup = NULL;
	}
}

void DownloadView::showOMAPopup(string msg, ViewItem *viewItem)
{
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;

	DM_LOGI("");

	if (!viewItem)
		return;
	if (msg.empty()) {
		viewItem->responseUserConfirm(false);
		return;
	}

	if (m_silentMode) {
		DM_LOGI("silent mode");
		evas_object_show(eoWindow);
		elm_win_activate(eoWindow);
	}

	/* If another popup is shown, delete it*/
	removePopup();

	eoPopup = elm_popup_add(eoWindow);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(eoPopup, msg.c_str());
	elm_object_part_text_set(eoPopup, "title,text", S_("IDS_COM_SK_DOWNLOAD"));
	btn1 = elm_button_add(eoPopup);
	elm_object_text_set(btn1, S_("IDS_COM_SK_CANCEL"));
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", omaPopupResponseCancelCB,
		viewItem);
	btn2 = elm_button_add(eoPopup);
	elm_object_text_set(btn2, S_("IDS_COM_SK_OK"));
	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", omaPopupResponseOKCB,
		viewItem);
	ea_object_event_callback_add(eoPopup, EA_CALLBACK_BACK, popupBackCB, NULL);
	evas_object_show(eoPopup);
	prevOmaViewItem = viewItem;
}

void DownloadView::omaPopupResponseOKCB(void *data, Evas_Object *obj,
		void *event_info)
{
	ViewItem *viewItem = (ViewItem *)data;
	DownloadView& view = DownloadView::getInstance();

	DM_LOGI("");
	if (viewItem)
		viewItem->responseUserConfirm(true);
	else
		DM_LOGE("No viewItem");
	view.removeOnlyPopupObj();
}

void DownloadView::omaPopupResponseCancelCB(void *data, Evas_Object *obj,
		void *event_info)
{
	ViewItem *viewItem = (ViewItem *)data;
	DownloadView& view = DownloadView::getInstance();

	DM_LOGI("");
	if (viewItem)
		viewItem->responseUserConfirm(false);
	else
		DM_LOGE("No viewItem");

	view.removeOnlyPopupObj();
}
#endif

void DownloadView::popupBackCB(void *data, Evas_Object *obj, void *event_info)
{
	DownloadView& view = DownloadView::getInstance();
	view.removePopup();
}
