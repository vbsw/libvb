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
#include <vb/err.h>

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
	void *(*alloc)(vb_err_t **err, void *obj, int64_t size);
	void *(*free)(vb_err_t **err, void *ptr);
	void *(*destroy)(vb_err_t **err, void *obj);
	vb_mst_block_t *block;
	int64_t size_used;
	int64_t size_total;
	int64_t size_init;
	int64_t size_total_limit;
} vb_mst_t;

vb_mst_t *vb_mst_new(vb_err_t **err, int64_t size_init, int64_t size_max);
vb_mst_t *vb_mst_new_0(vb_err_t **err);
vb_mst_t *vb_mst_new_lmt(vb_err_t **err, int64_t size_init);
vb_mst_t *vb_mst_new_max(vb_err_t **err, int64_t size_init);
void *vb_mst_alloc(vb_err_t **err, vb_mst_t *stack, int64_t size);
void *vb_mst_push(vb_err_t **err, vb_mst_t *stack, int64_t size);
void vb_mst_pop(vb_err_t **err, vb_mst_t *stack);
vb_mst_t *vb_mst_destroy(vb_err_t **err, vb_mst_t *stack);

#define VB_MST_PUSHED(stack) ((stack)->block->top_chunk->counter > 0 || (stack)->block->top_chunk->jump_back)

#endif /* VBSW_VB_MST_H */
