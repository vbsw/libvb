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

static vb_err_t oom_err = { "out of memory (1)",      VB_ERR_OUT_OF_MEMORY, VB_ERR_NONE };
static vb_err_t s1o_err = { "str1 size overflow (2)", VB_ERR_STR1_OVERFLOW, VB_ERR_NONE };
static vb_err_t s2o_err = { "str2 size overflow (3)", VB_ERR_STR2_OVERFLOW, VB_ERR_NONE };
static vb_err_t n2o_err = { "num2 size overflow (4)", VB_ERR_NUM2_OVERFLOW, VB_ERR_NONE };

vb_err_t *vb_err_asgf (vb_err_t **const err, vb_err_t *const other_err) {
	return err ? *err = other_err : vb_err_free(other_err);
}

vb_err_t *vb_err_new(vb_err_t **const err, const int64_t num1, const int64_t num2, const char *const str1, const char *const str2) {
	assert(num1);
	if (err) {
		vb_err_t *ret_val = NULL;
		int64_t n1 = num1 > 0 ? num1 : -num1, n2 = num2 > 0 ? num2 : -num2;
		char n1str[22], n2str[22];
		size_t n1len = 0, n2len = 0;
		const size_t str1len = (str1 ? strlen(str1) : 0);
		const size_t str2len = (str2 ? strlen(str2) : 0);
		do {
			n1str[n1len++] = '0' + (n1%10);
			n1 /= 10;
		} while (n1 != 0);
		if (num1 < 0)
			n1str[n1len++] = '-';
		if (n2 != 0) {
			do {
				n2str[n2len++] = '0' + (n2%10);
				n2 /= 10;
			} while (n2 != 0);
			if (num2 < 0)
				n2str[n2len++] = '-';
		}
		// string pattern: str1 (num1; num2); str2
		size_t err_size = sizeof(vb_err_t) + n1len + (num2 != 0 ? n2len + 2 : 0) + 3; // 2 = "; ", 3 = "()\0"
		if (str1len > 0) {
			if (err_size + 3 <= SIZE_MAX - str1len) {
				err_size += (str1len + 1); // 1 = " "
				if (str2len > 0) {
					if (err_size + 2 <= SIZE_MAX - str1len) {
						err_size += (str1len + 2); // 2 = "; "
					} else {
						ret_val = &s2o_err;
						ret_val->num2 = num1;
					}
				}
			} else {
				ret_val = &s1o_err;
				ret_val->num2 = num1;
			}
		} else if (str2len > 0) {
			if (err_size + 2 <= SIZE_MAX - str1len) {
				err_size += (str1len + 2); // 2 = "; "
			} else {
				ret_val = &s2o_err;
				ret_val->num2 = num1;
			}
		}
		if (ret_val == NULL) {
			ret_val = malloc(err_size);
			if (ret_val) {
				char *const data = (char*)(&ret_val[1]);
				size_t offset = 0;
				if (str1len) {
					memcpy(data, str1, str1len);
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
					memcpy(&data[offset], str2, str2len);
					offset += str2len;
				}
				data[offset] = '\0';
				ret_val->str = data;
				ret_val->num1 = num1;
				ret_val->num2 = num2;
			} else {
				ret_val = &oom_err;
				ret_val->num2 = num1;
			}
		}
		*err = ret_val;
		return ret_val;
	}
	return NULL;
}

vb_err_t *vb_err_new_oom(vb_err_t **const err, const int64_t num1, const int64_t num2, const char *const str2) {
	return vb_err_new(err, num1, num2, "out of memory", str2);
}

vb_err_t *vb_err_free(vb_err_t *const err) {
	if (err && err != &oom_err && err != &s1o_err && err != &s2o_err && err != &n2o_err)
		free(err);
	return NULL;
}
