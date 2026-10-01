/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <vb/err.h>

static vb_err_t oom_err = { NULL, "out of memory (1)", "err.c", "vb_err_new", VB_ERR_OUT_OF_MEMORY, VB_ERR_NONE, 0, 5, 10 };

vb_err_t *vb_err_asgf (vb_err_t **const err, vb_err_t *const other_err) {
	return err ? *err = other_err : vb_err_free(other_err);
}

vb_err_t *vb_err_new(vb_err_t **const err, const int64_t num1, const int64_t num2, const char *const str1, const char *const str2, const char *const file_path, const char *const func_name, const size_t line, const size_t len1, const size_t len2) {
	assert(num1);
	assert(file_path);
	assert(func_name);
	assert(line > 0 && line <= INT32_MAX);
	assert(len1 > 1 && len1 <= PATH_MAX && file_path[len1-1] == 0);     // path length
	assert(len2 > 1 && len2 <= 2047 && func_name[len2-1] == 0);         // function name length
	if (err) {
		vb_err_t *ret_val = NULL;
		int64_t n1 = num1 > 0 ? num1 : -num1, n2 = num2 > 0 ? num2 : -num2, file_name_off = (int64_t)(len1-2);
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
		while (file_name_off >= 0 && file_path[file_name_off] != '/' && file_path[file_name_off] != '\\')
			file_name_off--;
		file_name_off++;
		const size_t file_name_len0 = len1 - (size_t)file_name_off;
		// string pattern: str1 (num1; num2); str2
		size_t err_size = sizeof(vb_err_t) + n1len + (num2 != 0 ? n2len + 2 : 0) + 3; // 2 = "; ", 3 = "()\0"
		if (str1len > 0) {
			if (err_size + 3 <= SIZE_MAX - str1len) {
				err_size += (str1len + 1); // 1 = " "
				if (str2len > 0) {
					if (err_size + 2 <= SIZE_MAX - str1len)
						err_size += (str1len + 2); // 2 = "; "
					else
						ret_val = vb_err_new(err, VB_ERR_SIZE_OVERFLOW, 2, "MAX_SIZE overflow", NULL, VB_ERR_FFL);
				}
			} else {
				ret_val = vb_err_new(err, VB_ERR_SIZE_OVERFLOW, 1, "MAX_SIZE overflow", NULL, VB_ERR_FFL);
			}
		} else if (str2len > 0) {
			if (err_size + 2 <= SIZE_MAX - str2len)
				err_size += (str2len + 2); // 2 = "; "
			else
				ret_val = vb_err_new(err, VB_ERR_SIZE_OVERFLOW, 3, "MAX_SIZE overflow", NULL, VB_ERR_FFL);
		}
		if (ret_val == NULL) {
			if (err_size <= SIZE_MAX - file_name_len0) {
				err_size += file_name_len0;
				if (err_size <= SIZE_MAX - len2)
					err_size += file_name_len0;
				else
					ret_val = vb_err_new(err, VB_ERR_SIZE_OVERFLOW, 5, "MAX_SIZE overflow", NULL, VB_ERR_FFL);
			} else {
				ret_val = vb_err_new(err, VB_ERR_SIZE_OVERFLOW, 4, "MAX_SIZE overflow", NULL, VB_ERR_FFL);
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
				data[offset++] = '\0';
				memcpy(&data[offset], &file_path[file_name_off], file_name_len0);
				ret_val->file_name = &data[offset];
				offset += file_name_len0;
				memcpy(&data[offset], func_name, len2);
				ret_val->func_name = &data[offset];
				ret_val->next_err = NULL;
				ret_val->str = data;
				ret_val->num1 = num1;
				ret_val->num2 = num2;
				ret_val->file_line = (int32_t)line;
				ret_val->file_name_len = (int32_t)(file_name_len0 - 1);
				ret_val->func_name_len = (int32_t)(len2 - 1);
			} else {
				oom_err.file_line = (int32_t)__LINE__;
				ret_val = &oom_err;
			}
			vb_err_t **last_err = err;
			while (*last_err)
				last_err = (vb_err_t**)&(*last_err)->next_err;
			*last_err = ret_val;
		}
		return ret_val;
	}
	return NULL;
}

vb_err_t *vb_err_new_oom(vb_err_t **const err, const int64_t num1, const int64_t num2, const char *const str2, const char *const file_path, const char *const func_name, const size_t line, const size_t len1, const size_t len2) {
	return vb_err_new(err, num1, num2, "out of memory", str2, file_path, func_name, line, len1, len2);
}

vb_err_t *vb_err_free(vb_err_t *err) {
	vb_err_t *next_err = err;
	while (next_err && next_err != &oom_err) {
		err = next_err;
		next_err = next_err->next_err;
		free(err);
	}
	return NULL;
}
