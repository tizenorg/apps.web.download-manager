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
 * @file	download-manager-viewItem.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	item data class for download view
 */

#include <stdio.h>

#include "download-manager-viewItem.h"
#include "download-manager-items.h"
#include "download-manager-view.h"

Elm_Genlist_Item_Class ViewItem::dldGenlistStyle;
Elm_Genlist_Item_Class ViewItem::dldHistoryGenlistStyle;
Elm_Genlist_Item_Class ViewItem::dldGroupTitleGenlistStyle;

ViewItem::ViewItem(Item *item)
	: m_item(item)
	, m_glItem(NULL)
	, m_checkedBtn(NULL)
	, m_checked(EINA_FALSE)
	, m_isRetryCase(false)
	, m_isClickedFromNoti(false)
	, m_isGroupTitle(false)
	, m_insertAtFirst(false)
	, m_group(VIEWITEM_GROUP::TODAY)
{
	// FIXME need to makes exchange subject?? not yet, but keep it in mind!
	if (item) {
		m_aptr_observer = auto_ptr<Observer>(
			new Observer(updateCB, this, "viewItemObserver"));
		item->subscribe(m_aptr_observer.get());
	}

#ifdef _TIZEN_2_3_UX
	dldGenlistStyle.item_style = "dm/progress.main";
#else
	dldGenlistStyle.item_style = "3text.3icon.progress";
#endif
	dldGenlistStyle.func.text_get = getGenlistLabelCB;
	dldGenlistStyle.func.content_get = getGenlistIconCB;
	dldGenlistStyle.func.state_get = NULL;
	dldGenlistStyle.func.del = deleteGenlistData;
	dldGenlistStyle.decorate_all_item_style = "edit_default";

#ifdef _TIZEN_2_3_UX
	dldHistoryGenlistStyle.item_style = "type1";
#else
	dldHistoryGenlistStyle.item_style = "3text.1icon.1";
#endif
	dldHistoryGenlistStyle.func.text_get = getGenlistLabelCB;
	dldHistoryGenlistStyle.func.content_get = getGenlistIconCB;
	dldHistoryGenlistStyle.func.state_get = NULL;
	dldHistoryGenlistStyle.func.del = deleteGenlistData;
	dldHistoryGenlistStyle.decorate_all_item_style = "edit_default";

	dldGroupTitleGenlistStyle.item_style = "group_index";
	dldGroupTitleGenlistStyle.func.text_get = getGenlistLabelCB;
	dldGroupTitleGenlistStyle.func.content_get = NULL;
	dldGroupTitleGenlistStyle.func.state_get = NULL;
	dldGroupTitleGenlistStyle.func.del = deleteGenlistData;
	dldGroupTitleGenlistStyle.decorate_all_item_style = NULL;
}

ViewItem::ViewItem(VIEWITEM_GROUP::GROUP type)
	: m_item(NULL)
	, m_glItem(NULL)
	, m_checkedBtn(NULL)
	, m_checked(EINA_FALSE)
	, m_isRetryCase(false)
	, m_isClickedFromNoti(false)
	, m_isGroupTitle(true)
	, m_insertAtFirst(false)
	, m_group(type)
{
	DM_LOGD("");
}

ViewItem::~ViewItem()
{
	DM_LOGD("");
}

void ViewItem::create(Item *item)
{
	ViewItem *newViewItem = new ViewItem(item);
	newViewItem->setInsertAtFirst(true);
	DownloadView &view = DownloadView::getInstance();
	view.attachViewItem(newViewItem);
}

void ViewItem::destroy()
{
	DM_LOGD("");
	/* After item is destory,
	   view item also will be destroyed through event system */
	if (m_item) {
		m_item->destroy();
	}
}

void ViewItem::cancel()
{
	DM_LOGD("");
	if (m_item) {
		m_item->cancel();
	}
}

void ViewItem::updateCB(void *data)
{
	if (data)
		static_cast<ViewItem*>(data)->updateFromItem();
}

