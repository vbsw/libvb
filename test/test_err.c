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

static void test_new(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		vb_err_new(&err, 12, 23, "one", "two", VB_ERR_FFL);
		ASSERT(err)
		ASSERT(err->next_err == NULL)
		ASSERT(strcmp(err->str, "one (12; 23); two") == 0)
		ASSERT(strcmp(err->file_name, "test_err.c") == 0)
		ASSERT(strcmp(err->func_name, "test_new") == 0)
		ASSERT((err = vb_err_free(err)) == NULL)

		err = vb_err_new(&err, 12, 0, "one", "two", VB_ERR_FFL);
		ASSERT(err)
		ASSERT(err->next_err == NULL)
		ASSERT(strcmp(err->str, "one (12); two") == 0)
		ASSERT((err = vb_err_free(err)) == NULL)

		err = vb_err_new(&err, 12, 0, "", "two", VB_ERR_FFL);
		ASSERT(err)
		ASSERT(err->next_err == NULL)
		ASSERT(strcmp(err->str, "(12); two") == 0)
		ASSERT((err = vb_err_free(err)) == NULL)

		vb_err_new(&err, 12, 0, "", "", VB_ERR_FFL);
		ASSERT(err)
		ASSERT(err->next_err == NULL)
		ASSERT(strcmp(err->str, "(12)") == 0)
		ASSERT((err = vb_err_free(err)) == NULL)

		err = vb_err_new(&err, 12, 0, "one", "", VB_ERR_FFL);
		ASSERT(err)
		ASSERT(err->next_err == NULL)
		ASSERT(strcmp(err->str, "one (12)") == 0)
		ASSERT((err = vb_err_free(err)) == NULL)

		err = vb_err_new(&err, 12, 0, "", "two", VB_ERR_FFL);
		ASSERT(err)
		ASSERT(err->next_err == NULL)
		ASSERT(strcmp(err->str, "(12); two") == 0)
		ASSERT((err = vb_err_free(err)) == NULL)

		err = vb_err_new(&err, -12, -23, "two", "three", VB_ERR_FFL);
		ASSERT(err)
		ASSERT(err->next_err == NULL)
		ASSERT(strcmp(err->str, "two (-12; -23); three") == 0)
		ASSERT((err = vb_err_free(err)) == NULL)
	}
}

static void test_next(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err1 = NULL;
		err1 = vb_err_new(&err1, 12, 23, "one", "two", VB_ERR_FFL);
		ASSERT(err1)
		ASSERT(err1->next_err == NULL)
		ASSERT(strcmp(err1->str, "one (12; 23); two") == 0)

		vb_err_t *err2 = vb_err_new(&err1, 12, 0, "one", "two", VB_ERR_FFL);
		ASSERT(err1)
		ASSERT(err1->next_err)
		ASSERT(((vb_err_t*)err1->next_err) == err2)
		ASSERT(((vb_err_t*)err1->next_err)->next_err == NULL)
		ASSERT(strcmp(err2->str, "one (12); two") == 0)
		ASSERT(vb_err_free(err1) == NULL)
	}
}

void test_err(int *const err_line) {
	*err_line = 0;
	test_new(err_line);
	test_next(err_line);
}
