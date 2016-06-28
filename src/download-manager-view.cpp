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
//#include "utilX.h"
#include <efl_extension.h>

#include "download-manager-view.h"
#include "download-manager-history-db.h"
#include "download-manager-dateTime.h"

static void destroy_window_cb(void *data, Evas_Object *obj, void *event);

enum {
	DOWNLOAD_NOTIFY_SELECTED,
	DOWNLOAD_NOTIFY_DELETED
};

DownloadView::DownloadView(void)
	: eoWindow(NULL)
#ifndef _TIZEN_2_3_UX
	, eoIndicatorBackground(NULL)
#endif
	, eoBackground(NULL)
	, eoConform(NULL)
	, eoLayout(NULL)
	, eoEmptyNoContent(NULL)
	, eoNaviBar(NULL)
	, eoNaviBarItem(NULL)
#ifndef _TIZEN_2_3_UX
	, eoToolBar(NULL)
	, eoToolBarItem(NULL)
#endif
	, eoBox(NULL)
	, eoDldList(NULL)
	, eoPopup(NULL)
#ifdef _TIZEN_2_3_UX
	, m_theme(NULL)
	, eoMoreMenu(NULL)
	, eoSelectAllLayout(NULL)
	, m_isEditMode(false)
#endif
	, m_allChecked(EINA_FALSE)
#ifdef _ENABLE_OMA_DOWNLOAD
	, prevOmaViewItem(NULL)
#endif
	, m_pauseTimer(NULL)
	, m_viewItemCount(0)
	, m_selectedItemsCount(0)
	, m_silentMode(false)
	, m_activatedLockScreen(false)
{
	m_groupTitle[VIEWITEM_GROUP::TODAY].subItemsCount = 0;
	m_groupTitle[VIEWITEM_GROUP::TODAY].viewItem = NULL;
	m_groupTitle[VIEWITEM_GROUP::YESTERDAY].subItemsCount = 0;
	m_groupTitle[VIEWITEM_GROUP::YESTERDAY].viewItem = NULL;
	m_groupTitle[VIEWITEM_GROUP::OLDER].subItemsCount = 0;
	m_groupTitle[VIEWITEM_GROUP::OLDER].viewItem = NULL;
	m_viewLastRefreshTime = time(NULL);
// FIXME Later : init private members
	DownloadEngine &engine = DownloadEngine::getInstance();
	engine.initEngine();
	DateUtil &inst = DateUtil::getInstance();
	inst.updateLocale();
#ifdef _TIZEN_2_3_UX
	m_theme = elm_theme_new();
	elm_theme_ref_set(m_theme, NULL);
	elm_theme_extension_add(m_theme, EDJEDIR"/download-manager-view.edj");
#endif
}

DownloadView::~DownloadView()
{
	DM_LOGD("");
#ifdef _TIZEN_2_3_UX
	elm_theme_extension_del(m_theme, EDJEDIR"/download-manager-view.edj");
	elm_theme_free(m_theme);
#endif
}