void ViewItem::updateFromItem()
{
	DownloadView &view = DownloadView::getInstance();
	ITEM::STATE state = getState();
	bool ItemClassUpdated = false;
	Elm_Object_Item *item = NULL;
	ViewItem *viewItem = NULL;
	DM_LOGV("state[%d]", state);
#ifdef _ENABLE_OMA_DOWNLOAD
	/* The OMA popup should be changed to layout style */
	if (state == ITEM::REQUEST_USER_CONFIRM) {
		view.showOMAPopup(m_item->getUserMessage(), this);
		return;
	}
#endif

	if (state == ITEM::DESTROY) {
		DM_LOGD("DESTROY");
		if (m_item)
			m_item->deSubscribe(m_aptr_observer.get());
		m_aptr_observer->clear();
		if (m_glItem == NULL) {
			DM_LOGE("Check NULL:m_glItem");
			return;
		}
		DM_LOGTEST("DEL:glItem[%p]viewItem[%p]",m_glItem, this);

		item = elm_genlist_item_next_get(m_glItem);
		if (item) {
			viewItem = (ViewItem *)elm_object_item_data_get(item);
			if (viewItem && viewItem->isGroupTitle()) {
				item = elm_genlist_item_prev_get(m_glItem);
				if (item) {
					viewItem = (ViewItem *)elm_object_item_data_get(item);
					if (viewItem && !(viewItem->isGroupTitle()))
						elm_object_item_signal_emit(item,
							"elm,state,bottomline,hide", "");
				}
			}
		}

		view.detachViewItem(this);
		return;
	}
	if (m_glItem == NULL) {
		DM_LOGE("Check NULL:m_glItem");
		return;
	}

	if (isFinishedWithErr() &&
			m_item->getErrorCode() == ERROR::NOT_ENOUGH_MEMORY) {
		string msg;
		string installDir = m_item->getInstallDir();
		if (installDir.compare(DM_DEFAULT_PHONE_INSTALL_DIR) == 0) {
			FILE *fp = NULL;
			fp = fopen(DM_DEFAULT_MMC_INSTALL_DIR, "r");
			if (fp != NULL) {
				msg = ERROR_POPUP_LOW_MEM_DEFAULT_PHONE_WITH_SD_CARD;
				fclose(fp);
			} else
				msg = ERROR_POPUP_LOW_MEM_DEFAULT_PHONE_WITHOUT_SD_CARD;
		} else if (installDir.compare(DM_DEFAULT_MMC_INSTALL_DIR) == 0) {
			FileUtility  fileObj;
			if (fileObj.checkAvailableMemory(0))
				msg = ERROR_POPUP_LOW_MEM_DEFAULT_MMC_WITH_PHONE_MEMORY_AVAILABLE;
			else
				msg = ERROR_POPUP_LOW_MEM_DEFAULT_MMC_WITH_NO_PHONE_MEMORY_AVAILABLE;
		} else {
			DM_LOGD("Can not enter here");
			return;
		}
		view.showMemoryFullPopup(msg);
	}

	DM_LOGTEST("Update:glItem[%p]viewItem[%p]",m_glItem, this);
	if (state == ITEM::SUSPENDED) {
		return;
	} else if (state == ITEM::DOWNLOADING) {
#ifdef _TIZEN_2_3_UX
		Evas_Object *progress = elm_object_item_part_content_get(m_glItem, "elm.swallow.end");
#else
		Evas_Object *progress = elm_object_item_part_content_get(m_glItem, "elm.swallow.progress");
#endif
		if (getFileSize() > 0 && progress) {
			double percentageProgress = 0.0;
			percentageProgress = (double)getReceivedFileSize() /
					(double)getFileSize();
			DM_LOGV("progress value[%.2f]",percentageProgress);
			elm_progressbar_value_set(progress, percentageProgress);
		}
#ifdef _TIZEN_2_3_UX
		elm_genlist_item_fields_update(m_glItem,"elm.swallow.end",
			ELM_GENLIST_ITEM_FIELD_TEXT);
#else
		elm_genlist_item_fields_update(m_glItem,"elm.text.2",
			ELM_GENLIST_ITEM_FIELD_TEXT);
#endif
	} else if (m_isRetryCase && state == ITEM::PREPARE_TO_RETRY) {
		elm_genlist_item_item_class_update(m_glItem, &dldGenlistStyle);
		ItemClassUpdated = true;
	} else if (!isFinished()) {
		elm_genlist_item_update(m_glItem);
	} else {/* finished state */
		/* If item is not in today group, move it to today group  */
		if (m_group != VIEWITEM_GROUP::TODAY) {
			/* 1. set item data NULL, to prevent deletion of Viewitem */
			elm_object_item_data_set(m_glItem, static_cast<void*>(NULL));
			/* 2. delete the item from group */
			view.detachViewItem(this);
			/* 3. set flag to insert the item at first Position */
			m_insertAtFirst = true;
			/* 4. Insert the item to DownloadView */
			view.attachViewItem(this);
		} else {
			elm_genlist_item_item_class_update(m_glItem, &dldHistoryGenlistStyle);
		}
		ItemClassUpdated = true;
		if (view.isGenlistEditMode()) {
			elm_object_item_disabled_set(m_glItem, EINA_FALSE);
			view.handleCheckedState();
		}
	}

	if (ItemClassUpdated) {
		item = elm_genlist_item_next_get(m_glItem);
		if (item)
			viewItem = (ViewItem *)elm_object_item_data_get(item);
		if (viewItem && viewItem->isGroupTitle()) {
			elm_object_item_signal_emit(m_glItem,
				"elm,state,bottomline,hide", "");
		}
	}
}

