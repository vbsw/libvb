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

static vb_err_t out_of_mem_err = { "out of memory (1)", VB_ERR_OUT_OF_MEMORY, VB_ERR_NONE };

vb_err_t *vb_err_new(const uint32_t num1, const uint32_t num2, const char *const str1, const char *const str2) {
	assert(num1);
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
	// string pattern: <str1> (num1; num2); str2
	const size_t err_size = sizeof(vb_err_t) + (str1 ? str1len + 1 : 0) + n1len + (num2 ? n2len + 2 : 0) + (str2 ? str2len + 2 : 0) + 3;
	vb_err_t *err = (vb_err_t*)malloc(err_size);
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
		err = &out_of_mem_err;
		err->num2 = num1;
	}
	return err;
}

vb_err_t *vb_err_oom_new(const uint32_t num2, const char *const str2) {
	return vb_err_new(VB_ERR_OUT_OF_MEMORY, num2, "out of memory", str2);
}

vb_err_t *vb_err_free(vb_err_t *const err) {
	if (err && err != &out_of_mem_err)
		free((void*)err);
	return NULL;
}
