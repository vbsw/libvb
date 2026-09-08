/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_MST_H
#define VBSW_VB_MST_H

#include <stdalign.h>
#include <stddef.h>
#include <stdbool.h>
#include <vb/err.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vb_mst_chunk_t {
	struct vb_mst_chunk_t *jump_back;
	int64_t counter;
} vb_mst_chunk_t;

typedef struct vb_mst_block_t {
	struct vb_mst_block_t *next_block;
	vb_mst_chunk_t *top_chunk;
	int64_t size_used;
	int64_t size_total;
} vb_mst_block_t;

typedef struct {
	vb_mst_block_t *block;
	int64_t size_used;
	int64_t size_total;
	int64_t size_overhead;
	int64_t size_init;
	int64_t size_total_limit;
} vb_mst_head_t;

typedef struct {
	vb_mst_head_t *head;
	vb_err_t *err;
} vb_mst_t;

bool vb_mst_init(vb_mst_t *stack, int64_t size_init, int64_t size_max);
bool vb_mst_init_empty(vb_mst_t *stack);
bool vb_mst_init_min(vb_mst_t *stack, int64_t size_init);
bool vb_mst_init_max(vb_mst_t *stack, int64_t size_init);
void *vb_mst_alloc(vb_mst_t *stack, int64_t size);
void *vb_mst_push(vb_mst_t *stack, int64_t size);
void vb_mst_pop(vb_mst_t *stack);
void vb_mst_destroy(vb_mst_t *stack);

#define VB_MST_PUSHED(stack) ((stack)->head->block->top_chunk->counter > 0 || (stack)->head->block->top_chunk->jump_back)

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_MST_H */