char *ViewItem::getGenlistLabelCB(void *data, Evas_Object *obj, const char *part)
{
	if(!data || !obj || !part)
		return NULL;

	ViewItem *item = static_cast<ViewItem *>(data);
	return item->getGenlistLabel(obj, part);
}

char *ViewItem::getGenlistLabel(Evas_Object *obj, const char *part)
{
	DM_LOGD("part[%s]", part);
#ifdef _TIZEN_2_3_UX
		if (strcmp(part, "elm.text.sub") == 0) {
			return (char *)getMessage();
		} else if (strcmp(part, "elm.text.sub.end") == 0) {
			if (isFinished()) {
				string outBuf;
				DateUtil &inst = DateUtil::getInstance();
				inst.getDateStr(getFinishedTime(), outBuf);
				return strdup(outBuf.c_str());
			}
		} else if (strcmp(part, "elm.text") == 0) {
			if (m_isGroupTitle)
				return (char *)getGroupTitle();
			else
				return strdup(getTitle());
		} else if (strcmp(part, "elm.swallow.end") == 0) {
			return (char *)getMessage();
		}
#else
	if (strncmp(part, "elm.text.1", strlen("elm.text.1")) == 0) {
		return strdup(getTitle());
	} else if (strncmp(part, "elm.text.2", strlen("elm.text.2")) == 0) {
		return (char *)getMessage();
	} else if (strncmp(part, "elm.text.3", strlen("elm.text.3")) == 0) {
		if (isFinished()) {
			string outBuf;
			DateUtil &inst = DateUtil::getInstance();
			inst.getDateStr(getFinishedTime(), outBuf);
			return strdup(outBuf.c_str());
		}
	} else if (strncmp(part, "elm.text", strlen("elm.text")) == 0) {
		return (char *)getGroupTitle();
	}
#endif
	return NULL;
}

Evas_Object *ViewItem::getGenlistIconCB(void *data, Evas_Object *obj,
	const char *part)
{
	if(!data || !obj || !part) {
		DM_LOGE("NULL Check:parameter");
		return NULL;
	}

	ViewItem *item = static_cast<ViewItem *>(data);
	return item->getGenlistIcon(obj, part);
}

