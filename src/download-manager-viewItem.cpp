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
#include "download-manager-viewItem.h"
#include "download-manager-items.h"
#include "download-manager-view.h"

ViewItem::ViewItem(Item *item)
	: m_item(item)
	, m_glItem(NULL)
	, m_progressBar(NULL)
	, m_checkedBtn(NULL)
	, m_checked(EINA_FALSE)
	, m_isRetryCase(false)
{
	// FIXME need to makes exchange subject?? not yet, but keep it in mind!
	if (item) {
		m_aptr_observer = auto_ptr<Observer>(
			new Observer(updateCB, this, "viewItemObserver"));
		item->subscribe(m_aptr_observer.get());
	}

	dldGenlistStyle.item_style = "3text.3icon.progress";
	dldGenlistStyle.func.text_get = getGenlistLabelCB;
	dldGenlistStyle.func.content_get = getGenlistIconCB;
	dldGenlistStyle.func.state_get = NULL;
	dldGenlistStyle.func.del = NULL;
	dldGenlistStyle.decorate_all_item_style = "edit_default";

	dldHistoryGenlistStyle.item_style = "3text.1icon.2";
	dldHistoryGenlistStyle.func.text_get = getGenlistLabelCB;
	dldHistoryGenlistStyle.func.content_get = getGenlistIconCB;
	dldHistoryGenlistStyle.func.state_get = NULL;
	dldHistoryGenlistStyle.func.del = NULL;
	dldHistoryGenlistStyle.decorate_all_item_style = "edit_default";
}

ViewItem::~ViewItem()
{
	DP_LOGD_FUNC();
}

void ViewItem::create(Item *item)
{
	ViewItem *newViewItem = new ViewItem(item);

	DownloadView &view = DownloadView::getInstance();
	view.attachViewItem(newViewItem);
}

