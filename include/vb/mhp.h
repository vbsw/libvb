/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_MHP_H
#define VBSW_VB_MHP_H

#include <stdalign.h>
#include <stddef.h>
#include <stdbool.h>
#include "err.h"
#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vb_mhp_chunk_t {
	struct vb_mhp_chunk_t *jump_back;
	int64_t counter;
} vb_mhp_chunk_t;

typedef struct vb_mhp_block_t {
	struct vb_mhp_block_t *next_block;
	vb_mhp_chunk_t *top_chunk;
	int64_t size_used;
	int64_t size_total;
} vb_mhp_block_t;

typedef struct {
	vb_mhp_block_t *block;
	int64_t size_used;
	int64_t size_total;
	int64_t size_overhead;
	int64_t size_init;
	int64_t size_total_limit;
} vb_mhp_head_t;

typedef struct {
	vb_mhp_head_t *head;
	vb_err_t *err;
} vb_mhp_t;

void*     vb_mhp_alloc     (vb_mhp_t *heap, int64_t size);
void      vb_mhp_destroy   (vb_mhp_t *heap);
void*     vb_mhp_free      (vb_mhp_t *heap, void *ptr);
vb_mem_t* vb_mhp_mem_init  (vb_mhp_t *heap, vb_mem_t *mem);
vb_mem_t* vb_mhp_mem_new   (vb_mhp_t *heap);
bool      vb_mhp_new       (vb_mhp_t *heap, int64_t size_init, int64_t size_max);
bool      vb_mhp_new_empty (vb_mhp_t *heap);
bool      vb_mhp_new_max   (vb_mhp_t *heap, int64_t size_init);
bool      vb_mhp_new_min   (vb_mhp_t *heap, int64_t size_init);

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_MHP_H */
