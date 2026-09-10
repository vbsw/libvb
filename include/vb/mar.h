/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_MAR_H
#define VBSW_VB_MAR_H

#include <stdalign.h>
#include <stddef.h>
#include <stdbool.h>
#include "err.h"
#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vb_mar_block_t {
	struct vb_mar_block_t *next_block;
	void *cursor;
	int64_t size_used;
	int64_t size_total;
} vb_mar_block_t;

typedef struct {
	vb_mar_block_t *block;
	int64_t size_used;
	int64_t size_total;
	int64_t size_overhead;
	int64_t size_init;
	int64_t size_total_limit;
} vb_mar_head_t;

typedef struct {
	vb_mar_head_t *head;
	vb_err_t *err;
} vb_mar_t;

void*     vb_mar_alloc     (vb_mar_t *arena, int64_t size);
void      vb_mar_destroy   (vb_mar_t *arena);
void*     vb_mar_free      (vb_mar_t *arena, void *ptr);
vb_mem_t* vb_mar_mem_init  (vb_mar_t *arena, vb_mem_t *mem);
vb_mem_t* vb_mar_mem_new   (vb_mar_t *arena);
bool      vb_mar_new       (vb_mar_t *arena, int64_t size_init, int64_t size_max);
bool      vb_mar_new_empty (vb_mar_t *arena);
bool      vb_mar_new_max   (vb_mar_t *arena, int64_t size_init);
bool      vb_mar_new_min   (vb_mar_t *arena, int64_t size_init);

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_MAR_H */
