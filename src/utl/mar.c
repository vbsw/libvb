/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdalign.h>
#include <vb/mar.h>

#define MAR_INT_MAX   ((sizeof(size_t) >= sizeof(int64_t)) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mar_head_t))
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mar_block_t))

#define STRUCTS_INIT_SIZE   (HEAD_T_SIZE + BLOCK_T_SIZE)
#define MAX_BEFORE_ROUND_UP (MAR_INT_MAX - alignof(max_align_t) + 1)

void *vb_mar_alloc(vb_mar_t *const arena, const int64_t size) {
	assert(arena);
	void *ret_val = NULL;
	if (arena->err == NULL) {
		if (size > 0) {
			if (size <= (int64_t)(MAX_BEFORE_ROUND_UP - STRUCTS_INIT_SIZE)) {
				const int64_t size_up = ROUND_UP(size);
				if (size_up <= arena->head->size_total_max - arena->head->size_used) {
					vb_mar_block_t *block = arena->head->block;
					// per block
					while (true) {
						const int64_t block_size_free = block->size_total - block->size_used;
						if (size_up <= block_size_free) {
							ret_val = (void*)&((char*)block)[block->size_used];
							block->size_used += size_up;
							arena->head->size_used += size_up;
							break;
						// next block
						} else if (block->next_block) {
							block = block->next_block;
						// create new block
						} else {
							const int64_t block_size_used_new = size_up + BLOCK_T_SIZE;
							const int64_t total_rest = arena->head->size_total_max - arena->head->size_total;
							if (block_size_used_new <= total_rest) {
								const int64_t block_size_total_new = (arena->head->size_block_init > block_size_used_new) ? (arena->head->size_block_init <= total_rest ? arena->head->size_block_init : total_rest) : block_size_used_new;
								vb_mar_block_t *const block_new = malloc(block_size_total_new);
								if (block_new) {
									ret_val = (void*)&((char*)block_new)[BLOCK_T_SIZE];
									block_new->next_block = NULL;
									block_new->size_used = block_size_used_new;
									block_new->size_total = block_size_total_new;
									block->next_block = block_new;
									arena->head->size_used += block_size_used_new;
									arena->head->size_total += block_size_total_new;
									arena->head->size_overhead += BLOCK_T_SIZE;
								} else {
									arena->err = vb_err_new_oom(VB_ERR_MAR_OOM, 7, NULL);
								}
							} else {
								arena->err = vb_err_new_oom(VB_ERR_MAR_OOM, 6, NULL);
							}
							break;
						}
					}
				} else {
					arena->err = vb_err_new_oom(VB_ERR_MAR_OOM, 5, NULL);
				}
			} else {
				arena->err = vb_err_new(VB_ERR_MAR_OVERFLOW, 4, "allocation size overflow", NULL);
			}
		} else {
			arena->err = vb_err_new(VB_ERR_MAR_UNDERFLOW, 4, "allocation size underflow", NULL);
		}
	}
	return ret_val;
}