Evas_Object *DownloadView::create(void)
{
	Evas_Object *window = NULL;
	Evas_Object *conformant = NULL;
	FileUtility fileObj;
	window = createWindow(PACKAGE_NAME);
	if (!window)
		return NULL;

	if (elm_win_wm_rotation_supported_get(window)) {
		int rots[4] = {0, 90, 180, 270};
		elm_win_wm_rotation_available_rotations_set(window, rots, 4);
	}
	/* If there are some job when the rotation event is happened, please enable this.
	 * It is handled to rotate of a window by window manager. Please don't care it */
//	evas_object_smart_callback_add(window, "wm,rotation,changed", _rot_changed_cb, NULL);

	conformant = createConformant(window);
	createBackground(window);
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
	if(eoDldList)
		it = elm_genlist_first_item_get(eoDldList);

	while (it) {
		item = (ViewItem *)elm_object_item_data_get(it);
		it = elm_genlist_item_next_get(it);
		if (item && item->isGroupTitle() == false) {
			if (item->getState() < ITEM::FINISH_DOWNLOAD)
				item->cancel();
			item->destroy();
		}
	}
	engine.deinitEngine();
	if (isGenlistEditMode()) {
		hideGenlistEditMode();
	}
	removePopup();
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
#ifndef _TIZEN_2_3_UX
	if(eoToolBar) {
		evas_object_del(eoToolBar);
		eoToolBar = NULL;
	}
#endif
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
#ifndef _TIZEN_2_3_UX
	if(eoIndicatorBackground) {
		evas_object_del(eoIndicatorBackground);
		eoIndicatorBackground = NULL;
	}
#endif
	if(eoBackground) {
		evas_object_del(eoBackground);
		eoBackground = NULL;
	}
	if(eoWindow) {
		evas_object_del(eoWindow);
		eoWindow = NULL;
	}
	if(m_pauseTimer) {
		ecore_timer_del(m_pauseTimer);
		m_pauseTimer = NULL;
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
	if (!user_data) {
		DM_LOGE("NULL Check:user_data");
		return;
	}
	if (!node) {
		DM_LOGE("NULL Check:node");
		return;
	}
}

void DownloadView::checkEditMode()
{
	DM_LOGD("");
	if (isGenlistEditMode() && !m_activatedLockScreen) {
		DM_LOGI("Clear genlist mode");
		hideGenlistEditMode();
	}
}

Eina_Bool DownloadView::pauseTimerCB(void *data)
{
	DM_LOGD("");
	DownloadView &view = DownloadView::getInstance();
	view.m_pauseTimer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

void DownloadView::pause()
{
	DM_LOGI("");
//	removePopup();

	if (vconf_notify_key_changed(
			VCONFKEY_IDLE_LOCK_STATE, lockStateChangedCB, this) != 0) {
		DM_LOGE("Fail to vconf_notify_key_changed");
	}
	m_pauseTimer = ecore_timer_add(0.2, pauseTimerCB, NULL);
}

void DownloadView::resume()
{
	DM_LOGI("");
	if (vconf_ignore_key_changed(
			VCONFKEY_IDLE_LOCK_STATE, lockStateChangedCB) != 0) {
		DM_LOGD("Fail to vconf_ignore_key_changed");
	}
	// one more check if the vconf value of lock state has not been changed due to some error
	m_activatedLockScreen = false;
	DateUtil &util = DateUtil::getInstance();
	int diffDays = util.getDiffDays(time(NULL),(time_t)m_viewLastRefreshTime);
	if (diffDays > 0) {
		if (m_viewItemCount > 0)
			update(diffDays);
		m_viewLastRefreshTime = time(NULL);
	}
}

void DownloadView::activateWindow()
{
	if (!eoWindow)
		create();
	else
		resume();
	show();
}

void DownloadView::clickedItemFromNoti(unsigned id, NOTIFICATION_TYPE::TYPE type)
{
	Elm_Object_Item *it = NULL;
	ViewItem *item = NULL;
	it = elm_genlist_first_item_get(eoDldList);
	while (it) {
		item = (ViewItem *)elm_object_item_data_get(it);
		/* elm_genlist_item_select_mode_get is needed to check group item */
		if (item && item->isGroupTitle() == false && item->getHistoryId() == id) {
			elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
			if (type == NOTIFICATION_TYPE::NOTI_FAILED) {
				string msg[2];
				msg[0] = __("IDS_DM_BODY_DOWNLOAD_FAILED_M_STATUS_ABB");
#ifdef _TIZEN_2_3_UX
				msg[1] = __("IDS_DM_BODY_TAP_DOWNLOAD_TO_TRY_AGAIN");
#else
				msg[1] = __("IDS_DM_POP_FILE_NOT_RECEIVED_DOWNLOAD_AGAIN_Q");
#endif
				item->setIsClickedFromNoti(true);
				showRetryPopup(item, msg);
			}
			break;
		}
		it = elm_genlist_item_next_get(it);
	}
}

void DownloadView::setIndicator(Evas_Object *window)
{
	elm_win_indicator_mode_set(window, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(window, ELM_WIN_INDICATOR_OPAQUE);
}

Evas_Object *DownloadView::createWindow(const char *windowName)
{
    bindtextdomain(PROJ_NAME, LOCALEDIR);
	eoWindow = elm_win_add(NULL, windowName, ELM_WIN_BASIC);
	if (eoWindow) {
		elm_win_title_set(eoWindow, DM_HEADER_DOWNLOAD_MANAGER);
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
#ifndef _TIZEN_2_3_UX
	eoIndicatorBackground = elm_bg_add(window);
	if (eoIndicatorBackground) {
		evas_object_size_hint_weight_set(eoIndicatorBackground, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_style_set(eoIndicatorBackground, "indicator/headerbg");
		elm_object_part_content_set(eoConform, "elm.swallow.indicator_bg", eoIndicatorBackground);
	}
#endif
	eoBackground = elm_bg_add(window);
	if (eoBackground) {
		evas_object_size_hint_weight_set(eoBackground, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
#ifdef _TIZEN_2_3_UX
		elm_bg_color_set(eoBackground, 255, 255, 255);
#endif
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
#ifdef _TIZEN_2_3_UX
		elm_object_part_content_set(eoLayout, "elm.swallow.bg", eoBackground);
#endif
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
	if (m_viewItemCount < 1)
		showEmptyView();
	else
		createList();
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
	eext_object_event_callback_add(eoNaviBar, EEXT_CALLBACK_BACK,
			eext_naviframe_back_cb, NULL);
	createBox();
	eoNaviBarItem = elm_naviframe_item_push(eoNaviBar,
	        DM_HEADER_DOWNLOAD_MANAGER ,NULL, NULL, eoBox, NULL);
	if (!eoNaviBarItem)
		DM_LOGE("Null Check:eoNaviBarItem");
#ifdef _TIZEN_2_3_UX
	eext_object_event_callback_add(eoNaviBar, EEXT_CALLBACK_MORE, moreKeyCB, NULL);
#else
	createDeleteBtn();
#endif

	elm_naviframe_item_pop_cb_set(eoNaviBarItem, popCB, NULL);
	evas_object_show(eoNaviBar);
}

void DownloadView::deletePopupDeleteCB(void *data, Evas_Object *obj, void *event_info)
{
	DownloadView& view = DownloadView::getInstance();
	view.destroyCheckedItem();
	view.removePopup();
	Evas_Object *navi_it_access_obj = elm_object_item_access_object_get(view.eoNaviBarItem);
	elm_access_highlight_set(navi_it_access_obj);
}

void DownloadView::showDeletePopup()
{
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	char buff[MAX_BUF_LEN] = {0,};

	DM_LOGI("");

	/* If another popup is shown, delete it*/
	removePopup();
	eoPopup = elm_popup_add(eoWindow);
	elm_popup_align_set(eoPopup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoPopup, EVAS_HINT_FILL,
		EVAS_HINT_FILL);
#ifdef _TIZEN_2_3_UX
	snprintf(buff, MAX_BUF_LEN, "%s%s%s", "<align=left>",
		DM_OPT_TEXT_DELETE, "</align>");
	elm_object_part_text_set(eoPopup, "title,text", buff);

	if (m_selectedItemsCount == 1) {
		snprintf(buff, MAX_BUF_LEN, "%s", __("IDS_DM_BODY_1_ITEM_WILL_BE_DELETED"));
	} else {
		const char *format_str = __("IDS_DM_BODY_PD_ITEMS_WILL_BE_DELETED");
		snprintf(buff, MAX_BUF_LEN, format_str, m_selectedItemsCount);
	}
#else
	elm_object_part_text_set(eoPopup, "title,text", __("IDS_DM_HEADER_DELETE_FILE"));

	if (m_selectedItemsCount == 1) {
		snprintf(buff, MAX_BUF_LEN, "%s", __("IDS_DM_POP_DELETE_Q"));
	} else {
		const char *format_str = __("IDS_DM_POP_DELETE_PD_FILES_Q");
		snprintf(buff, MAX_BUF_LEN, format_str, m_selectedItemsCount);
	}
#endif
	elm_object_text_set(eoPopup, buff);
	btn1 = elm_button_add(eoPopup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, DM_SK_TEXT_CANCEL);
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", popupBackCB,
			NULL);
	btn2 = elm_button_add(eoPopup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, DM_OPT_TEXT_DELETE);
	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", deletePopupDeleteCB,
			NULL);
	eext_object_event_callback_add(eoPopup, EEXT_CALLBACK_BACK, popupBackCB, NULL);
	evas_object_show(eoPopup);
}

void DownloadView::deleteToolBarBtnCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGI("");
	DownloadView& view = DownloadView::getInstance();
	view.showDeletePopup();
}

#ifdef _TIZEN_2_3_UX
void DownloadView::moreKeyCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGD("");
	DownloadView &view = DownloadView::getInstance();
	view.createContextPopup();
}

void DownloadView::rotateContextPopupCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGD("");
	DownloadView &view = DownloadView::getInstance();
	view.moveContextPopup();
}

void DownloadView::contextPopupDismissedCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGD("");
	DownloadView &view = DownloadView::getInstance();
	view.deleteContextPopup();
}

void DownloadView::tabBarCancelButtonCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGD("");
	DownloadView &view = DownloadView::getInstance();
	view.hideGenlistEditMode();
}