Evas_Object *ViewItem::getGenlistIcon(Evas_Object *obj, const char *part)
{
	DM_LOGD("part[%s]state[%s]", part, stateStr());

#ifdef _TIZEN_2_3_UX
	DownloadView &view = DownloadView::getInstance();
    if (strcmp(part,"elm.swallow.icon") == 0) {
        Evas_Object *image = elm_image_add(obj);
        elm_image_file_set(image, getIconPath(), NULL);
        evas_object_size_hint_min_set(image, ELM_SCALE_SIZE(GENLIST_ICON_SIZE),
                                      ELM_SCALE_SIZE(GENLIST_ICON_SIZE));
        evas_object_show(image);
        return image;
	} else if (getState() < ITEM::FINISH_DOWNLOAD) {
		if (strcmp(part, "elm.swallow.icon") == 0)
			return createProgressBar(obj);
		else if (strcmp(part,"elm.swallow.icon.2") == 0)
			return createCancelBtn(obj);
	} else if (view.isGenlistEditMode() && strcmp(part,"elm.swallow.icon.2") == 0) {
		Evas_Object *check = elm_check_add(obj);
		elm_check_state_pointer_set(check, &m_checked);
		evas_object_repeat_events_set(check, EINA_FALSE);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_smart_callback_add(check, "changed", checkBoxChangedCB, this);
		m_checkedBtn = check;
        return check;
	}
	return NULL;
#else
	if (elm_genlist_decorate_mode_get(obj) && isFinished()) {
		if (strncmp(part,"elm.edit.icon.1", strlen("elm.edit.icon.1")) == 0) {
			Evas_Object *checkBtn = elm_check_add(obj);
			elm_object_style_set(checkBtn, "default/genlist");
			elm_check_state_pointer_set(checkBtn, &m_checked);
			evas_object_repeat_events_set(checkBtn, EINA_TRUE);
			evas_object_propagate_events_set(checkBtn, EINA_FALSE);
			elm_access_object_unregister(checkBtn);
			m_checkedBtn = checkBtn;
			return checkBtn;
		} else if (strncmp(part,"elm.edit.icon.2", strlen("elm.edit.icon.2")) ==
			0) {
			return NULL;
		}
	}
	/* elm.icon.2 should be checked prior to elm.icon */
	if (strncmp(part,"elm.icon.2", strlen("elm.icon.2")) == 0) {
		if (getState() == ITEM::RECEIVING_DOWNLOAD_INFO ||
			getState() == ITEM::DOWNLOADING ||
			isPreparingDownload())
			return createCancelBtn(obj);
		else
			return NULL;
	} else if (strncmp(part,"elm.icon.1", strlen("elm.icon.1")) == 0 ||
		strncmp(part, "elm.icon", strlen("elm.icon")) == 0) {
		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, getIconPath(), NULL);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		return icon;
	} else if (strcmp(part,"elm.swallow.progress") == 0) {
		return createProgressBar(obj);
	} else {
		DM_LOGE("Cannot enter here");
		return NULL;
	}
#endif
}

#ifdef _TIZEN_2_3_UX
void ViewItem::checkBoxChangedCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGD("");
	if (!obj || !data) {
		DM_LOGD("NULL check:parameter");
		return;
	}
	DownloadView &view = DownloadView::getInstance();
	ViewItem *item = (ViewItem *)data;
	item->setCheckedValue(elm_check_state_get(obj));
	view.handleCheckedState();
}
#endif

void ViewItem::deleteGenlistData(void *data, Evas_Object *obj)
{
	DM_LOGD("");
	ViewItem *item = static_cast<ViewItem*>(data);
	if (item)
		delete item;
}


void ViewItem::clickedCancelButton()
{
	DM_LOGV("");
	if (m_item)
		m_item->cancel();
}

void ViewItem::clickedCanceledRetryButton()
{
	DM_LOGV("");
	if (m_item && m_isClickedFromNoti) {
		m_item->deleteCompleteNoti();
	}
	m_isClickedFromNoti = false;
}

void ViewItem::clickedRetryButton()
{
	DM_LOGV("");
	retryViewItem();
	m_isClickedFromNoti = false;
}

