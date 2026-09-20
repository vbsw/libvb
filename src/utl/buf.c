/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <assert.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdlib.h>
#include <vb/buf.h>

#define MHP_INT_MAX   (sizeof(size_t) >= sizeof(int64_t) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))

#define MAX_BEFORE_ROUND_UP (MHP_INT_MAX - alignof(max_align_t) + 1 - ROUND_UP(sizeof(vb_buf_t)))

static bool buf_init_new(vb_buf_t *const buf, const int64_t len, const int64_t cap, vb_mem_t *const mem, vb_err_t **const err, const int64_t num2a, const int64_t num2b, const int64_t num2c, const int64_t num2d) {
	assert(buf);
	bool ret_val = false;
	if (len >= 0) {
		if (len <= cap) {
			if (cap != 0) {
				if (cap <= MAX_BEFORE_ROUND_UP) {
					if (mem)
						buf->data = VB_MEM_ALLOC(mem, cap);
					else
						buf->data = malloc((size_t)cap);
					if (buf->data) {
						buf->len = len;
						buf->cap = cap;
						ret_val = true;
					} else if (err) {
						if (mem) {
							*err = *mem->err;
							if (*err) {
								(*err)->num1 = VB_ERR_BUF_OOM;
								(*err)->num2 = num2a;
								*mem->err = NULL;
							} else {
								*err = vb_err_new_oom(VB_ERR_BUF_OOM, num2a, NULL);
							}
						} else {
							*err = vb_err_new_oom(VB_ERR_BUF_OOM, num2a, NULL);
						}
						buf->len = 0;
						buf->cap = 0;
					}
				} else if (err) {
					*err = vb_err_new(VB_ERR_BUF_OVERFLOW, num2b, "capacity overflow", NULL);
				}
			} else {
				buf->data = NULL;
				buf->len = 0;
				buf->cap = 0;
				ret_val = true;
			}
		} else if (err) {
			*err = vb_err_new(VB_ERR_BUF_LIMIT_EXCEEDED, num2c, "length exceeds capacity", NULL);
		}
	} else if (err) {
		*err = vb_err_new(VB_ERR_BUF_UNDERFLOW, num2d, "length underflow", NULL);
	}
	return ret_val;
}

static vb_buf_t *buf_new(const int64_t len, const int64_t cap, vb_mem_t *const mem, vb_err_t **const err, const int64_t num2a, const int64_t num2b, const int64_t num2c, const int64_t num2d) {
	vb_buf_t *ret_val = NULL;
	if (len >= 0) {
		if (len <= cap) {
			if (cap <= MAX_BEFORE_ROUND_UP) {
				const int64_t size_total = cap + ROUND_UP(sizeof(vb_buf_t));
				if (mem)
					ret_val = VB_MEM_ALLOC(mem, size_total);
				else
					ret_val = malloc((size_t)size_total);
				if (ret_val) {
					if (cap != 0)
						ret_val->data = (char*)ret_val + ROUND_UP(sizeof(vb_buf_t));
					else
						ret_val->data = NULL;
					ret_val->len = len;
					ret_val->cap = cap;
				} else if (err) {
					if (mem) {
						*err = *mem->err;
						if (*err) {
							(*err)->num1 = VB_ERR_BUF_OOM;
							(*err)->num2 = num2a;
							*mem->err = NULL;
						} else {
							*err = vb_err_new_oom(VB_ERR_BUF_OOM, num2a, NULL);
						}
					} else {
						*err = vb_err_new_oom(VB_ERR_BUF_OOM, num2a, NULL);
					}
				}
			} else if (err) {
				*err = vb_err_new(VB_ERR_BUF_OVERFLOW, num2b, "capacity overflow", NULL);
			}
		} else if (err) {
			*err = vb_err_new(VB_ERR_BUF_LIMIT_EXCEEDED, num2c, "length exceeds capacity", NULL);
		}
	} else if (err) {
		*err = vb_err_new(VB_ERR_BUF_UNDERFLOW, num2d, "length underflow", NULL);
	}
	return ret_val;
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
