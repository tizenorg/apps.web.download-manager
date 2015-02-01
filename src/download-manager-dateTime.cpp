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
 * @file 	download-manager-dateTime.cpp
 * @author	Jungki Kwak (jungki.kwak@samsung.com)
 * @brief	data and utility APIs for Date and Time
 */

#include <string>
#include <stdlib.h>
#include "runtime_info.h"
#include "download-manager-dateTime.h"

#define MAX_SKELETON_BUFFER_LEN 16
#define MAX_PATTERN_BUFFER_LEN 64
#define MAX_STR_LEN 256
/* requested by capi-base-utils-i18n team */
#define DM_UDAT_PATTERN (I18N_UDATE_NONE - 1)


DateUtil::DateUtil()
	: dateShortFormat(NULL)
	, timeFormat12H(NULL)
	, timeFormat24H(NULL)
{
}

DateUtil::~DateUtil()
{
	DM_LOGD("");
	deinitLocaleData();
}

void DateUtil::deinitLocaleData()
{
	i18n_udate_destroy(dateShortFormat);
	i18n_udate_destroy(timeFormat12H);
	i18n_udate_destroy(timeFormat24H);
	dateShortFormat = NULL;
	timeFormat12H = NULL;
	timeFormat24H = NULL;
}

int DateUtil::getDiffDays(time_t nowTime,time_t refTime)
{
	int diffDays = 0;
	int nowYear = 0;
	int nowYday = 0;
	int refYday = 0;
	int refYear = 0;
	struct tm *nowDate = localtime(&nowTime);
	nowYday = nowDate->tm_yday;
	nowYear = nowDate->tm_year;
	struct tm *finishedDate = localtime(&refTime);
	refYday = finishedDate->tm_yday;
	refYear = finishedDate->tm_year;
	diffDays = nowYday - refYday;
	DM_LOGV("refDate[%d/%d/%d]refTime[%ld]yday[%d]",
		(finishedDate->tm_year + 1900), (finishedDate->tm_mon + 1),
		finishedDate->tm_mday, refTime, refYday);
	DM_LOGV("nowDate[%d/%d/%d]",
			(nowDate->tm_year + 1900), (nowDate->tm_mon + 1),
			nowDate->tm_mday);
	if ((nowYear-refYear)>0 && diffDays < 0) {
		int year = nowDate->tm_year;
		diffDays = diffDays + 365;
		/* leap year */
		if ((year%4 == 0 && year%100 != 0) || year%400 == 0)
			diffDays++;
	}
	DM_LOGD("diffDays[%d]",diffDays);
	return diffDays;
}

i18n_udate_format_h DateUtil::getBestPattern(const char *formatString,
		i18n_udatepg_h patternGenerator, const char *locale)
{
	int ret = I18N_ERROR_NONE;
	i18n_udate_format_h format = NULL;
	i18n_uchar uchCustomFormat[MAX_SKELETON_BUFFER_LEN] = {0,};
	i18n_uchar bestPattern[MAX_PATTERN_BUFFER_LEN] = {0,};
	int pattrenLength;
	int bestPatternLength;

	i18n_ustring_copy_ua(uchCustomFormat, formatString);
	pattrenLength = i18n_ustring_get_length(uchCustomFormat);

	// gets the best pattern that matches the given custom_format
	ret = i18n_udatepg_get_best_pattern(patternGenerator, uchCustomFormat,
		pattrenLength, bestPattern, MAX_PATTERN_BUFFER_LEN, &bestPatternLength);
	if (ret != I18N_ERROR_NONE) {
		DM_LOGE("failed to generate pattren, error [%d]", ret);
	}

	if (bestPatternLength < 1) {
		ret = i18n_udate_create(I18N_UDATE_SHORT, I18N_UDATE_NONE, locale, NULL, -1,
			NULL, -1, &format);
	} else {
		ret = i18n_udate_create((i18n_udate_format_style_e)DM_UDAT_PATTERN,
				(i18n_udate_format_style_e)DM_UDAT_PATTERN, locale, NULL, -1,
				bestPattern, -1, &format);
	}
	return format;
}

void DateUtil::updateLocale()
{
	DM_LOGD("");
	int ret = I18N_ERROR_NONE;
	const char *locale = NULL;
	i18n_udatepg_h patternGenerator = NULL;
	locale = getenv("LC_TIME");
	if (strlen(locale) > MAX_STR_LEN) {
		DM_LOGE("Size of locale is greater than MAX STR LEN");
		return;
	}
	DM_LOGD("locale : %s", locale);
	deinitLocaleData();
	// open a pattern generator according to a given locale
	ret = i18n_udatepg_create(locale, &patternGenerator);

	if(!patternGenerator) {
		DM_LOGE("i18n_udatpg_open fail errot %d", ret);
		return;
	}

	ret = i18n_ulocale_set_default(locale);
	if (ret != I18N_ERROR_NONE) {
		DM_LOGE("unable to set default locale error [%d]", ret);
	}

	ret = i18n_ulocale_get_default(&locale);
	if (ret != I18N_ERROR_NONE) {
		DM_LOGE("unable to get default locale error [%d]", ret);
	}

	timeFormat12H = getBestPattern("hh:mm", patternGenerator, locale);
	timeFormat24H = getBestPattern("HH:mm", patternGenerator, locale);
	dateShortFormat = getBestPattern("MMMd", patternGenerator, locale);
	i18n_udatepg_destroy(patternGenerator);
}

void DateUtil::getDateStr(double finishTime, string &outBuf)
{
	int ret = I18N_ERROR_NONE;
	bool value = false;
	int diffDay = 0;
	i18n_udate_format_h format = NULL;

	double nowTime = time(NULL);
	diffDay = getDiffDays((time_t)nowTime, (time_t)finishTime);
	if (diffDay == 0 || diffDay == 1) {
		if (runtime_info_get_value_bool(
				RUNTIME_INFO_KEY_24HOUR_CLOCK_FORMAT_ENABLED, &value) != 0) {
			DM_LOGE("Fail to get runtime_info_get_value_bool");
			format = timeFormat12H;
		} else {
			if (value)
				format = timeFormat24H;
			else
				format = timeFormat12H;
		}
	} else {
		format = dateShortFormat;
	}
	if (format) {
		int formattedLength;
		i18n_uchar formatted[MAX_BUF_LEN] = {0,};
		char result[MAX_BUF_LEN] = {0,};
		ret = i18n_udate_format_date(format, (finishTime * 1000), formatted,
				MAX_BUF_LEN, NULL, &formattedLength);
		if (ret != I18N_ERROR_NONE)
			DM_LOGE("i18n_udate_format_date failed : ret [%d]", ret);
		i18n_ustring_copy_au_n(result, formatted, MAX_BUF_LEN);
		result[MAX_BUF_LEN - 1] = '\0';
		outBuf = string(result);
	} else {
		DM_LOGE("Fail to get time value");
		outBuf = string(DM_POP_TEXT_ERROR);
	}
}

