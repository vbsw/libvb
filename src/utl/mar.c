/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <vb/mar.h>

#define MST_INT_MAX   ((sizeof(size_t) >= sizeof(int64_t)) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mar_head_t))
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mar_block_t))

#define STRUCTS_INIT_SIZE (HEAD_T_SIZE + BLOCK_T_SIZE)
#define MAX_ROUND_UP      (MST_INT_MAX - alignof(max_align_t) + 1)

void *vb_mar_alloc(vb_mar_t *const arena, const int64_t size) {
	assert(arena);
	void *ret_val = NULL;
	if (arena->err == NULL) {
		// TODO
	}
	return ret_val;
}

void vb_mar_destroy(vb_mar_t *const arena) {
	assert(arena);
	if (arena->err == NULL) {
		if (arena->head) {
			vb_mar_block_t *block = arena->head->block->next_block;
			free((void*)arena->head);
			while (block) {
				vb_mar_block_t *const next_block = block->next_block;
				free((void*)block);
				block = next_block;
			}
			arena->head = NULL;
		}
	}
}

void *vb_mar_free(vb_mar_t *const arena, void *const ptr) {
	// arena allocated data can not be freed individually
	return NULL;
}

vb_mem_t *vb_mar_mem_init(vb_mar_t *const arena, vb_mem_t *const mem) {
	assert(arena);
	assert(mem);
	mem->alloc = (vb_mem_alloc_t)vb_mar_alloc;
	mem->free = (vb_mem_free_t)vb_mar_free;
	mem->destroy = (vb_mem_destroy_t)vb_mar_destroy;
	mem->obj = (void*)arena;
	mem->err = &arena->err;
	return mem;
}

vb_mem_t *vb_mar_mem_new(vb_mar_t *const arena) {
	vb_mem_t *const mem = vb_mar_alloc(arena, sizeof(vb_mem_t));
	return mem ? vb_mar_mem_init(arena, mem) : NULL;
}

bool vb_mar_new(vb_mar_t *const arena, const int64_t size_init, const int64_t size_limit) {
	assert(arena);
	bool ret_val = false;
	if (arena->err == NULL) {
		// TODO
	}
	return ret_val;
}

bool vb_mar_new_empty(vb_mar_t *const arena) {
	assert(arena);
	bool ret_val = false;
	if (arena->err == NULL) {
		// TODO
	}
	return ret_val;
}

bool vb_mar_new_max(vb_mar_t *const arena, const int64_t size_init) {
	assert(arena);
	bool ret_val = false;
	if (arena->err == NULL) {
		// TODO
	}
	return ret_val;
}

bool vb_mar_new_min(vb_mar_t *const arena, const int64_t size_init) {
	assert(arena);
	bool ret_val = false;
	if (arena->err == NULL) {
		// TODO
	}
	return ret_val;
}
