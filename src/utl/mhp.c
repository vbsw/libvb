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

#define MHP_INT_MAX   (sizeof(size_t) >= sizeof(int64_t) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mhp_head_t))
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mhp_block_t))
#define CHUNK_T_SIZE  ROUND_UP(sizeof(vb_mhp_chunk_t))

#define STRUCTS_INIT_SIZE (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)
#define MAX_ROUND_UP      (MHP_INT_MAX - alignof(max_align_t) + 1)

void *vb_mhp_alloc(vb_mhp_t *const heap, const int64_t size) {
	assert(heap);
	assert(heap->head);
	void *ret_val = NULL;
	if (heap->err == NULL) {
		if (size > 0) {
			if (size <= (int64_t)(MAX_ROUND_UP - STRUCTS_INIT_SIZE - CHUNK_T_SIZE)) {
				const size_t size_up = (size_t)ROUND_UP(size);
				if (size_up <= heap->head->size_total_limit - heap->head->size_used) {
					vb_mhp_block_t *block = heap->head->block;
					int64_t block_size_total_sum = 0;
					// per block
					while (true) {
						const int64_t block_size_free = block->size_total - block->size_used;
						block_size_total_sum += block->size_total;
						if (size_up <= block_size_free) {
							vb_mhp_chunk_t *free_chunk = block->first_free_chunk;
							assert(free_chunk);
							if (size_up <= free_chunk->size) {
								// first chunk is sufficient
								ret_val = (void*)&((char*)free_chunk)[CHUNK_T_SIZE];
								if (free_chunk->size - size_up > CHUNK_T_SIZE) {
									const size_t size_up_chunk = size_up + CHUNK_T_SIZE;
									free_chunk->size = size_up;
									block->first_free_chunk = (vb_mhp_chunk_t*)&((char*)free_chunk)[size_up_chunk];
									block->first_free_chunk->ref = free_chunk->ref;
									block->first_free_chunk->size = free_chunk->size - size_up_chunk;
									block->size_used += size_up_chunk;
									heap->head->size_used += size_up_chunk;
									heap->head->size_overhead += CHUNK_T_SIZE;
								} else {
									block->first_free_chunk = free_chunk->ref;
									block->size_used += free_chunk->size;
									heap->head->size_used += free_chunk->size;
								}
								free_chunk->ref = (void*)block;
								break;
							} else {
								// search for other chunk
								vb_mhp_chunk_t *free_chunk_prev = free_chunk;
								while (free_chunk->ref) {
									free_chunk = (vb_mhp_chunk_t*)free_chunk->ref;
									if (size_up <= free_chunk->size) {
										ret_val = (void*)&((char*)free_chunk)[CHUNK_T_SIZE];
										break;
									}
								}
								if (ret_val) {
									if (free_chunk->size - size_up > CHUNK_T_SIZE) {
										const size_t size_up_chunk = size_up + CHUNK_T_SIZE;
										free_chunk->size = size_up;
										free_chunk_prev->ref = (void*)&((char*)free_chunk)[size_up_chunk];
										((vb_mhp_chunk_t*)free_chunk_prev->ref)->ref = free_chunk->ref;
										((vb_mhp_chunk_t*)free_chunk_prev->ref)->size = free_chunk->size - size_up_chunk;
										block->size_used += size_up_chunk;
										heap->head->size_used += size_up_chunk;
										heap->head->size_overhead += CHUNK_T_SIZE;
									} else {
										free_chunk_prev->ref = free_chunk->ref;
										block->size_used += free_chunk->size;
										heap->head->size_used += free_chunk->size;
									}
									free_chunk->ref = (void*)block;
									break;
								}
							}
						} else if (block->next_block) {
							block = block->next_block;
						} else {
							// create new block
							const size_t size_up_chunk = size_up + CHUNK_T_SIZE;
							const size_t block_size_used_new = size_up_chunk + BLOCK_T_SIZE;
							const size_t total_rest = heap->head->size_total_limit - block_size_total_sum;
							if (block_size_used_new <= total_rest) {
								const size_t block_size_total_new = (heap->head->size_init > block_size_used_new) ? (heap->head->size_init <= total_rest ? heap->head->size_init : total_rest) : block_size_used_new;
								vb_mhp_block_t *const block_new = (vb_mhp_block_t*)malloc(block_size_total_new);
								if (block_new) {
									vb_mhp_chunk_t *const chunk0 = (vb_mhp_chunk_t*)&((char*)block_new)[BLOCK_T_SIZE];
									const size_t chunk1_size_total = block_size_total_new - BLOCK_T_SIZE - size_up_chunk;
									if (chunk1_size_total > CHUNK_T_SIZE) {
										vb_mhp_chunk_t *const chunk1 = (vb_mhp_chunk_t*)&((char*)chunk0)[size_up_chunk];
										chunk0->size = size_up;
										chunk1->ref = NULL;
										chunk1->size = chunk1_size_total - CHUNK_T_SIZE;
										block_new->first_free_chunk = chunk1;
										block_new->size_used = block_size_used_new + CHUNK_T_SIZE;
										heap->head->size_overhead += (BLOCK_T_SIZE + CHUNK_T_SIZE*2);
									} else {
										chunk0->size = block_size_total_new - BLOCK_T_SIZE - CHUNK_T_SIZE;
										block_new->first_free_chunk = NULL;
										block_new->size_used = block_size_total_new;
										heap->head->size_overhead += (BLOCK_T_SIZE + CHUNK_T_SIZE);
									}
									chunk0->ref = (void*)block_new;
									block_new->next_block = NULL;
									block_new->size_total = block_size_total_new;
									block->next_block = block_new;
									heap->head->size_used += block_new->size_used;
									heap->head->size_total += block_size_total_new;
								} else {
									heap->err = vb_err_new_oom(VB_ERR_MHP_OOM, 7, NULL);
								}
							} else {
								heap->err = vb_err_new_oom(VB_ERR_MHP_OOM, 6, NULL);
							}
							break;
						}
					}
				} else {
					heap->err = vb_err_new_oom(VB_ERR_MHP_OOM, 5, NULL);
				}
			} else {
				heap->err = vb_err_new(VB_ERR_MHP_OVERFLOW, 4, "allocation size overflow", NULL);
			}
		} else {
			heap->err = vb_err_new(VB_ERR_MHP_UNDERFLOW, 4, "allocation size underflow", NULL);
		}
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
