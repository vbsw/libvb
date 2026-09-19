/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_TME
#define VBSW_VB_TME

#include <stdint.h>
#include <stdbool.h>
#include <vb/err.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	vb_err_t *err;
	int16_t year;
	int16_t millis;
	int8_t month;
	int8_t day;
	int8_t hour;
	int8_t minute;
	int8_t second;
} vb_tme_clock_t;

typedef struct {
	uint64_t start;
	int64_t nanos;
	int64_t millis;
	vb_err_t *err;
} vb_tme_mono_t;

bool vb_tme_clock_now     (vb_tme_clock_t *clock);
bool vb_tme_clock_now_utc (vb_tme_clock_t *clock);
bool vb_tme_mono_now      (vb_tme_mono_t *mono);
bool vb_tme_mono_reset    (vb_tme_mono_t *mono);

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_TME */
