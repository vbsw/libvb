/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_BUF_H
#define VBSW_VB_BUF_H

#include <stdbool.h>
#include <vb/mem.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *data;
	int64_t len;
	int64_t cap;
} vb_buf_t;

bool vb_buf_init_new     (vb_buf_t *buf, int64_t len, int64_t cap, vb_mem_t *mem, vb_err_t **err);
bool vb_buf_init_new_cap (vb_buf_t *buf, int64_t cap, vb_mem_t *mem, vb_err_t **err);
bool vb_buf_init_new_len (vb_buf_t *buf, int64_t len, vb_mem_t *mem, vb_err_t **err);

vb_buf_t *vb_buf_new     (int64_t len, int64_t cap, vb_mem_t *mem, vb_err_t **err);
vb_buf_t *vb_buf_new_cap (int64_t cap, vb_mem_t *mem, vb_err_t **err);
vb_buf_t *vb_buf_new_len (int64_t len, vb_mem_t *mem, vb_err_t **err);

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_BUF_H */