void ViewItem::destroy()
{
	DP_LOGD("ViewItem::destroy");
	/* After item is destory,
	   view item also will be destroyed through event system */
	if (m_item) {
		m_item->destroy();
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
	DP_LOGV("ViewItem::updateFromItem() ITEM::[%d]", state());
#ifdef _ENABLE_OMA_DOWNLOAD
	/* The OMA popup should be changed to layout style */
	if (state() == ITEM::REQUEST_USER_CONFIRM) {
		view.showOMAPopup(m_item->getUserMessage(), this);
		return;
	}
#endif

	if (state() == ITEM::DESTROY) {
		DP_LOGD("ViewItem::updateFromItem() ITEM::DESTROY");
		if (m_item)
			m_item->deSubscribe(m_aptr_observer.get());
		m_aptr_observer->clear();
		elm_object_item_del(m_glItem);
		m_glItem = NULL;
		view.detachViewItem(this);
		return;
	}
	if (m_glItem == NULL) {
		return;
	}
	if (state() == ITEM::SUSPENDED) {
		return;
	} else if (state() == ITEM::DOWNLOADING) {
		if (fileSize() > 0 && m_progressBar) {
			double percentageProgress = 0.0;
			percentageProgress = (double)(receivedFileSize()) /
				(double)(fileSize());
			DP_LOGV("progress value[%.2f]",percentageProgress);
			elm_progressbar_value_set(m_progressBar, percentageProgress);
		}
		elm_genlist_item_fields_update(m_glItem,"elm.text.2",
			ELM_GENLIST_ITEM_FIELD_TEXT);
	} else if (m_isRetryCase && state() == ITEM::PREPARE_TO_RETRY) {
		elm_genlist_item_item_class_update(m_glItem, &dldGenlistStyle);
	} else if (!isFinished()) {
		elm_genlist_item_update(m_glItem);
	} else {/* finished state */
		elm_genlist_item_item_class_update(m_glItem, &dldHistoryGenlistStyle);
		if (view.isGenlistEditMode())
			elm_object_item_disabled_set(m_glItem, EINA_FALSE);
	}
}

char *ViewItem::getGenlistLabelCB(void *data, Evas_Object *obj, const char *part)
{
//	DP_LOGV_FUNC();

	if(!data || !obj || !part)
		return NULL;

	ViewItem *item = static_cast<ViewItem *>(data);
	return item->getGenlistLabel(obj, part);
}

char *ViewItem::getGenlistLabel(Evas_Object *obj, 	const char *part)
{
	DP_LOGV("ViewItem::getListLabel:part[%s]", part);

	if (strncmp(part, "elm.text.1", strlen("elm.text.1")) == 0) {
		return strdup(getTitle());
	} else if (strncmp(part, "elm.text.2", strlen("elm.text.2")) == 0) {
		return (char *)getMessage();
	} else if (strncmp(part, "elm.text.3", strlen("elm.text.3")) == 0) {
		if (!isFinished()) {
			return NULL;
		} else {
			string outBuf;
			DateUtil &inst = DateUtil::getInstance();
			inst.getDateStr(finishedTime(), outBuf);
			return strdup(outBuf.c_str());
		}
	} else {
		DP_LOGD("No Implementation");
		return NULL;
	}
}

Evas_Object *ViewItem::getGenlistIconCB(void *data, Evas_Object *obj,
	const char *part)
{
//	DP_LOGV_FUNC();
	if(!data || !obj || !part) {
		DP_LOGE("parameter is NULL");
		return NULL;
	}

	ViewItem *item = static_cast<ViewItem *>(data);
	return item->getGenlistIcon(obj, part);
}

Evas_Object *ViewItem::getGenlistIcon(Evas_Object *obj, const char *part)
{
	//DP_LOGD("ViewItem::getGenlistIcon:part[%s]state[%s]", part, stateStr());

	if (elm_genlist_decorate_mode_get(obj) && isFinished()) {
		if (strncmp(part,"elm.edit.icon.1", strlen("elm.edit.icon.1")) == 0) {
			Evas_Object *checkBtn = elm_check_add(obj);
			elm_check_state_pointer_set(checkBtn, &m_checked);
			evas_object_smart_callback_add(checkBtn, "changed", checkChangedCB,
				this);
			m_checkedBtn = checkBtn;
			return checkBtn;
		} else if (strncmp(part,"elm.edit.icon.2", strlen("elm.edit.icon.2")) ==
			0) {
			return NULL;
		}

	}
	/* elm.icon.2 should be checked prior to elm.icon */
	if (strncmp(part,"elm.icon.2", strlen("elm.icon.2")) == 0) {
		if (state() == ITEM::RECEIVING_DOWNLOAD_INFO ||
			state() == ITEM::DOWNLOADING ||
			isPreparingDownload())
			return createCancelBtn(obj);
		else
			return NULL;
	} else if (strncmp(part,"elm.icon.1", strlen("elm.icon.1")) == 0 ||
		strncmp(part, "elm.icon", strlen("elm.icon")) == 0) {
		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, getIconPath(), NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL,1,1);
		return icon;
	} else if (strcmp(part,"elm.swallow.progress") == 0) {
		return createProgressBar(obj);
	} else {
		DP_LOGE("Cannot enter here");
		return NULL;
	}
}

void ViewItem::checkChangedCB(void *data, Evas_Object *obj,
	void *event_info)
{
	DownloadView &view = DownloadView::getInstance();
	DP_LOGV_FUNC();
	//ViewItem *item = static_cast<ViewItem *>(data);
	//DP_LOGD("checked[%d] viewItem[%p]",(bool)(item->checkedValue()),item);
	view.handleCheckedState();
}

void ViewItem::clickedCancelButton()
{
	DP_LOGD("ViewItem::clickedCancelButton()");
	requestCancel();
}

void ViewItem::requestCancel()
{
	if (m_item) {
		m_item->cancel();
	}
}

void ViewItem::clickedRetryButton()
{
	DP_LOGV_FUNC();
	retryViewItem();
}