void vb_mar_destroy(vb_mar_t *const arena) {
	assert(arena);
	if (arena->err == NULL) {
		if (arena->head) {
			vb_mar_block_t *block = arena->head->block->next_block;
			free(arena->head);
			while (block) {
				vb_mar_block_t *const next_block = block->next_block;
				free(block);
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

bool vb_mar_new(vb_mar_t *const arena, const int64_t size_init, const int64_t size_total_max) {
	assert(arena);
	bool ret_val = false;
	if (arena->err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= size_total_max) {
				if (size_total_max <= MAX_BEFORE_ROUND_UP) {
					const int64_t size_init_up = ROUND_UP(size_init);
					vb_mar_head_t *const head = malloc(size_init_up);
					if (head) {
						vb_mar_block_t *const block = (vb_mar_block_t*)&((char*)head)[HEAD_T_SIZE];
						block->next_block = NULL;
						block->size_used = STRUCTS_INIT_SIZE;
						block->size_total = size_init_up;
						head->block = block;
						head->size_used = STRUCTS_INIT_SIZE;
						head->size_total = size_init_up;
						head->size_overhead = STRUCTS_INIT_SIZE;
						head->size_block_init = size_init_up;
						head->size_total_max = ROUND_UP(size_total_max);
						arena->head = head;
						ret_val = true;
					} else {
						arena->err = vb_err_new_oom(VB_ERR_MAR_OOM, 1, NULL);
					}
				} else {
					arena->err = vb_err_new(VB_ERR_MAR_OVERFLOW, 1, "total max size overflow", NULL);
				}
			} else {
				arena->err = vb_err_new(VB_ERR_MAR_LIMIT_EXCEEDED, VB_ERR_NONE, "initial size exceeds limit", NULL);
			}
		} else {
			arena->err = vb_err_new(VB_ERR_MAR_UNDERFLOW, 1, "initial size underflow", NULL);
		}
	}
	return ret_val;
}

bool vb_mar_new_empty(vb_mar_t *const arena) {
	assert(arena);
	bool ret_val = false;
	if (arena->err == NULL) {
		const int64_t size_init_up = STRUCTS_INIT_SIZE;
		vb_mar_head_t *const head = malloc(size_init_up);
		if (head) {
			vb_mar_block_t *const block = (vb_mar_block_t*)&((char*)head)[HEAD_T_SIZE];
			block->next_block = NULL;
			block->size_used = size_init_up;
			block->size_total = size_init_up;
			head->block = block;
			head->size_used = size_init_up;
			head->size_total = size_init_up;
			head->size_overhead = size_init_up;
			head->size_block_init = size_init_up;
			head->size_total_max = size_init_up;
			arena->head = head;
			ret_val = true;
		} else {
			arena->err = vb_err_new_oom(VB_ERR_MAR_OOM, 2, NULL);
		}
	}
	return ret_val;
}

bool vb_mar_new_max(vb_mar_t *const arena, const int64_t size_init) {
	assert(arena);
	bool ret_val = false;
	if (arena->err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= MAX_BEFORE_ROUND_UP) {
				const int64_t size_init_up = ROUND_UP(size_init);
				vb_mar_head_t *const head = malloc(size_init_up);
				if (head) {
					vb_mar_block_t *const block = (vb_mar_block_t*)&((char*)head)[HEAD_T_SIZE];
					block->next_block = NULL;
					block->size_used = STRUCTS_INIT_SIZE;
					block->size_total = size_init_up;
					head->block = block;
					head->size_used = STRUCTS_INIT_SIZE;
					head->size_total = size_init_up;
					head->size_overhead = STRUCTS_INIT_SIZE;
					head->size_block_init = size_init_up;
					head->size_total_max = ROUND_UP(MAX_BEFORE_ROUND_UP);
					arena->head = head;
					ret_val = true;
				} else {
					arena->err = vb_err_new_oom(VB_ERR_MAR_OOM, 4, NULL);
				}
			} else {
				arena->err = vb_err_new(VB_ERR_MAR_OVERFLOW, 3, "initial size overflow", NULL);
			}
		} else {
			arena->err = vb_err_new(VB_ERR_MAR_UNDERFLOW, 3, "initial size underflow", NULL);
		}
	}
	return ret_val;
}

bool vb_mar_new_min(vb_mar_t *const arena, const int64_t size_init) {
	assert(arena);
	bool ret_val = false;
	if (arena->err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= MAX_BEFORE_ROUND_UP) {
				const int64_t size_init_up = ROUND_UP(size_init);
				vb_mar_head_t *const head = malloc(size_init_up);
				if (head) {
					vb_mar_block_t *const block = (vb_mar_block_t*)&((char*)head)[HEAD_T_SIZE];
					block->next_block = NULL;
					block->size_used = STRUCTS_INIT_SIZE;
					block->size_total = size_init_up;
					head->block = block;
					head->size_used = STRUCTS_INIT_SIZE;
					head->size_total = size_init_up;
					head->size_overhead = STRUCTS_INIT_SIZE;
					head->size_block_init = size_init_up;
					head->size_total_max = size_init_up;
					arena->head = head;
					ret_val = true;
				} else {
					arena->err = vb_err_new_oom(VB_ERR_MAR_OOM, 3, NULL);
				}
			} else {
				arena->err = vb_err_new(VB_ERR_MAR_OVERFLOW, 2, "initial size overflow", NULL);
			}
		} else {
			arena->err = vb_err_new(VB_ERR_MAR_UNDERFLOW, 2, "initial size underflow", NULL);
		}
	}
	return ret_val;
}
