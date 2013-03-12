/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
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
#include "download-manager-view.h"
#include "download-manager-history-db.h"
#include "download-manager-downloadItem.h"
#include "status.h"

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
	, eoBackBtn(NULL)
	, eoControlBar(NULL)
	, eoDeleteBtn(NULL)
	, eoBoxLayout(NULL)
	, eoBox(NULL)
	, eoDldList(NULL)
	, eoPopup(NULL)
	, eoSelectAllLayout(NULL)
	, eoAllCheckedBox(NULL)
	, eoNotifyInfoLayout(NULL)
	, m_allChecked(EINA_FALSE)
#ifdef _ENABLE_OMA_DOWNLOAD
	, prevOmaViewItem(NULL)
#endif
	, m_viewItemCount(0)
	, m_silentMode(false)
{
// FIXME Later : init private members
	DownloadEngine &engine = DownloadEngine::getInstance();
	engine.initEngine();
	DateUtil &inst = DateUtil::getInstance();
	inst.updateLocale();
}

DownloadView::~DownloadView()
{
	DP_LOGD_FUNC();
//	DownloadEngine &engine = DownloadEngine::getInstance();
//	engine.deinitEngine();
}

Evas_Object *DownloadView::create(void)
{
	Evas_Object *window = NULL;
	Evas_Object *conformant = NULL;
	window = createWindow(PACKAGE);
	if (!window)
		return NULL;

	createBackground(window);
	conformant = createConformant(window);
	createLayout(conformant);
	setIndicator(window);
	createView();

	return window;
}

void DownloadView::destroy()
{
	DownloadEngine &engine = DownloadEngine::getInstance();
	engine.deinitEngine();
}

void DownloadView::show()
{
	DP_LOGD_FUNC();

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
	DP_LOGD_FUNC();
	removePopup();
	destroyNotifyInfo();
	if (isGenlistEditMode()) {
		hideGenlistEditMode();
	}
	evas_object_hide(eoWindow);
	elm_win_lower(eoWindow);
}

void DownloadView::pause()
{
	DP_LOGD_FUNC();
	removePopup();
	destroyNotifyInfo();
	if (isGenlistEditMode()) {
		hideGenlistEditMode();
	}
}

void DownloadView::activateWindow()
{
	if (!eoWindow)
		create();

	show();
}

#ifdef _ENABLE_ROTATE
void DownloadView::rotateWindow(int angle)
{
	if (angle >= 0)
		elm_win_rotation_with_resize_set(eoWindow, angle);
}
#endif

void DownloadView::showViewItem(int id, const char *title)
{
	DP_LOGD_FUNC();
}

/* This is called by AUL view mode */
void DownloadView::playContent(int id, const char *title)
{
	DP_LOGD_FUNC();
}

void DownloadView::setIndicator(Evas_Object *window)
{
	elm_win_indicator_mode_set(window, ELM_WIN_INDICATOR_SHOW);
}

Evas_Object *DownloadView::createWindow(const char *windowName)
{
	eoWindow = elm_win_add(NULL, windowName, ELM_WIN_BASIC);
	if (eoWindow) {
		elm_win_title_set(eoWindow, __("IDS_BR_HEADER_DOWNLOAD_MANAGER"));
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
		DP_LOGE("Fail to create bg object");
	}
	return eoBackground;
}

