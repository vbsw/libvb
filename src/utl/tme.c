/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <assert.h>
#include <time.h>
#include <vb/tme.h>

bool vb_tme_clock_now(vb_tme_clock_t *const clock) {
	assert(clock);
	bool ret_val = false;
	if (clock->err == NULL) {
		struct timespec ts;
		if (!clock_gettime(CLOCK_REALTIME, &ts)) {
			struct tm local;
			localtime_r(&ts.tv_sec, &local);
			clock->year = (int16_t)local.tm_year + 1900;
			clock->month = (int8_t)local.tm_mon + 1;
			clock->day = (int8_t)local.tm_mday;
			clock->hour = (int8_t)local.tm_hour;
			clock->minute = (int8_t)local.tm_min;
			clock->second = (int8_t)local.tm_sec;
			clock->millis = (int16_t)(ts.tv_nsec / 1000000);
			ret_val = true;
		} else {
			clock->err = vb_err_new(VB_ERR_TME_CLOCK, 2, "clock now failed", NULL);
		}
	}
	return ret_val;
}

bool vb_tme_clock_now_utc(vb_tme_clock_t *const clock) {
	assert(clock);
	bool ret_val = false;
	if (clock->err == NULL) {
		struct timespec ts;
		if (!clock_gettime(CLOCK_REALTIME, &ts)) {
			struct tm utc;
			gmtime_r(&ts.tv_sec, &utc);
			clock->year = (int16_t)utc.tm_year + 1900;
			clock->month = (int8_t)utc.tm_mon + 1;
			clock->day = (int8_t)utc.tm_mday;
			clock->hour = (int8_t)utc.tm_hour;
			clock->minute = (int8_t)utc.tm_min;
			clock->second = (int8_t)utc.tm_sec;
			clock->millis = (int16_t)(ts.tv_nsec / 1000000);
			ret_val = true;
		} else {
			clock->err = vb_err_new(VB_ERR_TME_CLOCK, 1, "clock UTC now failed", NULL);
		}
	}
	return ret_val;
}

bool vb_tme_mono_now(vb_tme_mono_t *const mono) {
	assert(mono);
	assert(mono->start);
	bool ret_val = false;
	if (mono->err == NULL) {
		struct timespec ts;
		if (!clock_gettime(CLOCK_MONOTONIC, &ts)) {
			const uint64_t now = (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
			mono->nanos = mono->start - now;
			mono->millis = mono->nanos / 1000000;
			ret_val = true;
		} else {
			mono->err = vb_err_new(VB_ERR_TME_MONO, 2, "mono time now failed", NULL);
		}
	}
	return ret_val;
}

bool vb_tme_mono_reset(vb_tme_mono_t *const mono) {
	assert(mono);
	bool ret_val = false;
	if (mono->err == NULL) {
		struct timespec ts;
		if (!clock_gettime(CLOCK_MONOTONIC, &ts)) {
			mono->start = (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
			mono->nanos = 0;
			mono->millis = 0;
			ret_val = true;
		} else {
			mono->err = vb_err_new(VB_ERR_TME_MONO, 1, "mono time reset failed", NULL);
		}
	}
	return ret_val;
}