void ViewItem::clickedGenlistItem()
{
	DownloadView &view = DownloadView::getInstance();
	DM_LOGD("");
	if (!m_item) {
		DM_LOGE("NULL Check:m_item");
		return;
	}
	if (m_glItem)
		elm_genlist_item_selected_set(m_glItem, EINA_FALSE);
	else
		DM_LOGE("Cannot enter here! genlist item cannot be NULL");

	if (view.isGenlistEditMode() && m_glItem) {
		Evas_Object *checkBox = NULL;
		Eina_Bool state;
#ifdef _TIZEN_2_3_UX
        checkBox = elm_object_item_part_content_get(m_glItem, "elm.swallow.icon.2");
#else
		checkBox = elm_object_item_part_content_get(m_glItem, "elm.edit.icon.1");
#endif
		if (checkBox) {
			state = elm_check_state_get(checkBox);
			m_checked = !state;
			elm_check_state_set(checkBox, m_checked);
		}
		view.handleCheckedState();
	} else if (getState() == ITEM::FINISH_DOWNLOAD) {
		if (m_item->isExistedFile()) {
            if (!m_item->play()) {
				string msg[2];
				msg[0] = __("IDS_DM_HEADER_UNABLE_TO_OPEN_FILE");
#ifdef _TIZEN_2_3_UX
				msg[1] = __("IDS_DM_BODY_UNABLE_TO_FIND_AN_APPLICATION_TO_OPEN_THIS_FILE");
#else
				msg[1] = S_("IDS_COM_BODY_NO_APPLICATIONS_CAN_PERFORM_THIS_ACTION");
#endif
				view.showErrPopup(msg);
			}
		} else {
			string msg[2];
			msg[0] = __("IDS_DM_HEADER_UNABLE_TO_OPEN_FILE");
#ifdef _TIZEN_2_3_UX
			msg[1] = __("IDS_DM_BODY_THIS_FILE_CANNOT_BE_FOUND_TAP_DOWNLOAD_TO_TRY_DOWNLOADING_IT_AGAIN");
#else
			msg[1] = __("IDS_DM_POP_FILE_DOES_NOT_EXIST_DOWNLOAD_AGAIN_Q");
#endif
			view.showRetryPopup(this, msg);
		}
	} else if (isFinishedWithErr()) {
		string msg[2];
		msg[0] = __("IDS_DM_BODY_DOWNLOAD_FAILED_M_STATUS_ABB");
#ifdef _TIZEN_2_3_UX
		msg[1] = __("IDS_DM_BODY_TAP_DOWNLOAD_TO_TRY_AGAIN");
#else
		msg[1] = __("IDS_DM_POP_FILE_NOT_RECEIVED_DOWNLOAD_AGAIN_Q");
#endif
		view.showRetryPopup(this, msg);
	}
}

Elm_Genlist_Item_Class *ViewItem::elmGenlistStyle()
{
	/* Change the genlist style class in case of download history item */
	if (m_isGroupTitle)
		return &dldGroupTitleGenlistStyle;
	if (getState() >= ITEM::FINISH_DOWNLOAD)
		return &dldHistoryGenlistStyle;
	else
		return &dldGenlistStyle;
}

const char *ViewItem::getGroupTitle()
{
	switch(m_group) {
		case VIEWITEM_GROUP::TODAY:
			return strdup(DM_BODY_TEXT_TODAY);
		case VIEWITEM_GROUP::YESTERDAY:
			return strdup(DM_BODY_TEXT_YESTERDAY);
		case VIEWITEM_GROUP::OLDER:
			return strdup(__("IDS_DM_HEADER_OLDER"));
		default:
			return NULL;
	}
}