void ViewItem::clickedGenlistItem()
{
	DownloadView &view = DownloadView::getInstance();
	Elm_Object_Item *itemObject = NULL;
	DP_LOGD_FUNC();
	if (!m_item) {
		DP_LOGE("m_item is NULL");
		return;
	}
	itemObject = genlistItem();
	if (itemObject)
		elm_genlist_item_selected_set(itemObject, EINA_FALSE);
	else
		DP_LOGE("Cannot enter here! genlist item cannot be NULL");

	if (view.isGenlistEditMode()) {
		m_checked = !m_checked;
		if (m_checkedBtn && itemObject)
			elm_genlist_item_fields_update(itemObject,"elm.edit.icon.1",
				ELM_GENLIST_ITEM_FIELD_CONTENT);
		else
			DP_LOGE("m_checkedBtn is NULL");
		view.handleCheckedState();
	} else if (state() == ITEM::FINISH_DOWNLOAD) {
		if (m_item->isExistedFile()) {
			bool ret = m_item->play();
			if (ret) {
				m_item->deleteFromDB();
				destroy();
			} else {
				string msg = S_("IDS_COM_BODY_NO_APPLICATIONS_CAN_PERFORM_THIS_ACTION");
				view.showErrPopup(msg);
			}
		} else {
			string msg = __("IDS_DM_POP_FILE_DOES_NOT_EXIST_DOWNLOAD_AGAIN_Q");
			view.showRetryPopup(this, msg);
		}
	} else if (isFinishedWithErr()) {
		string msg = __("IDS_DM_POP_FILE_NOT_RECEIVED_DOWNLOAD_AGAIN_Q");
		view.showRetryPopup(this, msg);
	}
}

Elm_Genlist_Item_Class *ViewItem::elmGenlistStyle()
{
	/* Change the genlist style class in case of download history item */
	if (state() >= ITEM::FINISH_DOWNLOAD)
		return &dldHistoryGenlistStyle;
	else
		return &dldGenlistStyle;
}

const char *ViewItem::getMessage()
{
	DP_LOGV("ViewItem state() ITEM::[%d]", state());
	const char *buff = NULL;
	switch(state()) {
	case ITEM::IDLE:
	case ITEM::REQUESTING:
	case ITEM::PREPARE_TO_RETRY:
	case ITEM::RECEIVING_DOWNLOAD_INFO:
	/* Do not display string and show only the progress bar */
//		buff = __("Check for download");
		break;
	case ITEM::DOWNLOADING:
	case ITEM::SUSPENDED:
		buff = getHumanFriendlyBytesStr(receivedFileSize(), true);
		//DP_LOGD("%s", buff);
		break;
	case ITEM::CANCEL:
	case ITEM::FAIL_TO_DOWNLOAD:
	case ITEM::FINISH_DOWNLOAD:
		buff = senderName().c_str();
		break;
	case ITEM::REGISTERING_TO_SYSTEM:
		break;
#ifdef _ENABLE_OMA_DOWNLOAD
	case ITEM::NOTIFYING:
		buff = __("IDS_BR_BODY_NOTIFYING_ING");
		break;
#endif
	default:
		break;
	}
	if (buff)
		return strdup(buff);
	else
		return buff;
}

const char *ViewItem::getHumanFriendlyBytesStr(unsigned long int bytes,
	bool progressOption)
{
	double doubleTypeBytes = 0.0;
	const char *unitStr[4] = {"B", "KB", "MB", "GB"};
	int unit = 0;
	unsigned long int unitBytes = bytes;

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

	if (progressOption && fileSize() != 0) {
		/* using fixed point arithmetic to avoid floating point arithmetic */
		const int fixed_point = 6;
		unsigned long long int receivedBytes = receivedFileSize() << fixed_point;
		unsigned long long int result = (receivedBytes*100) / fileSize();
		unsigned long long int result_int = result >> fixed_point;
		unsigned long long int result_fraction = result &
			~(0xFFFFFFFF << fixed_point);
		if (unit == 0)
			snprintf(str, sizeof(str), "%lu %s / %llu.%.2llu %%",
				bytes, unitStr[unit], result_int, result_fraction);
		else
			snprintf(str, sizeof(str), "%.2f %s / %llu.%.2llu %%",
				doubleTypeBytes, unitStr[unit], result_int, result_fraction);
	} else {
		if (unit == 0)
			snprintf(str, sizeof(str), "%lu %s", bytes, unitStr[unit]);
		else
			snprintf(str, sizeof(str), "%.2f %s", doubleTypeBytes, unitStr[unit]);
	}
	str[63] = '\0';
	string temp = string(str);
	return temp.c_str();
}

