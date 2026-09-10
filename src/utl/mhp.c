/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <vb/mhp.h>

#define MST_INT_MAX   ((sizeof(size_t) >= sizeof(int64_t)) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mhp_head_t))
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mhp_block_t))
#define CHUNK_T_SIZE  ROUND_UP(sizeof(vb_mhp_chunk_t))

#define STRUCTS_INIT_SIZE (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)
#define MAX_ROUND_UP      (MST_INT_MAX - alignof(max_align_t) + 1)

void *vb_mhp_alloc(vb_mhp_t *const heap, const int64_t size) {
	assert(heap);
	void *ret_val = NULL;
	if (heap->err == NULL) {
		// TODO
	}
	return ret_val;
}

void vb_mhp_destroy(vb_mhp_t *const heap) {
	assert(heap);
	if (heap->err == NULL) {
		if (heap->head) {
			vb_mhp_block_t *block = heap->head->block->next_block;
			free((void*)heap->head);
			while (block) {
				vb_mhp_block_t *const next_block = block->next_block;
				free((void*)block);
				block = next_block;
			}
			heap->head = NULL;
		}
	}
}

void *vb_mhp_free(vb_mhp_t *const heap, void *const ptr) {
	// TODO
	return NULL;
}

vb_mem_t *vb_mhp_mem_init(vb_mhp_t *const heap, vb_mem_t *const mem) {
	assert(heap);
	assert(mem);
	mem->alloc = (vb_mem_alloc_t)vb_mhp_alloc;
	mem->free = (vb_mem_free_t)vb_mhp_free;
	mem->destroy = (vb_mem_destroy_t)vb_mhp_destroy;
	mem->obj = (void*)heap;
	mem->err = &heap->err;
	return mem;
}

vb_mem_t *vb_mhp_mem_new(vb_mhp_t *const heap) {
	vb_mem_t *const mem = vb_mhp_alloc(heap, sizeof(vb_mem_t));
	return mem ? vb_mhp_mem_init(heap, mem) : NULL;
}

bool vb_mhp_new(vb_mhp_t *const heap, const int64_t size_init, const int64_t size_limit) {
	assert(heap);
	bool ret_val = false;
	if (heap->err == NULL) {
		// TODO
	}
	return ret_val;
}

bool vb_mhp_new_empty(vb_mhp_t *const heap) {
	assert(heap);
	bool ret_val = false;
	if (heap->err == NULL) {
		// TODO
	}
	return ret_val;
}

bool vb_mhp_new_max(vb_mhp_t *const heap, const int64_t size_init) {
	assert(heap);
	bool ret_val = false;
	if (heap->err == NULL) {
		// TODO
	}
	return ret_val;
}

bool vb_mhp_new_min(vb_mhp_t *const heap, const int64_t size_init) {
	assert(heap);
	bool ret_val = false;
	if (heap->err == NULL) {
		// TODO
	}
	return ret_val;
}