Evas_Object *DownloadView::createConformant(Evas_Object *window)
{
	DP_LOGV_FUNC();
	if (!window)
		return NULL;
	eoConform = elm_conformant_add(window);
	if (eoConform) {
		evas_object_size_hint_weight_set(eoConform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(window, eoConform);
	}
	if (!eoConform) {
		DP_LOGE("Fail to create conformant object");
		return NULL;
	}
	evas_object_show(eoConform);
	return eoConform;
}

Evas_Object *DownloadView::createLayout(Evas_Object *parent)
{
	if (!parent) {
		DP_LOGE("Invalid Paramter");
		return NULL;
	}

	eoLayout = elm_layout_add(parent);
	if (eoLayout) {
		if (!elm_layout_theme_set(eoLayout, "layout", "application", "default" ))
			DP_LOGE("Fail to set elm_layout_theme_set");

		evas_object_size_hint_weight_set(eoLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_content_set(parent, eoLayout);
		evas_object_show(eoLayout);
	} else {
		DP_LOGE("Fail to create layout");
	}
	return eoLayout;
}

void DownloadView::createView()
{
	DP_LOGD_FUNC();
	createNaviBar();
	createList();
	if (m_viewItemCount < 1)
		showEmptyView();
}

void DownloadView::createNaviBar()
{
	DP_LOGV_FUNC();
	eoNaviBar = elm_naviframe_add(eoLayout);
	elm_object_part_content_set(eoLayout, "elm.swallow.content", eoNaviBar);
	createBackBtn();
	createBox();
	eoNaviBarItem = elm_naviframe_item_push(eoNaviBar,
		__("IDS_BR_HEADER_DOWNLOAD_MANAGER"),eoBackBtn, NULL, eoBoxLayout, NULL);
	createControlBar();
	evas_object_show(eoNaviBar);
}

void DownloadView::createBackBtn()
{
	DP_LOGV_FUNC();
	eoBackBtn = elm_button_add(eoNaviBar);
	elm_object_style_set(eoBackBtn, "naviframe/end_btn/default");
	evas_object_smart_callback_add(eoBackBtn, "clicked", backBtnCB,NULL);
	evas_object_show(eoBackBtn);
}

void DownloadView::createControlBar()
{
	DP_LOGV_FUNC();

	eoDeleteBtn = elm_button_add(eoNaviBar);
	elm_object_style_set(eoDeleteBtn, "naviframe/toolbar/default");
	elm_object_text_set(eoDeleteBtn, S_("IDS_COM_OPT_DELETE"));
	evas_object_smart_callback_add(eoDeleteBtn, "clicked", cbDeleteBtnCB, eoNaviBar);
	elm_object_item_part_content_set(eoNaviBarItem, "toolbar_button1",
		eoDeleteBtn);
	elm_object_disabled_set(eoDeleteBtn, EINA_TRUE);
	evas_object_show(eoControlBar);
}

void DownloadView::createBox()
{
	DP_LOGV_FUNC();
	eoBoxLayout = elm_layout_add(eoNaviBar);
	elm_layout_file_set(eoBoxLayout, EDJE_PATH, "download/selectioninfo");

	evas_object_size_hint_weight_set(eoBoxLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoBoxLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	eoBox = elm_box_add(eoBoxLayout);
	elm_object_part_content_set(eoBoxLayout, "gen.swallow.contents", eoBox);

	evas_object_show(eoBox);
	evas_object_show(eoBoxLayout);
}

void DownloadView::createList()
{
	//DP_LOGD_FUNC();
	eoDldList = elm_genlist_add(eoBoxLayout);
	DP_LOGD("create::eoDldList[%p]",eoDldList);
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
	DP_LOGD_FUNC();
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
	DP_LOGV_FUNC();
	if (m_viewItemCount < 1) {
		hideEmptyView();
		createList();
	}
	if (viewItem) {
		addViewItemToGenlist(viewItem);
		m_viewItemCount++;
	}
}

void DownloadView::detachViewItem(ViewItem *viewItem)
{
	DP_LOGD("delete viewItem[%p]",viewItem);
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
	DP_LOGD_FUNC();
	if (!eoDldList) {
		DP_LOGE("download list is NULL");
		return;
	}

	it = elm_genlist_first_item_get(eoDldList);
	while (it) {
		DP_LOGV("glItem[%p]",it);
		elm_genlist_item_update(it);
		it = elm_genlist_item_next_get(it);
	}
}

void DownloadView::update(ViewItem *viewItem)
{
	if (!viewItem)
		return;

	DP_LOGD("DownloadView::update viewItem [%p]", viewItem);
	elm_genlist_item_update(viewItem->genlistItem());
}

void DownloadView::update(Elm_Object_Item *glItem)
{
	if (!glItem)
		return;

	DP_LOGD("DownloadView::update glItem [%p]", glItem);
	elm_genlist_item_update(glItem);
}

void DownloadView::updateLang()
{
	DP_LOGD("DownloadView::updateLang");
	if (eoNaviBarItem) {
		if (isGenlistEditMode()) {
			elm_object_item_text_set(eoNaviBarItem, S_("IDS_COM_OPT_DELETE"));
			if (eoDeleteBtn)
				elm_object_text_set(eoDeleteBtn, S_("IDS_COM_OPT_DELETE"));
		} else {
			elm_object_item_text_set(eoNaviBarItem, __("IDS_BR_HEADER_DOWNLOAD_MANAGER"));
			if (eoDeleteBtn)
				elm_object_text_set(eoDeleteBtn, S_("IDS_COM_OPT_DELETE"));
		}
	}
	if (eoEmptyNoContent)
		elm_object_part_text_set(eoEmptyNoContent, "elm.text",__("IDS_DL_BODY_NO_DOWNLOADS"));
	update();
}


void DownloadView::addViewItemToGenlist(ViewItem *viewItem)
{
	DP_LOGV_FUNC();
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
		DP_LOGE("Fail to add a genlist item");

	DP_LOGD("genlist item[%p] viewItem[%p]", glItem, viewItem);
	viewItem->setGenlistItem(glItem);
	/* Move scrollbar to top.
	 * When groupItem means today group in case of addtion of download link item
	**/
	if (!viewItem->isFinished())
		elm_genlist_item_show(glItem, ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

void DownloadView::showEmptyView()
{
	DP_LOGV_FUNC();
	if (!eoEmptyNoContent) {
		eoEmptyNoContent = elm_layout_add(eoLayout);
		elm_layout_theme_set(eoEmptyNoContent, "layout", "nocontents", "text");
		evas_object_size_hint_weight_set(eoEmptyNoContent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(eoEmptyNoContent, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_part_text_set(eoEmptyNoContent, "elm.text",
			__("IDS_DL_BODY_NO_DOWNLOADS"));
		evas_object_size_hint_weight_set (eoEmptyNoContent,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		if (eoDldList) {
			elm_box_unpack(eoBox,eoDldList);
			/* Detection code */
			DP_LOGD("del::eoDldList[%p]",eoDldList);
			evas_object_del(eoDldList);
			eoDldList = NULL;
		}
		elm_box_pack_start(eoBox, eoEmptyNoContent);
	}
	evas_object_show(eoEmptyNoContent);
	elm_object_disabled_set(eoDeleteBtn, EINA_TRUE);
}

void DownloadView::hideEmptyView()
{
	DP_LOGD_FUNC();
	if(eoEmptyNoContent) {
		elm_box_unpack(eoBox, eoEmptyNoContent);
		evas_object_del(eoEmptyNoContent);
		eoEmptyNoContent = NULL;
	}
	elm_object_disabled_set(eoDeleteBtn, EINA_FALSE);
}

bool DownloadView::isGenlistEditMode()
{
	return (bool)elm_genlist_decorate_mode_get(eoDldList);
}

void DownloadView::createSelectAllLayout()
{
	eoSelectAllLayout = elm_layout_add(eoBox);
	elm_layout_theme_set(eoSelectAllLayout, "genlist", "item",
		"select_all/default");
	evas_object_size_hint_weight_set(eoSelectAllLayout, EVAS_HINT_EXPAND,
		EVAS_HINT_FILL);
	evas_object_size_hint_align_set(eoSelectAllLayout, EVAS_HINT_FILL,
		EVAS_HINT_FILL);
	evas_object_event_callback_add(eoSelectAllLayout, EVAS_CALLBACK_MOUSE_DOWN,
		selectAllClickedCB, NULL);
	eoAllCheckedBox = elm_check_add(eoSelectAllLayout);
	elm_check_state_pointer_set(eoAllCheckedBox, &m_allChecked);
	evas_object_smart_callback_add(eoAllCheckedBox, "changed",
		selectAllChangedCB, NULL);
	evas_object_propagate_events_set(eoAllCheckedBox, EINA_FALSE);
	elm_object_part_content_set(eoSelectAllLayout, "elm.icon", eoAllCheckedBox);
	elm_object_text_set(eoSelectAllLayout, S_("IDS_COM_BODY_SELECT_ALL"));
	elm_box_pack_start(eoBox, eoSelectAllLayout);
	evas_object_show(eoSelectAllLayout);
	m_allChecked = EINA_FALSE;
}

void DownloadView::changeAllCheckedValue()
{
	m_allChecked = !m_allChecked;
	elm_check_state_pointer_set(eoAllCheckedBox, &m_allChecked);
	handleChangedAllCheckedState();
}

void DownloadView::destroyCheckedItem()
{
	Eina_List *list = NULL;
	Elm_Object_Item *it = NULL;
	ViewItem *item = NULL;
	int checkedCount = 0;
	queue <unsigned int> deleteQueue;

	DP_LOGD_FUNC();

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
		DP_LOGD("There is no delete item");
		return;
	}

	checkedCount = eina_list_count(list);
	if (checkedCount < 1)
		return;
	DP_LOGD("checkedCount[%d]", checkedCount);

	for (int i = 0; i < checkedCount; i++)
	{
		it = (Elm_Object_Item *)eina_list_data_get(list);
		if (it)
			item = (ViewItem *)elm_object_item_data_get(it);
		else
			DP_LOGE("genlist item is null");
		list = eina_list_next(list);
		if (item) {
			deleteQueue.push(item->historyId());
			item->destroy();
		} else {
			DP_LOGE("viewItem is null");
		}
	}
	if (list)
		eina_list_free(list);

	DownloadHistoryDB::deleteMultipleItem(deleteQueue);
	showNotifyInfo(DOWNLOAD_NOTIFY_DELETED, checkedCount);
	hideGenlistEditMode();
}

void DownloadView::showGenlistEditMode()
{
	DP_LOGD_FUNC();
	/* Initialize notify info widget */
	destroyNotifyInfo();
	elm_object_item_text_set(eoNaviBarItem, S_("IDS_COM_OPT_DELETE"));
	/* Change layoutbackground color to edit mode color */
	elm_object_style_set(eoBackground, "edit_mode");

	/* Append 'Select All' layout */
	createSelectAllLayout();
	/* Set reorder end edit mode */
	elm_genlist_reorder_mode_set(eoDldList, EINA_TRUE);
	elm_genlist_decorate_mode_set(eoDldList, EINA_TRUE);
	/* This means even if the ouside of checked box is selected,
	   it is same to click a check box. */
	elm_genlist_select_mode_set(eoDldList, ELM_OBJECT_SELECT_MODE_ALWAYS);

	Elm_Object_Item *it = NULL;
	ViewItem *viewItem = NULL;
	it = elm_genlist_first_item_get(eoDldList);
	while (it) {
		viewItem = (ViewItem *)elm_object_item_data_get(it);
		if (viewItem && !(viewItem->isFinished()))
			elm_object_item_disabled_set(it, EINA_TRUE);
		it = elm_genlist_item_next_get(it);
	}
	elm_object_disabled_set(eoDeleteBtn, EINA_TRUE);
}

void DownloadView::hideGenlistEditMode()
{
	DP_LOGD_FUNC();

	elm_object_item_text_set(eoNaviBarItem, __("IDS_BR_HEADER_DOWNLOAD_MANAGER"));
	elm_object_style_set(eoBackground, "default");

	elm_box_unpack(eoBox, eoSelectAllLayout);

	destroyEvasObj(eoAllCheckedBox);
	destroyEvasObj(eoSelectAllLayout);

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

	if (m_viewItemCount < 1) {
		elm_object_disabled_set(eoDeleteBtn, EINA_TRUE);
		showEmptyView();
	} else
		elm_object_disabled_set(eoDeleteBtn, EINA_FALSE);
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
		elm_object_disabled_set(eoDeleteBtn, EINA_FALSE);
		showNotifyInfo(DOWNLOAD_NOTIFY_SELECTED, checkedCount);
	} else {
		elm_object_disabled_set(eoDeleteBtn, EINA_TRUE);
		showNotifyInfo(DOWNLOAD_NOTIFY_SELECTED, 0);
	}
}

void DownloadView::handleCheckedState()
{
	int checkedCount = 0;
	int deleteAbleTotalCount = 0;

	DP_LOGD_FUNC();

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

	if (checkedCount == 0) {
		elm_object_disabled_set(eoDeleteBtn, EINA_TRUE);
		destroyNotifyInfo();
	} else
		elm_object_disabled_set(eoDeleteBtn, EINA_FALSE);
	showNotifyInfo(DOWNLOAD_NOTIFY_SELECTED, checkedCount);
}

void DownloadView::createNotifyInfo()
{
	DP_LOGD_FUNC();
	eoNotifyInfoLayout = elm_layout_add(eoBoxLayout);
	elm_object_part_content_set(eoBoxLayout, "sel.swallow.contents",
			eoNotifyInfoLayout);
	evas_object_size_hint_weight_set(eoNotifyInfoLayout,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoNotifyInfoLayout,
			EVAS_HINT_FILL, EVAS_HINT_FILL);
}

void DownloadView::showNotifyInfo(int type, int selectedCount)
{
	string buf;
	DP_LOGD_FUNC();

	if (selectedCount == 0) {
		destroyNotifyInfo();
		return;
	}

	if (!eoNotifyInfoLayout)
		createNotifyInfo();

	elm_layout_theme_set(eoNotifyInfoLayout, "standard", "selectioninfo",
			"center_text");
	buf.append(" ");
	if (type == DOWNLOAD_NOTIFY_SELECTED) {
		stringstream countStr;
		countStr << selectedCount;
		buf = S_("IDS_COM_BODY_SELECTED");
		buf.append(" (");
		buf.append(countStr.str());
		buf.append(")");
	} else if (type == DOWNLOAD_NOTIFY_DELETED) {
		status_message_post(S_("IDS_COM_POP_DELETED"));
		destroyNotifyInfo();
		return;
	}
	elm_object_part_text_set(eoNotifyInfoLayout, "elm.text", buf.c_str());
	elm_object_signal_emit(eoBoxLayout, "show,selection,info", "elm");
	evas_object_show(eoNotifyInfoLayout);
}

Eina_Bool DownloadView::deletedNotifyTimerCB(void *data)
{
	DownloadView& view = DownloadView::getInstance();
	view.destroyNotifyInfo();
	return ECORE_CALLBACK_RENEW;
}

void DownloadView::destroyNotifyInfo()
{
	DP_LOGD_FUNC();
	destroyEvasObj(eoNotifyInfoLayout);
	eoNotifyInfoLayout = NULL;
}

/* Static callback function */
void DownloadView::showNotifyInfoCB(void *data, Evas *evas, Evas_Object *obj,
	void *event)
{
	Evas_Object *layout = (Evas_Object *)data;
	if (!data) {
		DP_LOGE("data is NULL");
		return;
	}
	edje_object_signal_emit(_EDJ(layout), "elm,layout,content,bottom_padding",
		"layout");
}

void DownloadView::hideNotifyInfoCB(void *data, Evas *evas, Evas_Object *obj,
	void *event)
{
	Evas_Object *layout = (Evas_Object *)data;
	if (!data) {
		DP_LOGE("data is NULL");
		return;
	}
	edje_object_signal_emit(_EDJ(layout), "elm,layout,content,default", "layout");
}

void DownloadView::selectAllClickedCB(void *data, Evas *evas, Evas_Object *obj,
	void *event_info)
{
	DownloadView &view = DownloadView::getInstance();
	DP_LOGD_FUNC();
	view.changeAllCheckedValue();
}

void DownloadView::selectAllChangedCB(void *data, Evas_Object *obj,
	void *event_info)
{
	DownloadView &view = DownloadView::getInstance();
	DP_LOGD_FUNC();
	view.handleChangedAllCheckedState();
}

void DownloadView::backBtnCB(void *data, Evas_Object *obj, void *event_info)
{
	DownloadView& view = DownloadView::getInstance();
	if (view.isGenlistEditMode()) {
		view.destroyNotifyInfo();
		view.hideGenlistEditMode();
	} else {
		view.hide();
	}
}

void DownloadView::cbDeleteBtnCB(void *data, Evas_Object *obj, void *event_info)
{

	DownloadView& view = DownloadView::getInstance();
	if (!view.isGenlistEditMode())
		view.showGenlistEditMode();
	else
		view.destroyCheckedItem();
}

void DownloadView::cbItemCancelCB(void *data, Evas_Object *obj, void *event_info)
{
	DownloadView& view = DownloadView::getInstance();
	view.destroyNotifyInfo();
	view.hideGenlistEditMode();
}

void DownloadView::genlistClickCB(void *data, Evas_Object *obj, void *event_info)
{
	ViewItem *item = reinterpret_cast<ViewItem *>(data);
	DP_LOGD_FUNC();
	if (!data) {
		DP_LOGE("data is NULL");
		return;
	}
	item->clickedGenlistItem();
}

void DownloadView::cancelClickCB(void *data, Evas_Object *obj, void *event_info)
{
	ViewItem *item = NULL;

	DP_LOGD_FUNC();

	if (!data) {
		DP_LOGE("data is NULL");
		return;
	}
	item = reinterpret_cast<ViewItem *>(data);
	item->requestCancel();

}

void DownloadView::showRetryPopup(ViewItem *viewItem, string msg)
{
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;

	DP_LOGD_FUNC();
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
	elm_object_text_set(btn1, S_("IDS_COM_SK_RETRY"));
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", retryPopupRetryCB,
			viewItem);
	btn2 = elm_button_add(eoPopup);
	elm_object_text_set(btn2, S_("IDS_COM_SK_CANCEL"));

	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", retryPopupCancelCB,
			NULL);
	evas_object_show(eoPopup);
}

void DownloadView::showErrPopup(string &desc)
{
	removePopup();

	eoPopup = elm_popup_add(eoWindow);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(eoPopup, desc.c_str());
	elm_popup_timeout_set(eoPopup, 2);
	evas_object_smart_callback_add(eoPopup, "response", errPopupResponseCB, NULL);
	evas_object_show(eoPopup);
}

void DownloadView::errPopupResponseCB(void *data, Evas_Object *obj, void *event_info)
{
	DP_LOGD_FUNC();
	DownloadView& view = DownloadView::getInstance();
	view.removePopup();
}

void DownloadView::retryPopupCancelCB(void *data, Evas_Object *obj, void *event_info)
{
	DownloadView& view = DownloadView::getInstance();
	view.removePopup();
}

void DownloadView::retryPopupRetryCB(void *data, Evas_Object *obj, void *event_info)
{
	ViewItem *viewItem = (ViewItem *)data;
	DownloadView& view = DownloadView::getInstance();

	DP_LOGD_FUNC();
	if (viewItem)
		viewItem->clickedRetryButton();
	else
		DP_LOGE("No viewItem");
	view.removePopup();
}

void DownloadView::removePopup()
{
	DP_LOGD_FUNC();
#ifdef _ENABLE_OMA_DOWNLOAD
	if (eoPopup && prevOmaViewItem) {
		DP_LOGD("Destroy oma popup and cancel oma download");
		prevOmaViewItem->responseUserConfirm(false);
		prevOmaViewItem = NULL;
	} else {
		prevOmaViewItem = NULL;
	}
#endif
	destroyEvasObj(eoPopup);
}

void DownloadView::cleanGenlistData()
{
	DP_LOGD_FUNC();
	elm_genlist_clear(eoDldList);
}

void DownloadView::moveRetryItem(ViewItem *viewItem)
{

}

#ifdef _ENABLE_OMA_DOWNLOAD
void DownloadView::removeOnlyPopupObj()
{
	DP_LOGD_FUNC();
	prevOmaViewItem = NULL;
	destroyEvasObj(eoPopup);
}

void DownloadView::showOMAPopup(string msg, ViewItem *viewItem)
{
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	DP_LOGD_FUNC();
	if (!viewItem)
		return;
	if (msg.empty()) {
		viewItem->responseUserConfirm(false);
		return;
	}
	/* If another popup is shown, delete it*/
	removePopup();

	eoPopup = elm_popup_add(eoWindow);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(eoPopup, msg.c_str());
	elm_object_part_text_set(eoPopup, "title,text", __("IDS_BR_POP_DOWNLOAD_Q"));
	btn1 = elm_button_add(eoPopup);
	elm_object_text_set(btn1, S_("IDS_COM_SK_OK"));
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", omaPopupResponseOKCB,
		viewItem);
	btn2 = elm_button_add(eoPopup);
	elm_object_text_set(btn2, S_("IDS_COM_SK_CANCEL"));
	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", omaPopupResponseCancelCB,
		viewItem);

	evas_object_show(eoPopup);
	prevOmaViewItem = viewItem;
}

void DownloadView::omaPopupResponseOKCB(void *data, Evas_Object *obj,
		void *event_info)
{
	ViewItem *viewItem = (ViewItem *)data;
	DownloadView& view = DownloadView::getInstance();

	DP_LOGD_FUNC();
	if (viewItem)
		viewItem->responseUserConfirm(true);
	else
		DP_LOGE("No viewItem");
	view.removeOnlyPopupObj();
}

void DownloadView::omaPopupResponseCancelCB(void *data, Evas_Object *obj,
		void *event_info)
{
	ViewItem *viewItem = (ViewItem *)data;
	DownloadView& view = DownloadView::getInstance();

	DP_LOGD_FUNC();
	if (viewItem)
		viewItem->responseUserConfirm(false);
	else
		DP_LOGE("No viewItem");

	view.removeOnlyPopupObj();
}

#endif

