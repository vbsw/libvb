/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <vb/err.h>

static vb_err_t oom_err = { "out of memory (1)", VB_ERR_OUT_OF_MEMORY, VB_ERR_NONE };
static vb_err_t s1o_err = { "str1 size overflow (2)", VB_ERR_STR1_OVERFLOW, VB_ERR_NONE };
static vb_err_t s2o_err = { "str2 size overflow (3)", VB_ERR_STR2_OVERFLOW, VB_ERR_NONE };
static vb_err_t n2o_err = { "num2 size overflow (4)", VB_ERR_NUM2_OVERFLOW, VB_ERR_NONE };

vb_err_t *vb_err_new(const uint32_t num1, const uint32_t num2, const char *const str1, const char *const str2) {
	assert(num1);
	vb_err_t *err = NULL;
	uint32_t n1 = num1, n2 = num2;
	char n1str[10], n2str[10];
	size_t n1len = 0, n2len = 0;
	const size_t str1len = (str1 ? strlen(str1) : 0);
	const size_t str2len = (str2 ? strlen(str2) : 0);
	size_t offset = 0;
	do {
		n1str[n1len++] = '0' + (n1%10);
		n1 /= 10;
	} while (n1 > 0);
	if (n2 > 0) {
		do {
			n2str[n2len++] = '0' + (n2%10);
			n2 /= 10;
		} while (n2 > 0);
	}
	// string pattern: str1 (num1; num2); str2
	size_t err_size = sizeof(vb_err_t) + n1len + (num2 > 0 ? n2len + 2 : 0) + 3; // 2 = "; ", 3 = "()\0"
	if (str1len > 0) {
		if (err_size + 3 <= SIZE_MAX - str1len) {
			err_size += (str1len + 1); // 1 = " "
			if (str2len > 0) {
				if (err_size + 2 <= SIZE_MAX - str1len) {
					err_size += (str1len + 2); // 2 = "; "
				} else {
					err = &s2o_err;
					err->num2 = num1;
				}
			}
		} else {
			err = &s1o_err;
			err->num2 = num1;
		}
	} else if (str2len > 0) {
		if (err_size + 2 <= SIZE_MAX - str1len) {
			err_size += (str1len + 2); // 2 = "; "
		} else {
			err = &s2o_err;
			err->num2 = num1;
		}
	}
	if (err == NULL) {
		err = (vb_err_t*)malloc(err_size);
		if (err) {
			char *const data = (char*)(&err[1]);
			if (str1len) {
				memcpy((void*)data, (const void*)str1, str1len);
				data[str1len] = ' ';
				offset = str1len+1;
			}
			data[offset++] = '(';
			for (int i = 0; i < n1len; i++)
				data[offset+i] = n1str[n1len-1-i];
			offset += n1len;
			if (num2) {
				data[offset++] = ';';
				data[offset++] = ' ';
				for (int i = 0; i < n2len; i++)
					data[offset+i] = n2str[n2len-1-i];
				offset += n2len;
			}
			data[offset++] = ')';
			if (str2len) {
				data[offset++] = ';';
				data[offset++] = ' ';
				memcpy((void*)&data[offset], (const void*)str2, str2len);
				offset += str2len;
			}
			data[offset] = '\0';
			err->str = data;
			err->num1 = num1;
			err->num2 = num2;
		} else {
			err = &oom_err;
			err->num2 = num1;
		}
	}
	return err;
}

vb_err_t *vb_err_new_oom(const uint32_t num1, const uint32_t num2, const char *const str2) {
	return vb_err_new(num1, num2, "out of memory", str2);
}

vb_err_t *vb_err_free(vb_err_t *const err) {
	if (err && err != &oom_err && err != &s1o_err && err != &s2o_err && err != &n2o_err)
		free((void*)err);
	return NULL;
}