void DownloadView::createContextPopup()
{
	DM_LOGD("");

	if (isGenlistEditMode() || eoEmptyNoContent)
		return;

	Evas_Object *moreMenu = elm_ctxpopup_add(eoWindow);
	if (!moreMenu) {
		DM_LOGE("elm_ctxpopup_add failed");
		return;
	}
	elm_object_style_set(moreMenu, "more/default");
	elm_ctxpopup_direction_priority_set(moreMenu, ELM_CTXPOPUP_DIRECTION_UP,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	evas_object_size_hint_weight_set(moreMenu, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(moreMenu, "dismissed", contextPopupDismissedCB, this);
	elm_ctxpopup_auto_hide_disabled_set(moreMenu, EINA_TRUE);
	evas_object_smart_callback_add(elm_object_top_widget_get(moreMenu),
			"rotation,changed", rotateContextPopupCB, this);
	eext_object_event_callback_add(moreMenu, EEXT_CALLBACK_BACK,
			eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(moreMenu, EEXT_CALLBACK_MORE,
			eext_ctxpopup_back_cb, NULL);
	Elm_Object_Item *item = elm_ctxpopup_item_append(moreMenu,
			DM_OPT_TEXT_DELETE, NULL, deleteBtnCB, NULL);
	if (m_viewItemCount < 1)
		elm_object_item_disabled_set(item, EINA_TRUE);

	eoMoreMenu = moreMenu;
	moveContextPopup();
	evas_object_show(moreMenu);
}

void DownloadView::deleteContextPopup()
{
	DM_LOGD("");
	evas_object_smart_callback_del(eoWindow,
			"rotation,changed", rotateContextPopupCB);
	evas_object_del(eoMoreMenu);
	eoMoreMenu = NULL;
}

void DownloadView::moveContextPopup()
{
	DM_LOGD("");
	if (!eoMoreMenu) {
		DM_LOGE("Null Check:eoMoreMenu");
		return;
	}
	Evas_Coord w, h;
	int pos = -1;

	elm_win_screen_size_get(eoWindow, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(eoWindow);
	switch (pos) {
		case 0:
		case 180:
			evas_object_move(eoMoreMenu, (w/2), h);
			break;
		case 90:
			evas_object_move(eoMoreMenu, (h/2), w);
			break;
		case 270:
			evas_object_move(eoMoreMenu, (h/2), w);
			break;
	}
}
#else

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
	eoToolBarItem = elm_toolbar_item_append(eoToolBar, NULL,
			DM_OPT_TEXT_DELETE, deleteToolBarBtnCB, NULL);
	if (eoNaviBarItem)
		elm_object_item_part_content_set(eoNaviBarItem, "toolbar", eoToolBar);
	evas_object_show(eoToolBar);
}

void DownloadView::destroyToolBar()
{
	if(eoToolBar) {
		evas_object_del(eoToolBar);
		eoToolBar = NULL;
		eoToolBarItem = NULL;
	}
	if (eoNaviBarItem)
		elm_object_item_part_content_unset(eoNaviBarItem, "toolbar");
}

void DownloadView::createDeleteBtn()
{
	Evas_Object *icon = NULL;
	Evas_Object *button = elm_button_add(eoNaviBar);
	if (!button) {
		DM_LOGE("Null Check:eoNaviBarItem");
		return;
	}
	elm_object_style_set(button, "naviframe/title_icon");
	icon = elm_image_add(eoNaviBar);
	elm_image_file_set(icon, DM_DELETE_ICON_PATH, NULL);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_access_info_set(button, ELM_ACCESS_INFO, S_("IDS_COM_SK_DELETE"));
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
#endif

void DownloadView::deleteBtnCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGI("");
	DownloadView& view = DownloadView::getInstance();
	view.showGenlistEditMode();
}

void DownloadView::createBox()
{
	DM_LOGD("");

	eoBox = elm_box_add(eoNaviBar);
	evas_object_size_hint_weight_set(eoBox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoBox, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(eoBox);
}

void DownloadView::realizedCB(void *data, Evas_Object *obj,
	void *event_info) {
	Elm_Object_Item *it = NULL;
	Elm_Object_Item *nextIt = NULL;
	ViewItem *item = NULL;
	if (!event_info) {
		DM_LOGE("NULL Check:event_info");
		return;
	}
	if (!obj) {
		DM_LOGE("NULL Check:obj");
		return;
	}
	it = (Elm_Object_Item *)event_info;
	if (it != elm_genlist_last_item_get(obj))
		nextIt = elm_genlist_item_next_get(it);
	if (nextIt) {
		item = (ViewItem *)elm_object_item_data_get(nextIt);
		if (item && item->isGroupTitle()) {
			elm_object_item_signal_emit(it, "elm,state,bottomline,hide", "");
		}
	}
}

void DownloadView::createList()
{
	eoDldList = elm_genlist_add(eoLayout);
	DM_LOGTEST("CREATE:eoDldList[%p]",eoDldList);
/* When using ELM_LIST_LIMIT, the window size is broken at the landscape mode */
	evas_object_smart_callback_add(eoDldList, "language,changed", changedGenlistLanguage, NULL);
	evas_object_size_hint_weight_set(eoDldList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoDldList, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(eoDldList, "realized", realizedCB, NULL);
	elm_genlist_homogeneous_set(eoDldList, EINA_TRUE);
	elm_genlist_mode_set(eoDldList, ELM_LIST_COMPRESS);
#ifdef _TIZEN_2_3_UX
	elm_object_theme_set(eoDldList, m_theme);
#endif
	elm_genlist_block_count_set(eoDldList,8);
	elm_box_pack_end(eoBox, eoDldList);
	evas_object_show(eoDldList);
}

void DownloadView::changedGenlistLanguage(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_realized_items_update(obj);
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
#ifndef _TIZEN_2_3_UX
		if (eoNaviBarItem)
			if (!elm_object_item_part_content_get(eoNaviBarItem,
					"title_right_btn")) {
				createDeleteBtn();
			}
#endif
	}
	if (viewItem) {
		addViewItemToGenlist(viewItem);
		m_viewItemCount++;
	}
}

void DownloadView::detachViewItem(ViewItem *viewItem)
{
	DM_LOGI("delete viewItem[%p]",viewItem);
	if (viewItem && viewItem->isGroupTitle() == false) {
		groupTitleType &groupTitleItem = m_groupTitle[viewItem->getItemGroup()];
		m_viewItemCount--;
		groupTitleItem.subItemsCount--;
		elm_object_item_del(viewItem->genlistItem());
		if (groupTitleItem.subItemsCount == 0) {
			elm_object_item_del(groupTitleItem.viewItem->genlistItem());
			groupTitleItem.viewItem = NULL;
		}
	}
	if (m_viewItemCount < 1 && !isGenlistEditMode())
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

void DownloadView::update(int diffDays)
{
	DM_LOGD("");
	groupTitleType &todayGroupTitle = m_groupTitle[VIEWITEM_GROUP::TODAY];
	groupTitleType &yesterdayGroupTitle = m_groupTitle[VIEWITEM_GROUP::YESTERDAY];
	groupTitleType &olderGroupTitle = m_groupTitle[VIEWITEM_GROUP::OLDER];
	Elm_Object_Item *lastItem = NULL;

	if (diffDays == 1) {
		Elm_Object_Item *yesterdayLastItem = NULL;
		if (yesterdayGroupTitle.viewItem) {
			if (olderGroupTitle.viewItem) {
				yesterdayLastItem = elm_genlist_item_prev_get(
						olderGroupTitle.viewItem->genlistItem());
				elm_object_item_del(olderGroupTitle.viewItem->genlistItem());
				if (yesterdayLastItem)
					elm_object_item_signal_emit(
							yesterdayLastItem, "elm,state,bottomline,show", "");
			} else {
				yesterdayLastItem = elm_genlist_last_item_get(eoDldList);
			}
			olderGroupTitle.subItemsCount += yesterdayGroupTitle.subItemsCount;
			olderGroupTitle.viewItem = yesterdayGroupTitle.viewItem;
			olderGroupTitle.viewItem->setItemGroup(VIEWITEM_GROUP::OLDER);
			yesterdayGroupTitle.viewItem = NULL;
			yesterdayGroupTitle.subItemsCount = 0;
		}
		if (todayGroupTitle.viewItem) {
			yesterdayGroupTitle.viewItem = todayGroupTitle.viewItem;
			yesterdayGroupTitle.subItemsCount = todayGroupTitle.subItemsCount;
			yesterdayGroupTitle.viewItem->setItemGroup(VIEWITEM_GROUP::YESTERDAY);
			todayGroupTitle.viewItem = NULL;
			todayGroupTitle.subItemsCount = 0;
		}
		lastItem = yesterdayLastItem;
	} else {
		Elm_Object_Item *todayLastItem = NULL;
		Elm_Object_Item *yesterdayLastItem = NULL;
		int itemsCount = olderGroupTitle.subItemsCount;
		bool flag = false;

		if (todayGroupTitle.viewItem) {
			if (yesterdayGroupTitle.viewItem) {
				todayLastItem = elm_genlist_item_prev_get(
					yesterdayGroupTitle.viewItem->genlistItem());
				flag = true;
			} else if (olderGroupTitle.viewItem) {
				todayLastItem = elm_genlist_item_prev_get(
					olderGroupTitle.viewItem->genlistItem());
				flag = true;
			} else {
				todayLastItem = elm_genlist_last_item_get(eoDldList);
			}
			if (flag && todayLastItem)
				elm_object_item_signal_emit(
						todayLastItem, "elm,state,bottomline,show", "");
			lastItem = todayLastItem;
		}
		if (yesterdayGroupTitle.viewItem) {
			if (olderGroupTitle.viewItem) {
				yesterdayLastItem = elm_genlist_item_prev_get(
							olderGroupTitle.viewItem->genlistItem());
				if (yesterdayLastItem)
					elm_object_item_signal_emit(
						yesterdayLastItem, "elm,state,bottomline,show", "");
			} else {
				yesterdayLastItem = elm_genlist_last_item_get(eoDldList);
			}
			lastItem = yesterdayLastItem;
		}
		flag = false;
		if (todayGroupTitle.viewItem) {
			itemsCount += todayGroupTitle.subItemsCount;
			if (olderGroupTitle.viewItem)
				elm_object_item_del(olderGroupTitle.viewItem->genlistItem());
			olderGroupTitle.viewItem = todayGroupTitle.viewItem;
			olderGroupTitle.viewItem->setItemGroup(VIEWITEM_GROUP::OLDER);
			todayGroupTitle.viewItem = NULL;
			todayGroupTitle.subItemsCount = 0;
			flag = true;
		}
		if (yesterdayGroupTitle.viewItem) {
			itemsCount += yesterdayGroupTitle.subItemsCount;
			if (flag)
				elm_object_item_del(yesterdayGroupTitle.viewItem->genlistItem());
			else {
				if (olderGroupTitle.viewItem)
					elm_object_item_del(olderGroupTitle.viewItem->genlistItem());
				olderGroupTitle.viewItem = yesterdayGroupTitle.viewItem;
				olderGroupTitle.viewItem->setItemGroup(VIEWITEM_GROUP::OLDER);
			}
			yesterdayGroupTitle.subItemsCount = 0;
			yesterdayGroupTitle.viewItem = NULL;
		}
		olderGroupTitle.subItemsCount = itemsCount;
	}
	ViewItem *viewItem = NULL;
	Elm_Object_Item *item1 = NULL;
	Elm_Object_Item *item2 = NULL;
	if (yesterdayGroupTitle.viewItem)
		item1 = yesterdayGroupTitle.viewItem->genlistItem();
	if (olderGroupTitle.viewItem)
		item2 = olderGroupTitle.viewItem->genlistItem();
	/* Update the viewItem group of all the Items moved to Yesterday group */
	while(item1 && item1 != item2) {
		viewItem = (ViewItem *)elm_object_item_data_get(item1);
		if (viewItem)
			viewItem->setItemGroup(VIEWITEM_GROUP::YESTERDAY);
		item1 = elm_genlist_item_next_get(item1);
	}
	/* Update time field of genlist Item and viewItem group
	for all the Items moved to Older Group
	*/
	if (lastItem) {
		item1 = item2;
		while(item1) {
#ifdef _TIZEN_2_3_UX
			elm_genlist_item_fields_update(item1 , "elm.text.sub.end",
					ELM_GENLIST_ITEM_FIELD_TEXT);
#else
			elm_genlist_item_fields_update(item1 , "elm.text.3",
					ELM_GENLIST_ITEM_FIELD_TEXT);
#endif
			viewItem = (ViewItem *)elm_object_item_data_get(item1);
			if (viewItem)
				viewItem->setItemGroup(VIEWITEM_GROUP::OLDER);
			if (item1 == lastItem)
				break;
			item1 = elm_genlist_item_next_get(item1);
		}
	}
	update(todayGroupTitle.viewItem);
	update(yesterdayGroupTitle.viewItem);
	update(olderGroupTitle.viewItem);
}

void DownloadView::updateLang()
{
	DM_LOGD("");
	if (eoNaviBarItem) {
		if (isGenlistEditMode()) {
#ifdef _TIZEN_2_3_UX
			if (eoSelectAllLayout)
				elm_object_part_text_set(eoSelectAllLayout, "elm.text",
					DM_BODY_TEXT_SELECT_ALL);
			showSelectedNotify(m_selectedItemsCount);
#else
			if (eoToolBarItem)
				elm_object_item_text_set(eoToolBarItem, DM_OPT_TEXT_DELETE);
			elm_object_item_text_set(eoNaviBarItem, __("IDS_DM_HEADER_SELECT_ITEMS"));
#endif
		} else {
#ifndef _TIZEN_2_3_UX
			Evas_Object *buttonObj = NULL;
			buttonObj = elm_object_item_part_content_get(eoNaviBarItem, "title_right_btn");
			elm_object_text_set(buttonObj, DM_OPT_TEXT_DELETE);
#endif
			elm_object_item_text_set(eoNaviBarItem,
			        DM_HEADER_DOWNLOAD_MANAGER);
		}
	}
	if (eoEmptyNoContent) {
		elm_object_part_text_set(eoEmptyNoContent, "elm.text",
				__("IDS_DM_BODY_NO_DOWNLOADS"));
		elm_object_part_text_set(eoEmptyNoContent, "elm.help.text",
					__("IDS_DM_BODY_AFTER_YOU_DOWNLOAD_ITEMS_THEY_WILL_BE_SHOWN_HERE"));
	}
}

void DownloadView::addViewItemToGenlist(ViewItem *viewItem)
{
	DM_LOGD("");
	createGenlistItem(viewItem);
}

void DownloadView::createGenlistItem(ViewItem *viewItem)
{
	DM_LOGTEST("CreateItem:eoDldList[%p]", eoDldList);
	int diffDays = 0;
	ViewItem *parentItem = NULL;
	Elm_Object_Item *glItem = NULL;
	VIEWITEM_GROUP::GROUP group;
	double finishedTime = viewItem->getFinishedTime();
	if (finishedTime == 0)
		diffDays = 0;
	else {
		DateUtil &inst = DateUtil::getInstance();
		diffDays = inst.getDiffDays(time(NULL),(time_t)finishedTime);
	}

	if (diffDays < 1)
		group = VIEWITEM_GROUP::TODAY;
	else if (diffDays < 2)
		group = VIEWITEM_GROUP::YESTERDAY;
	else
		group = VIEWITEM_GROUP::OLDER;
	groupTitleType &groupTitleItem = m_groupTitle[group];
	if (!groupTitleItem.viewItem)
		groupTitleItem.viewItem = new ViewItem(group);
	parentItem = groupTitleItem.viewItem;

	if (groupTitleItem.subItemsCount == 0) {
		if (group == VIEWITEM_GROUP::TODAY) {
			glItem = elm_genlist_item_prepend(
				eoDldList,
				parentItem->elmGenlistStyle(),
				static_cast<const void*>(parentItem),
				NULL,
				ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		} else {
			glItem = elm_genlist_item_append(
				eoDldList,
				parentItem->elmGenlistStyle(),
				static_cast<const void*>(parentItem),
				NULL,
				ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
		}
		elm_genlist_item_select_mode_set(glItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		parentItem->setGenlistItem(glItem);
	}
	if (viewItem->getInsertAtFirst()) {
		glItem = elm_genlist_item_insert_after(
			eoDldList,
			viewItem->elmGenlistStyle(),
			static_cast<const void*>(viewItem),
			NULL,
			parentItem->genlistItem(),
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
	else {
		viewItem->setItemGroup(group);
		viewItem->setGenlistItem(glItem);
		groupTitleItem.subItemsCount++;
	}

	DM_LOGTEST("CreateItem:genlist item[%p] viewItem[%p]", glItem, viewItem);
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
		elm_layout_theme_set(eoEmptyNoContent, "layout", "nocontents", "default");
		evas_object_size_hint_weight_set(eoEmptyNoContent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(eoEmptyNoContent, EVAS_HINT_FILL, EVAS_HINT_FILL);
#ifndef _TIZEN_2_3_UX
		Evas_Object *icon = elm_image_add(eoEmptyNoContent);
		elm_image_file_set(icon, DM_NO_CONTENT_ICON_PATH, NULL);
		elm_object_part_content_set(eoEmptyNoContent, "nocontents.image", icon);
#endif
		elm_object_part_text_set(eoEmptyNoContent, "elm.text",
			__("IDS_DM_BODY_NO_DOWNLOADS"));
		elm_object_part_text_set(eoEmptyNoContent, "elm.help.text",
			__("IDS_DM_BODY_AFTER_YOU_DOWNLOAD_ITEMS_THEY_WILL_BE_SHOWN_HERE"));
#ifndef _TIZEN_2_3_UX
		Evas_Object *accessObject = NULL;
		Evas_Object *noContentAccess = (Evas_Object *)edje_object_part_object_get(_EDJ(eoEmptyNoContent), "elm.text");
		accessObject = elm_access_object_register(noContentAccess, eoEmptyNoContent);
		elm_object_focus_custom_chain_append(eoEmptyNoContent, accessObject, NULL);
		elm_access_info_set(accessObject, ELM_ACCESS_INFO, __("IDS_DM_BODY_NO_DOWNLOADS"));
		Evas_Object *noContentHelpAccess = (Evas_Object *)edje_object_part_object_get(_EDJ(eoEmptyNoContent), "elm.help.text");
		accessObject = elm_access_object_register(noContentHelpAccess, eoEmptyNoContent);
		elm_object_focus_custom_chain_append(eoEmptyNoContent, accessObject, NULL);
		elm_access_info_set(accessObject, ELM_ACCESS_INFO,
					__("IDS_DM_BODY_AFTER_YOU_DOWNLOAD_ITEMS_THEY_WILL_BE_SHOWN_HERE"));
#endif
		elm_layout_signal_emit(eoEmptyNoContent, "align.center", "elm");
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
#ifndef _TIZEN_2_3_UX
	destroyDeleteBtn();
#endif
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
#ifdef _TIZEN_2_3_UX
	return m_isEditMode;
#else
	if (eoDldList)
		return (bool)elm_genlist_decorate_mode_get(eoDldList);
	else
		return false;
#endif
}

#ifdef _TIZEN_2_3_UX
void DownloadView::createSelectAllLayout()
{
	DM_LOGD("");
	eoSelectAllLayout = elm_layout_add(eoBox);
	if (!eoSelectAllLayout) {
		DM_LOGE("elm_layout_add is failed");
		return;
	}

	elm_object_focus_allow_set(eoSelectAllLayout, EINA_TRUE);

	elm_layout_theme_set(eoSelectAllLayout, "genlist", "item", "select_all/default");
	evas_object_size_hint_weight_set(eoSelectAllLayout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(eoSelectAllLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *check = elm_check_add(eoSelectAllLayout);
	evas_object_propagate_events_set(check, EINA_TRUE);
	elm_object_part_content_set(eoSelectAllLayout, "elm.icon", check);
	elm_object_part_text_set(eoSelectAllLayout, "elm.text.main", DM_BODY_TEXT_SELECT_ALL);
	evas_object_show(eoSelectAllLayout);
	elm_box_pack_start(eoBox, eoSelectAllLayout);

	evas_object_event_callback_add(eoSelectAllLayout, EVAS_CALLBACK_MOUSE_DOWN,
			selectAllChangedCB, this);

	Evas_Object *cancelButton = elm_button_add(eoNaviBar);
	if (!cancelButton) return;
	elm_object_style_set(cancelButton, "naviframe/title_cancel");
	evas_object_smart_callback_add(cancelButton, "clicked", tabBarCancelButtonCB, NULL);
	elm_object_item_part_content_set(eoNaviBarItem, "title_left_btn", cancelButton);
	DM_LOGD("");
	Evas_Object *doneButton = elm_button_add(eoNaviBar);
	if (!doneButton) return;
	elm_object_style_set(doneButton, "naviframe/title_done");
	evas_object_smart_callback_add(doneButton, "clicked", deleteToolBarBtnCB, this);
	elm_object_item_part_content_set(eoNaviBarItem, "title_right_btn", doneButton);
	elm_object_disabled_set(doneButton, EINA_TRUE);
	m_allChecked = EINA_FALSE;
}

void DownloadView::deleteSelectAllLayout()
{
	if (eoSelectAllLayout) {
		elm_box_unpack(eoBox, eoSelectAllLayout);
		evas_object_del(eoSelectAllLayout);
		eoSelectAllLayout = NULL;
	}
	Evas_Object *cancelButton = elm_object_item_part_content_unset(
			eoNaviBarItem, "title_left_btn");
	evas_object_hide(cancelButton);
	Evas_Object *doneButton = elm_object_item_part_content_unset(
			eoNaviBarItem, "title_right_btn");
	evas_object_hide(doneButton);
	m_allChecked = EINA_FALSE;
}
#else
void DownloadView::changeSelectAll()
{
	Evas_Object *icon = NULL;
	Evas_Object *button = NULL;
	button = elm_object_item_part_content_get(eoNaviBarItem, "title_right_btn");
	icon = elm_object_part_content_get(button, "icon");
	elm_image_file_set(icon, DM_SELECT_ALL_ICON_PATH, NULL);
	evas_object_smart_callback_del(button, "clicked", deleteBtnCB);
	evas_object_smart_callback_add(button, "clicked", selectAllChangedCB, NULL);
	evas_object_show(button);
	m_allChecked = EINA_FALSE;
	elm_access_info_set(button, ELM_ACCESS_INFO, DM_BODY_TEXT_SELECT_ALL);
}

void DownloadView::restoreDeleteBtn()
{
	Evas_Object *icon = NULL;
	Evas_Object *button = NULL;
	button = elm_object_item_part_content_get(eoNaviBarItem, "title_right_btn");
	icon = elm_object_part_content_get(button, "icon");
	elm_image_file_set(icon, DM_DELETE_ICON_PATH, NULL);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_access_info_set(button, ELM_ACCESS_INFO, S_("IDS_COM_SK_DELETE"));
	evas_object_smart_callback_del(button, "clicked", selectAllChangedCB);
	evas_object_smart_callback_add(button, "clicked", deleteBtnCB, NULL);
	evas_object_show(button);
	m_allChecked = EINA_FALSE;
}
#endif

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
			deleteQueue.push(item->getHistoryId());
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

#ifdef _TIZEN_2_3_UX
	deleteContextPopup();
	showSelectedNotify(0);
	m_isEditMode = true;
	createSelectAllLayout();
//	elm_genlist_select_mode_set(eoDldList, ELM_OBJECT_SELECT_MODE_ALWAYS);
#else
	/* Initialize notify info widget */
	elm_object_item_text_set(eoNaviBarItem, __("IDS_DM_HEADER_SELECT_ITEMS"));
	/* Change layoutbackground color to edit mode color */
	elm_object_style_set(eoBackground, "edit_mode");

	createToolBar();

	/* Append 'Select All' layout */
	changeSelectAll();
	/* Set reorder end edit mode */
	elm_genlist_decorate_mode_set(eoDldList, EINA_TRUE);
	/* This means even if the ouside of checked box is selected,
	   it is same to click a check box. */
	elm_genlist_select_mode_set(eoDldList, ELM_OBJECT_SELECT_MODE_ALWAYS);
#endif
	Elm_Object_Item *it = NULL;
	Elm_Object_Item *prev = NULL;
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
			if (viewItem->isGroupTitle() && prev) {
				elm_object_item_signal_emit(prev,
					"elm,state,bottomline,hide", "");
			}
		}
		prev = it;
		it = elm_genlist_item_next_get(it);
	}


#ifdef _TIZEN_2_3_UX
	/* Update all the realized Items */
	if (m_viewItemCount > 0)
		elm_genlist_realized_items_update(eoDldList);
#else
	elm_object_item_disabled_set(eoToolBarItem, EINA_TRUE);
#endif
}

void DownloadView::hideGenlistEditMode()
{
	Elm_Object_Item *it = NULL;
	ViewItem *viewItem = NULL;

	DM_LOGD("");

	removePopup();
	elm_object_item_text_set(eoNaviBarItem,
	        DM_HEADER_DOWNLOAD_MANAGER);
#ifdef _TIZEN_2_3_UX
	m_isEditMode = false;
	deleteSelectAllLayout();
#else
	elm_object_style_set(eoBackground, "default");
	elm_genlist_decorate_mode_set(eoDldList, EINA_FALSE);
	elm_genlist_select_mode_set(eoDldList, ELM_OBJECT_SELECT_MODE_DEFAULT);
#endif
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
#ifdef _TIZEN_2_3_UX
	/* Update all the realized Items */
	if (m_viewItemCount > 0)
		elm_genlist_realized_items_update(eoDldList);
#else
	destroyToolBar();
	restoreDeleteBtn();
#endif

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
	m_allChecked = !m_allChecked;
	while (it) {
		viewItem = (ViewItem *)elm_object_item_data_get(it);
		if (viewItem) {
			if (viewItem->isFinished()) {
				viewItem->setCheckedValue(m_allChecked);
#ifdef _TIZEN_2_3_UX
				elm_genlist_item_fields_update(it, "elm.swallow.icon.2",
					ELM_GENLIST_ITEM_FIELD_CONTENT);
#else
				elm_genlist_item_fields_update(it, "elm.edit.icon.1",
					ELM_GENLIST_ITEM_FIELD_CONTENT);
#endif
				checkedCount++;
			}
		}
		it = elm_genlist_item_next_get(it);
	}

	if (m_allChecked && checkedCount > 0) {
#ifdef _TIZEN_2_3_UX
		Evas_Object *doneButton = elm_object_part_content_get(eoNaviBar, "title_right_btn");
		elm_object_disabled_set(doneButton, EINA_FALSE);
		Evas_Object *checkBox = elm_object_part_content_get(eoSelectAllLayout, "elm.swallow.icon");
		if (checkBox)
			elm_check_state_set(checkBox, m_allChecked);
#else
		elm_object_item_disabled_set(eoToolBarItem, EINA_FALSE);
#endif
		showSelectedNotify(checkedCount);
	} else {
		m_allChecked = EINA_FALSE;
		showSelectedNotify(0);
#ifdef _TIZEN_2_3_UX
		Evas_Object *doneButton = elm_object_part_content_get(eoNaviBar, "title_right_btn");
		elm_object_disabled_set(doneButton, EINA_TRUE);
		Evas_Object *checkBox = elm_object_part_content_get(eoSelectAllLayout, "elm.swallow.icon");
		if (checkBox)
			elm_check_state_set(checkBox, m_allChecked);
#else
		elm_object_item_disabled_set(eoToolBarItem, EINA_TRUE);
#endif
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

#ifdef _TIZEN_2_3_UX
	if (eoSelectAllLayout) {
		Evas_Object *checkBox = elm_object_part_content_get(eoSelectAllLayout, "elm.swallow.icon");
		if (checkBox)
			elm_check_state_set(checkBox, m_allChecked);
	}
	Evas_Object *doneButton = elm_object_part_content_get(eoNaviBar, "title_right_btn");
#endif


	if (checkedCount == 0) {
#ifdef _TIZEN_2_3_UX
		elm_object_disabled_set(doneButton, EINA_TRUE);
#else
		elm_object_item_disabled_set(eoToolBarItem, EINA_TRUE);
#endif
	} else {
#ifdef _TIZEN_2_3_UX
		elm_object_disabled_set(doneButton, EINA_FALSE);
#else
		elm_object_item_disabled_set(eoToolBarItem, EINA_FALSE);
#endif
	}
	showSelectedNotify(checkedCount);
}

void DownloadView::showSelectedNotify(int selectedCount)
{
	char buff[MAX_BUF_LEN] = {0,};

	DM_LOGD("");
#ifndef _TIZEN_2_3_UX
	if (selectedCount <= 0) {
		elm_object_item_text_set(eoNaviBarItem, __("IDS_DM_HEADER_SELECT_ITEMS"));
		return;
	}
#endif
	m_selectedItemsCount = selectedCount;
	const char *format_str = DM_BODY_TEXT_PD_SELECTED;
	snprintf(buff, MAX_BUF_LEN, format_str, selectedCount);

	elm_object_item_text_set(eoNaviBarItem, buff);
}

#ifdef _TIZEN_2_3_UX
void DownloadView::selectAllChangedCB(void *data, Evas *e, Evas_Object *obj,
	void *event_info)
{
	DM_LOGD("");
	DownloadView &view = DownloadView::getInstance();
	view.handleChangedAllCheckedState();
}
#else
void DownloadView::selectAllChangedCB(void *data, Evas_Object *obj,
	void *event_info)
{
	DownloadView &view = DownloadView::getInstance();
	DM_LOGD("");
	view.handleChangedAllCheckedState();
}
#endif

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

void DownloadView::showRetryPopup(ViewItem *viewItem, string *msg)
{
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;

	DM_LOGI("");
	if (!viewItem)
		return;
	/* If another popup is shown, delete it*/
	removePopup();
	eoPopup = elm_popup_add(eoWindow);
	elm_popup_align_set(eoPopup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoPopup, EVAS_HINT_FILL,
		EVAS_HINT_FILL);

#ifdef _TIZEN_2_3_UX
	string titleText = "<align=left>" + msg[0] + "</align>";
	elm_object_part_text_set(eoPopup, "title,text", titleText.c_str());
#else
	elm_object_part_text_set(eoPopup, "title,text", msg[0].c_str());
#endif
	elm_object_text_set(eoPopup, msg[1].c_str());
	btn1 = elm_button_add(eoPopup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, DM_SK_TEXT_CANCEL);
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", retryPopupCancelCB,
			viewItem);
	btn2 = elm_button_add(eoPopup);
	elm_object_style_set(btn2, "popup");
#ifdef _TIZEN_2_3_UX
		elm_object_text_set(btn2, DM_SK_TEXT_DOWNLOAD);
#else
		elm_object_text_set(btn2, S_("IDS_COM_SK_RETRY"));
#endif

	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", retryPopupRetryCB,
			viewItem);
	eext_object_event_callback_add(eoPopup, EEXT_CALLBACK_BACK, popupBackCB, NULL);
	evas_object_show(eoPopup);
}


void DownloadView::showMemoryFullPopup(string &msg)
{
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;
	char buff[MAX_BUF_LEN] = {0,};

	DM_LOGI("");

	if (m_silentMode) {
		DM_LOGI("silent mode");
		evas_object_show(eoWindow);
		elm_win_activate(eoWindow);
	}

	/* If another popup is shown, delete it*/
	removePopup();
	eoPopup = elm_popup_add(eoWindow);
	elm_popup_align_set(eoPopup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(eoPopup, EVAS_HINT_FILL,
			EVAS_HINT_FILL);

#ifdef _TIZEN_2_3_UX
	snprintf(buff, MAX_BUF_LEN, "%s%s%s", "<align=left>",
		__("IDS_DM_HEADER_DEFAULT_STROAGE_FULL"), "</align>");
	elm_object_part_text_set(eoPopup, "title,text", buff);
#else
	elm_object_part_text_set(eoPopup, "title,text",
			__("IDS_DM_HEADER_DEFAULT_STROAGE_FULL"));
#endif
	snprintf(buff, MAX_BUF_LEN, msg.c_str(), __("IDS_DM_BODY_STORAGE"));
	elm_object_text_set(eoPopup, buff);
	btn1 = elm_button_add(eoPopup);
	elm_object_style_set(btn1, "popup");
#ifdef _TIZEN_2_3_UX
	elm_object_text_set(btn1, DM_SK_TEXT_CANCEL);
#else
	elm_object_text_set(btn1, DM_SK_TEXT_OK);
#endif
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", memoryFullPopupCancelCB,
			NULL);
	btn2 = elm_button_add(eoPopup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, __("IDS_DM_BUTTON_GO_TO_STORAGE_ABB"));
	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", memoryFullPopupMyFilesCB,
			NULL);
	eext_object_event_callback_add(eoPopup, EEXT_CALLBACK_BACK, popupBackCB, NULL);
	evas_object_show(eoPopup);
}

void DownloadView::showErrPopup(string *desc)
{
	Evas_Object *btn1 = NULL;

	if (m_silentMode) {
		DM_LOGI("silent mode");
		evas_object_show(eoWindow);
		elm_win_activate(eoWindow);
	}

	removePopup();

	eoPopup = elm_popup_add(eoWindow);
	elm_popup_align_set(eoPopup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
#ifdef _TIZEN_2_3_UX
	string titleText = "<align=left>" + desc[0] + "</align>";
	elm_object_part_text_set(eoPopup, "title,text", titleText.c_str());
#else
	elm_object_part_text_set(eoPopup, "title,text", desc[0].c_str());
#endif
	elm_object_text_set(eoPopup, desc[1].c_str());
	btn1 = elm_button_add(eoPopup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, DM_SK_TEXT_OK);
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", errPopupResponseCB,
			NULL);
	eext_object_event_callback_add(eoPopup, EEXT_CALLBACK_BACK, popupBackCB, NULL);
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
	elm_popup_align_set(eoPopup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(eoPopup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(eoPopup, msg.c_str());
#ifdef _TIZEN_2_3_UX
	string titleText = "<align=left>" + string(DM_SK_TEXT_DOWNLOAD) + "</align>";
	elm_object_part_text_set(eoPopup, "title,text", titleText.c_str());
#else
	elm_object_part_text_set(eoPopup, "title,text", DM_SK_TEXT_DOWNLOAD);
#endif
	btn1 = elm_button_add(eoPopup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, DM_SK_TEXT_CANCEL);
	elm_object_part_content_set(eoPopup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", omaPopupResponseCancelCB,
		viewItem);
	btn2 = elm_button_add(eoPopup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, DM_SK_TEXT_OK);
	elm_object_part_content_set(eoPopup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", omaPopupResponseOKCB,
		viewItem);
	eext_object_event_callback_add(eoPopup, EEXT_CALLBACK_BACK, popupBackCB, NULL);
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