const char *ViewItem::getMessage()
{
	DM_LOGV("state[%d]", getState());
	char *buff = NULL;
	switch(getState()) {
	case ITEM::IDLE:
	case ITEM::REQUESTING:
	case ITEM::QUEUED:
	case ITEM::PREPARE_TO_RETRY:
	case ITEM::RECEIVING_DOWNLOAD_INFO:
#ifdef _TIZEN_2_3_UX
		buff = strdup(__("IDS_DM_BODY_PREPARING_ING_M_STATUS_ABB"));
#else
		buff = strdup(__("IDS_DM_BODY_PREPARING_TO_DOWNLOAD_ING"));
#endif
		break;
	case ITEM::DOWNLOADING:
	case ITEM::SUSPENDED:
		getHumanFriendlyBytesStr(getReceivedFileSize(), true, &buff);
		break;
	case ITEM::CANCEL:
		buff = strdup(__("IDS_DM_BODY_DOWNLOAD_CANCELLED_M_STATUS_ABB"));
		break;
	case ITEM::FAIL_TO_DOWNLOAD:
		buff = strdup(__("IDS_DM_BODY_DOWNLOAD_FAILED_M_STATUS_ABB"));
		break;
	case ITEM::FINISH_DOWNLOAD:
		buff = strdup(senderName().c_str());
		break;
	case ITEM::REGISTERING_TO_SYSTEM:
		break;
#ifdef _ENABLE_OMA_DOWNLOAD
	case ITEM::NOTIFYING:
#ifdef _TIZEN_2_3_UX
		buff = strdup(__("IDS_DM_BODY_NOTIFYING_SERVER_ING"));
#else
		buff = strdup(__("IDS_DM_BODY_NOTIFYING_SERVER_ING_ABB"));
#endif
		break;
#endif
#ifdef _ENABLE_WAITING_RO
	case ITEM::WAITING_RO:
#ifdef _TIZEN_2_3_UX
		buff = strdup(__("IDS_DM_BODY_ACTIVATING_DRM_CONTENT_ING"));
#else
		buff = strdup(S_("IDS_COM_POP_ACTIVATING"));
#endif
		break;
#endif
	default:
		break;
	}
	return buff;
}

void ViewItem::getHumanFriendlyBytesStr(unsigned long long bytes,
	bool progressOption, char **buff)
{
	double doubleTypeBytes = 0.0;
	const char *unitStr[4] = {"B", "KB", "MB", "GB"};
	int unit = 0;
	unsigned long long unitBytes = bytes;

	/* using bit operation to avoid floating point arithmetic */
	for (unit = 0; (unitBytes > 1024 && unit < 4); unit++) {
		unitBytes = unitBytes >> 10;
	}

	unitBytes = 1 << (10*unit);
	doubleTypeBytes = ((double)bytes / (double)(unitBytes));
	// FIXME following code should be broken into another function, but leave it now to save function call time.s
	char str[64] = {0};

	if (unit > 3)
		unit = 3;

	if (progressOption && getFileSize() != 0) {
		/* using fixed point arithmetic to avoid floating point arithmetic */
		const int fixed_point = 6;
		unsigned long long receivedBytes = getReceivedFileSize() << fixed_point;
		unsigned long long result = (receivedBytes*100) / getFileSize();
		unsigned long long result_int = result >> fixed_point;
		unsigned long long result_fraction = result &
			~(0xFFFFFFFF << fixed_point);
		if (unit == 0)
			snprintf(str, sizeof(str), "%llu.%.2llu %%", result_int,
					result_fraction);
		else
			snprintf(str, sizeof(str), "%llu.%.2llu %%", result_int,
					result_fraction);
	} else {
		if (unit == 0)
			snprintf(str, sizeof(str), "%llu %s", bytes, unitStr[unit]);
		else
			snprintf(str, sizeof(str), "%.2f %s", doubleTypeBytes, unitStr[unit]);
	}
	str[63] = '\0';
	*buff = strdup(str);
	return;
}

unsigned long long ViewItem::getReceivedFileSize()
{
	if (m_item)
		return m_item->getReceivedFileSize();

	return 0;
}

unsigned long long ViewItem::getFileSize()
{
	if (m_item)
		return m_item->getFileSize();

	return 0;
}

const char *ViewItem::getTitle()
{
	if (!m_item ||m_item->getTitle().empty())
		return DM_BODY_TEXT_NO_NAME;
	return m_item->getTitle().c_str();
}

