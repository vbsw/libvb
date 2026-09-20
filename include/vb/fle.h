/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_FLE_H
#define VBSW_VB_FLE_H

#include <stdint.h>
#include <stdbool.h>
#include <vb/err.h>
#include <vb/buf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	vb_err_t *err;
	vb_buf_t *buf;
	int8_t type;
	bool exists;
} vb_fle_t;

bool vb_fle_alloc_path_buffer (vb_fle_t *file, vb_mem_t *mem);
bool vb_fle_base_name         (vb_fle_t *file);
bool vb_fle_binary_path       (vb_fle_t *file);
bool vb_fle_dir_name          (vb_fle_t *file);
bool vb_fle_mkd               (vb_fle_t *file, const char *path);
bool vb_fle_mkf               (vb_fle_t *file, const char *path);
bool vb_fle_rm                (vb_fle_t *file, const char *path);
bool vb_fle_set_base_name     (vb_fle_t *file, const char *base_name);
bool vb_fle_stats             (vb_fle_t *file, const char *path);

#define VB_FLE_FILE  0
#define VB_FLE_DIR   1
#define VB_FLE_OTHER 127

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_FLE_H */
