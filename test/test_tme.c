
/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <vb/tme.h>

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

static void test_clock_now(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		vb_tme_clock_t clock = {.err = &err};

		// local time
		ASSERT(vb_tme_clock_now(&clock))
		ASSERT(err == NULL)
		ASSERT(clock.err == &err)
		ASSERT(clock.year > 2000)
		ASSERT(clock.month > 0)
		ASSERT(clock.day > 0)
		ASSERT(clock.hour > 0)
		ASSERT(clock.minute >= 0)
		ASSERT(clock.second >= 0)
		ASSERT(clock.millis >= 0 && clock.millis < 1000)
	}
}

static void test_clock_now_utc(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		vb_tme_clock_t clock = {.err = &err};

		// UTC time
		ASSERT(vb_tme_clock_now_utc(&clock))
		ASSERT(err == NULL)
		ASSERT(clock.err == &err)
		ASSERT(clock.year > 2000)
		ASSERT(clock.month > 0)
		ASSERT(clock.day > 0)
		ASSERT(clock.hour > 0)
		ASSERT(clock.minute >= 0)
		ASSERT(clock.second >= 0)
		ASSERT(clock.millis >= 0 && clock.millis < 1000)
	}
}

static void test_mono_reset_now(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		vb_tme_mono_t mono = {.err = &err};

		// reset mono time
		ASSERT(vb_tme_mono_reset(&mono))
		ASSERT(err == NULL)
		ASSERT(mono.err == &err)
		ASSERT(mono.start > (int64_t)UINT32_MAX)
		const int64_t prev_start = mono.start;

		// update
		ASSERT(vb_tme_mono_now(&mono))
		ASSERT(err == NULL)
		ASSERT(mono.err == &err)
		ASSERT(mono.start == prev_start)
		ASSERT(mono.nanos < (int64_t)INT32_MAX)
		ASSERT(mono.millis < (int64_t)INT32_MAX)
	}
}

void test_tme(int *const err_line) {
	*err_line = 0;
	test_clock_now(err_line);
	test_clock_now_utc(err_line);
	test_mono_reset_now(err_line);
}
