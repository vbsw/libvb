/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <assert.h>
#include <stdlib.h>
#include <vb/sze.h>
#include <vb/buf.h>

static bool buf_init_new(vb_buf_t *const buf, const int64_t len, const int64_t cap, vb_mem_t *const mem, vb_err_t **const err, const int64_t num2a, const int64_t num2b, const int64_t num2c, const int64_t num2d) {
	assert(buf);
	bool ret_val = false;
	if (VB_ERR_NULL(err)) {
		if (len >= 0) {
			if (len <= cap) {
				if (cap != 0) {
					if (VB_SZE_MAX_OK_S(cap, alignof(max_align_t) - 1 + VB_SZE_ROUND_UP(sizeof(vb_buf_t)))) {
						if (mem)
							buf->data = VB_MEM_ALLOC(mem, cap);
						else
							buf->data = malloc(cap);
						if (buf->data) {
							buf->len = len;
							buf->cap = cap;
							ret_val = true;
						} else {
							buf->len = 0;
							buf->cap = 0;
							vb_err_new_oom(err, VB_ERR_BUF_OOM, num2a, NULL, VB_ERR_FFL);
						}
					} else {
						vb_err_new(err, VB_ERR_BUF_OVERFLOW, num2b, "capacity overflow", NULL, VB_ERR_FFL);
					}
				} else {
					buf->data = NULL;
					buf->len = 0;
					buf->cap = 0;
					ret_val = true;
				}
			} else {
				vb_err_new(err, VB_ERR_BUF_LIMIT_EXCEEDED, num2c, "length exceeds capacity", NULL, VB_ERR_FFL);
			}
		} else {
			vb_err_new(err, VB_ERR_BUF_UNDERFLOW, num2d, "length underflow", NULL, VB_ERR_FFL);
		}
	}
	return ret_val;
}

static vb_buf_t *buf_new(const int64_t len, const int64_t cap, vb_mem_t *const mem, vb_err_t **const err, const int64_t num2a, const int64_t num2b, const int64_t num2c, const int64_t num2d) {
	vb_buf_t *ret_val = NULL;
	if (VB_ERR_NULL(err)) {
		if (len >= 0) {
			if (len <= cap) {
				if (VB_SZE_MAX_OK_S(cap, alignof(max_align_t) - 1 + VB_SZE_ROUND_UP(sizeof(vb_buf_t)))) {
					const int64_t size_total = cap + VB_SZE_ROUND_UP(sizeof(vb_buf_t));
					if (mem)
						ret_val = VB_MEM_ALLOC(mem, size_total);
					else
						ret_val = malloc(size_total);
					if (ret_val) {
						if (cap != 0)
							ret_val->data = (uint8_t*)ret_val + VB_SZE_ROUND_UP(sizeof(vb_buf_t));
						else
							ret_val->data = NULL;
						ret_val->len = len;
						ret_val->cap = cap;
					} else {
						if (mem) {
							*err = *mem->err;
							if (*err) {
								(*err)->num1 = VB_ERR_BUF_OOM;
								(*err)->num2 = num2a;
								*mem->err = NULL;
							} else {
								vb_err_new_oom(err, VB_ERR_BUF_OOM, num2a, NULL, VB_ERR_FFL);
							}
						} else {
							vb_err_new_oom(err, VB_ERR_BUF_OOM, num2a, NULL, VB_ERR_FFL);
						}
					}
				} else {
					vb_err_new(err, VB_ERR_BUF_OVERFLOW, num2b, "capacity overflow", NULL, VB_ERR_FFL);
				}
			} else {
				vb_err_new(err, VB_ERR_BUF_LIMIT_EXCEEDED, num2c, "length exceeds capacity", NULL, VB_ERR_FFL);
			}
		} else {
			vb_err_new(err, VB_ERR_BUF_UNDERFLOW, num2d, "length underflow", NULL, VB_ERR_FFL);
		}
	}
	return ret_val;
}

void vb_buf_init(vb_buf_t *const buf, uint8_t *const data, const int64_t len, const int64_t cap) {
	assert(buf);
	buf->data = data, buf->len = len, buf->cap = cap;
}

bool vb_buf_init_new(vb_buf_t *const buf, const int64_t len, const int64_t cap, vb_mem_t *const mem, vb_err_t **const err) {
	return buf_init_new(buf, len, cap, mem, err, 4, 4, 4, 4);
}

bool vb_buf_init_new_cap(vb_buf_t *const buf, const int64_t cap, vb_mem_t *const mem, vb_err_t **const err) {
	return buf_init_new(buf, 0, cap, mem, err, 6, 6, 6, 6);
}

bool vb_buf_init_new_len(vb_buf_t *const buf, const int64_t len, vb_mem_t *const mem, vb_err_t **const err) {
	return buf_init_new(buf, len, len, mem, err, 5, 5, 5, 5);
}

vb_buf_t *vb_buf_new(const int64_t len, const int64_t cap, vb_mem_t *const mem, vb_err_t **const err) {
	return buf_new(len, cap, mem, err, 1, 1, 1, 1);
}

vb_buf_t *vb_buf_new_cap(const int64_t cap, vb_mem_t *const mem, vb_err_t **const err) {
	return buf_new(0, cap, mem, err, 3, 3, 3, 3);
}

vb_buf_t *vb_buf_new_len(const int64_t len, vb_mem_t *const mem, vb_err_t **const err) {
	return buf_new(len, len, mem, err, 2, 2, 2, 2);
}
