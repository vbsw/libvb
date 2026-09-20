
/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <limits.h>
#include <vb/fle.h>

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

static void test_stats(int *const err_line, vb_fle_t *const exe_path) {
	if (*err_line == 0) {
		vb_fle_t file = {.err = NULL};

		ASSERT(vb_fle_set_base_name(exe_path, "testfile.txt"))
		ASSERT(vb_fle_stats(&file, exe_path->buf->data))
	}
}

void test_fle(int *const err_line) {
	*err_line = 0;
	vb_fle_t exe_path = {.err = NULL};

	ASSERT(vb_fle_alloc_path_buffer(&exe_path, NULL))
	ASSERT(exe_path.buf->data)
	ASSERT(exe_path.buf->len > 0)
	ASSERT(exe_path.buf->cap > 0)
	ASSERT(vb_fle_binary_path(&exe_path))

	test_stats(err_line, &exe_path);
}