unsigned long int ViewItem::receivedFileSize()
{
	if (m_item)
		return m_item->receivedFileSize();

	return 0;
}

unsigned long int ViewItem::fileSize()
{
	if (m_item)
		return m_item->fileSize();

	return 0;
}

const char *ViewItem::getTitle()
{
	if (!m_item)
		return S_("IDS_COM_BODY_NO_NAME");
	if (m_item->title().empty())
		return S_("IDS_COM_BODY_NO_NAME");
	return m_item->title().c_str();
}

Evas_Object *ViewItem::createProgressBar(Evas_Object *parent)
{
	Evas_Object *progress = NULL;
	if (!parent) {
		DP_LOGE("parent is NULL");
		return NULL;
	}
	progress = elm_progressbar_add(parent);
	setProgressBar(progress);
	if (isFinished()) {
		DP_LOGE("Cannot enter here. finished item has othere genlist style");
		return NULL;
	}

	if (fileSize() == 0 || isPreparingDownload()) {
		//DP_LOGD("Pending style::progressBar[%p]",progress);
		elm_object_style_set(progress, "pending_list");
		elm_progressbar_horizontal_set(progress, EINA_TRUE);
		evas_object_size_hint_align_set(progress, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(progress, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
		elm_progressbar_pulse(progress, EINA_TRUE);
	} else {
		//DP_LOGD("List style::progressBar[%p] fileSize[%d] state[%d]",progress, fileSize(),state());
		elm_object_style_set(progress, "list_progress");
		elm_progressbar_horizontal_set(progress, EINA_TRUE);

		if (isCompletedDownload())
			elm_progressbar_value_set(progress, 1.0);
		/* When realized event is happened, the progress is created.
		   This is needed for that case */
		else if (state() == ITEM::DOWNLOADING) {
			double percentageProgress = 0.0;
			percentageProgress = (double)(receivedFileSize()) /
				(double)(fileSize());
			elm_progressbar_value_set(progress, percentageProgress);
		}
	}
	evas_object_show(progress);
	return progress;
}

void ViewItem::updateCheckedBtn()
{
	if (m_checkedBtn)
		elm_check_state_pointer_set(m_checkedBtn,&m_checked);
}

Evas_Object *ViewItem::createCancelBtn(Evas_Object *parent)
{
	DP_LOGV_FUNC();
	Evas_Object *button = elm_button_add(parent);
	elm_object_part_content_set(parent, "btn_style1", button);
	elm_object_style_set(button, "style1/auto_expand");
	evas_object_size_hint_aspect_set(button, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_object_text_set(button, S_("IDS_COM_SK_CANCEL"));
	evas_object_propagate_events_set(button, EINA_FALSE);
	evas_object_smart_callback_add(button,"clicked", cancelBtnClickedCB, this);
	return button;
}

void ViewItem::cancelBtnClickedCB(void *data, Evas_Object *obj, void *event_info)
{
	DP_LOGV_FUNC();
	if (!data) {
		DP_LOGE("data is NULL");
		return;
	}
	ViewItem *viewItem = static_cast<ViewItem *>(data);
	viewItem->clickedCancelButton();
}


void ViewItem::retryViewItem(void)
{
	DownloadView &view = DownloadView::getInstance();
	DP_LOGV_FUNC();
	if (m_item) {
		m_isRetryCase = true;
		m_item->clearForRetry();
		if (!m_item->retry()) {
			DownloadView &view = DownloadView::getInstance();
			string desc = S_("IDS_COM_POP_FAILED");
			view.showErrPopup(desc);
			m_item->deleteFromDB();
			m_item->destroy();
			return;
		}
		/* Move a item to Today group, if it is not included to Today group */
		view.moveRetryItem(this);
	}
}

#ifdef _ENABLE_OMA_DOWNLOAD
void ViewItem::responseUserConfirm(bool res)
{
	m_item->handleUserConfirm(res);
}
#endif