Evas_Object *ViewItem::createProgressBar(Evas_Object *parent)
{
	Evas_Object *progress = NULL;
	if (!parent) {
		DM_LOGE("NULL Check:parent");
		return NULL;
	}
	progress = elm_progressbar_add(parent);
	if (isFinished()) {
		DM_LOGE("Cannot enter here. finished item has othere genlist style");
		return NULL;
	}

	if (getFileSize() == 0 || isPreparingDownload()) {
		//DM_LOGI("Pending style::progressBar[%p]",progress);
#ifdef _TIZEN_2_3_UX
		elm_object_style_set(progress, "pending");
#else
		elm_object_style_set(progress, "pending_list");
#endif
		elm_progressbar_horizontal_set(progress, EINA_TRUE);
		evas_object_size_hint_align_set(progress, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(progress, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
		elm_progressbar_pulse(progress, EINA_TRUE);
	} else {
		//DM_LOGI("List style::progressBar[%p] fileSize[%d] state[%d]",progress, getFileSize(),getState());
#ifdef _TIZEN_2_3_UX
		elm_object_style_set(progress, "default");
#else
		elm_object_style_set(progress, "list_progress");
#endif
		elm_progressbar_horizontal_set(progress, EINA_TRUE);

		if (isCompletedDownload())
			elm_progressbar_value_set(progress, 1.0);
		/* When realized event is happened, the progress is created.
		   This is needed for that case */
		else if (getState() == ITEM::DOWNLOADING) {
			double percentageProgress = 0.0;
			percentageProgress = (double)(getReceivedFileSize()) /
				(double)(getFileSize());
			elm_progressbar_value_set(progress, percentageProgress);
		}
	}
	evas_object_show(progress);
	return progress;
}

Evas_Object *ViewItem::createCancelBtn(Evas_Object *parent)
{
	DM_LOGV ("");
#ifdef _TIZEN_2_3_UX
	Evas_Object *layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "list/C/type.1", "default");
	Evas_Object *button = elm_button_add(parent);
	elm_object_style_set(button, "dm_cancel_button");
	Evas_Object *icon = elm_image_add(parent);
	elm_image_file_set(icon, DM_CANCEL_BUTTON_ICON_PATH, NULL);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(button, "elm.swallow.icon", icon);
	elm_object_part_content_set(layout, "elm.swallow.end", button);
	evas_object_propagate_events_set(button, EINA_FALSE);
	evas_object_smart_callback_add(button,"clicked", cancelBtnClickedCB, this);
	return layout;
#else
	Evas_Object *button = elm_button_add(parent);
	elm_object_text_set(button, DM_SK_TEXT_CANCEL);
	evas_object_propagate_events_set(button, EINA_FALSE);
	evas_object_smart_callback_add(button,"clicked", cancelBtnClickedCB, this);
	return button;
#endif
	return NULL;
}

void ViewItem::cancelBtnClickedCB(void *data, Evas_Object *obj, void *event_info)
{
	DM_LOGV("");
	if (!data) {
		DM_LOGE("NULL Check:data");
		return;
	}
	ViewItem *viewItem = static_cast<ViewItem *>(data);
	viewItem->clickedCancelButton();
}


void ViewItem::retryViewItem(void)
{
	DM_LOGV("");
	if (m_item) {
		m_isRetryCase = true;
		m_item->clearForRetry();
		if (!m_item->retry()) {
			DownloadView &view = DownloadView::getInstance();
			string desc[2];
			desc[0] = __("IDS_DM_BODY_DOWNLOAD_FAILED_M_STATUS_ABB");
			desc[1] = DM_POP_TEXT_FAILED;
			view.showErrPopup(desc);
			m_item->deleteFromDB();
			m_item->destroy();
			return;
		}
	}
}

#ifdef _ENABLE_OMA_DOWNLOAD
void ViewItem::responseUserConfirm(bool res)
{
	m_item->handleUserConfirm(res);
}
#endif
