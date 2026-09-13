/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <string.h>
#include <vb/err.h>

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

void test_err(int *const err_line) {
	*err_line = 0;
	vb_err_t *err = vb_err_new(10, 20, "one", "two");
	ASSERT(err)
	ASSERT(strcmp(err->str, "one (10; 20); two") == 0)
	ASSERT(vb_err_free(err) == NULL)

	err = vb_err_new(10, 0, "one", "two");
	ASSERT(strcmp(err->str, "one (10); two") == 0)
	ASSERT(vb_err_free(err) == NULL)

	err = vb_err_new(10, 0, "", "two");
	ASSERT(strcmp(err->str, "(10); two") == 0)
	ASSERT(vb_err_free(err) == NULL)

	err = vb_err_new(10, 0, "", "");
	ASSERT(strcmp(err->str, "(10)") == 0)
	ASSERT(vb_err_free(err) == NULL)

	err = vb_err_new(10, 0, "one", "");
	ASSERT(strcmp(err->str, "one (10)") == 0)
	ASSERT(vb_err_free(err) == NULL)

	err = vb_err_new(10, 0, "", "two");
	ASSERT(strcmp(err->str, "(10); two") == 0)
	ASSERT(vb_err_free(err) == NULL)
}
