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

#define STRUCTS_INIT_SIZE   (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)
#define MAX_BEFORE_ROUND_UP (MHP_INT_MAX - alignof(max_align_t) + 1)

void *vb_mhp_alloc(vb_mhp_t *const heap, const int64_t size) {
	assert(heap);
	assert(heap->head);
	void *ret_val = NULL;
	if (heap->err == NULL) {
		if (size > 0) {
			if (size <= (int64_t)(MAX_BEFORE_ROUND_UP - STRUCTS_INIT_SIZE - CHUNK_T_SIZE)) {
				const size_t size_up = (size_t)ROUND_UP(size);
				if (size_up <= heap->head->size_total_max - heap->head->size_used) {
					const size_t size_up_chunk = size_up + CHUNK_T_SIZE;
					vb_mhp_block_t *block = heap->head->block;
					int64_t block_size_total_sum = 0;
					// per block
					while (true) {
						const int64_t block_size_free = block->size_total - block->size_used;
						block_size_total_sum += block->size_total;
						if (size_up <= block_size_free) {
							assert(block->first_free_chunk);
							vb_mhp_chunk_t **prev_free_chunk_ref = &block->first_free_chunk;
							vb_mhp_chunk_t *curr_free_chunk = block->first_free_chunk;
							if (size_up_chunk > curr_free_chunk->size_total) {
								curr_free_chunk = (vb_mhp_chunk_t*)curr_free_chunk->ref;
								while (curr_free_chunk && size_up_chunk > curr_free_chunk->size_total) {
									prev_free_chunk_ref = (vb_mhp_chunk_t**)&(*prev_free_chunk_ref)->ref;
									curr_free_chunk = (vb_mhp_chunk_t*)curr_free_chunk->ref;
								}
							}
							if (curr_free_chunk) {
								ret_val = (void*)&((char*)curr_free_chunk)[CHUNK_T_SIZE];
								if (curr_free_chunk->size_total - size_up_chunk > CHUNK_T_SIZE) {
									*prev_free_chunk_ref = (vb_mhp_chunk_t*)&((char*)curr_free_chunk)[size_up_chunk];
									(*prev_free_chunk_ref)->ref = curr_free_chunk->ref;
									(*prev_free_chunk_ref)->size_total = curr_free_chunk->size_total - size_up_chunk;
									block->size_used += size_up_chunk;
									heap->head->size_used += size_up_chunk;
									heap->head->size_overhead += CHUNK_T_SIZE;
									curr_free_chunk->size_total = size_up_chunk;
								} else {
									const size_t add_size_used = curr_free_chunk->size_total - CHUNK_T_SIZE;
									*prev_free_chunk_ref = (vb_mhp_chunk_t*)curr_free_chunk->ref;
									block->size_used += add_size_used;
									heap->head->size_used += add_size_used;
								}
								curr_free_chunk->ref = (void*)block;
								break;
							}
						}
						// next block
						if (block->next_block) {
							block = block->next_block;
						// create new block
						} else {
							const size_t block_size_used_new = size_up_chunk + BLOCK_T_SIZE;
							const size_t total_rest = heap->head->size_total_max - block_size_total_sum;
							if (block_size_used_new <= total_rest) {
								const size_t block_size_total_new = (heap->head->size_block_init > block_size_used_new) ? (heap->head->size_block_init <= total_rest ? heap->head->size_block_init : total_rest) : block_size_used_new;
								vb_mhp_block_t *const block_new = (vb_mhp_block_t*)malloc(block_size_total_new);
								if (block_new) {
									vb_mhp_chunk_t *const chunk0 = (vb_mhp_chunk_t*)&((char*)block_new)[BLOCK_T_SIZE];
									const size_t chunk1_size_total = block_size_total_new - BLOCK_T_SIZE - size_up_chunk;
									ret_val = (void*)&((char*)chunk0)[CHUNK_T_SIZE];
									if (chunk1_size_total > CHUNK_T_SIZE) {
										vb_mhp_chunk_t *const chunk1 = (vb_mhp_chunk_t*)&((char*)chunk0)[size_up_chunk];
										chunk0->size_total = size_up_chunk;
										chunk1->ref = NULL;
										chunk1->size_total = chunk1_size_total;
										block_new->first_free_chunk = chunk1;
										block_new->size_used = block_size_used_new + CHUNK_T_SIZE;
										heap->head->size_overhead += (BLOCK_T_SIZE + CHUNK_T_SIZE*2);
									} else {
										chunk0->size_total = block_size_total_new - BLOCK_T_SIZE;
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
	assert(heap);
	assert(heap->head);
	if (heap->err == NULL && ptr) {
		vb_mhp_chunk_t *const ptr_chunk = (vb_mhp_chunk_t*)&((char*)ptr)[-CHUNK_T_SIZE];
		vb_mhp_block_t *const block = (vb_mhp_block_t*)ptr_chunk->ref;
		vb_mhp_chunk_t *curr_free_chunk = block->first_free_chunk;
		size_t freed_used = ptr_chunk->size_total - CHUNK_T_SIZE;
		size_t freed_overhead = 0;
		ptr_chunk->ref = NULL;
		if (curr_free_chunk) {
			assert(curr_free_chunk != ptr_chunk);
			vb_mhp_chunk_t *prev_free_chunk = curr_free_chunk;
			if (curr_free_chunk < ptr_chunk) {
				curr_free_chunk = (vb_mhp_chunk_t*)curr_free_chunk->ref;
				while (curr_free_chunk && curr_free_chunk < ptr_chunk) {
					prev_free_chunk = (vb_mhp_chunk_t*)prev_free_chunk->ref;
					curr_free_chunk = (vb_mhp_chunk_t*)curr_free_chunk->ref;
				}
			}
			if (curr_free_chunk) {
				if (&((char*)ptr_chunk)[ptr_chunk->size_total] == (char*)curr_free_chunk) {
					ptr_chunk->size_total += curr_free_chunk->size_total;
					freed_overhead = CHUNK_T_SIZE;
				} else {
					ptr_chunk->ref = (void*)curr_free_chunk;
				}
			}
			if (prev_free_chunk == curr_free_chunk) {
				block->first_free_chunk = ptr_chunk;
				// ptr_chunk has been linked to curr_free_chunk in previous if
			} else {
				if (&((char*)prev_free_chunk)[prev_free_chunk->size_total] == (char*)ptr_chunk) {
					prev_free_chunk->size_total += ptr_chunk->size_total;
					freed_overhead = CHUNK_T_SIZE;
				} else {
					prev_free_chunk->ref = (void*)ptr_chunk;
				}
			}
		} else {
			block->first_free_chunk = ptr_chunk;
		}
		freed_used += freed_overhead;
		block->size_used -= freed_used;
		heap->head->size_used -= freed_used;
		heap->head->size_overhead -= freed_overhead;
	}
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

bool vb_mhp_new(vb_mhp_t *const heap, const int64_t size_init, const int64_t size_total_max) {
	assert(heap);
	bool ret_val = false;
	if (heap->err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= size_total_max) {
				if (size_total_max <= MAX_BEFORE_ROUND_UP) {
					const int64_t size_init_up = ROUND_UP(size_init);
					vb_mhp_head_t *const head = (vb_mhp_head_t*)malloc((size_t)size_init_up);
					if (head) {
						vb_mhp_chunk_t *const chunk = (vb_mhp_chunk_t*)&((char*)head)[HEAD_T_SIZE + BLOCK_T_SIZE];
						vb_mhp_block_t *const block = (vb_mhp_block_t*)&((char*)head)[HEAD_T_SIZE];
						chunk->ref = NULL;
						chunk->size_total = size_init_up - HEAD_T_SIZE - BLOCK_T_SIZE;
						block->next_block = NULL;
						block->first_free_chunk = chunk;
						block->size_used = STRUCTS_INIT_SIZE;
						block->size_total = size_init_up;
						head->block = block;
						head->size_used = STRUCTS_INIT_SIZE;
						head->size_total = size_init_up;
						head->size_overhead = STRUCTS_INIT_SIZE;
						head->size_block_init = size_init_up;
						head->size_total_max = ROUND_UP(size_total_max);
						heap->head = head;
						ret_val = true;
					} else {
						heap->err = vb_err_new_oom(VB_ERR_MHP_OOM, 1, NULL);
					}
				} else {
					heap->err = vb_err_new(VB_ERR_MHP_OVERFLOW, 1, "total max size overflow", NULL);
				}
			} else {
				heap->err = vb_err_new(VB_ERR_MHP_LIMIT_EXCEEDED, VB_ERR_NONE, "initial size exceeds limit", NULL);
			}
		} else {
			heap->err = vb_err_new(VB_ERR_MHP_UNDERFLOW, 1, "initial size underflow", NULL);
		}
	}
	return ret_val;
}

bool vb_mhp_new_empty(vb_mhp_t *const heap) {
	assert(heap);
	bool ret_val = false;
	if (heap->err == NULL) {
		const size_t size_init_up = STRUCTS_INIT_SIZE;
		vb_mhp_head_t *const head = (vb_mhp_head_t*)malloc((size_t)size_init_up);
		if (head) {
			vb_mhp_chunk_t *const chunk = (vb_mhp_chunk_t*)&((char*)head)[HEAD_T_SIZE + BLOCK_T_SIZE];
			vb_mhp_block_t *const block = (vb_mhp_block_t*)&((char*)head)[HEAD_T_SIZE];
			chunk->ref = NULL;
			chunk->size_total = size_init_up - HEAD_T_SIZE - BLOCK_T_SIZE;
			block->next_block = NULL;
			block->first_free_chunk = chunk;
			block->size_used = size_init_up;
			block->size_total = size_init_up;
			head->block = block;
			head->size_used = size_init_up;
			head->size_total = size_init_up;
			head->size_overhead = size_init_up;
			head->size_block_init = size_init_up;
			head->size_total_max = size_init_up;
			heap->head = head;
			ret_val = true;
		} else {
			heap->err = vb_err_new_oom(VB_ERR_MHP_OOM, 2, NULL);
		}
	}
	return ret_val;
}

bool vb_mhp_new_max(vb_mhp_t *const heap, const int64_t size_init) {
	assert(heap);
	bool ret_val = false;
	if (heap->err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= MAX_BEFORE_ROUND_UP) {
				const int64_t size_init_up = ROUND_UP(size_init);
				vb_mhp_head_t *const head = (vb_mhp_head_t*)malloc((size_t)size_init_up);
				if (head) {
					vb_mhp_chunk_t *const chunk = (vb_mhp_chunk_t*)&((char*)head)[HEAD_T_SIZE + BLOCK_T_SIZE];
					vb_mhp_block_t *const block = (vb_mhp_block_t*)&((char*)head)[HEAD_T_SIZE];
					chunk->ref = NULL;
					chunk->size_total = size_init_up - HEAD_T_SIZE - BLOCK_T_SIZE;
					block->next_block = NULL;
					block->first_free_chunk = chunk;
					block->size_used = STRUCTS_INIT_SIZE;
					block->size_total = size_init_up;
					head->block = block;
					head->size_used = STRUCTS_INIT_SIZE;
					head->size_total = size_init_up;
					head->size_overhead = STRUCTS_INIT_SIZE;
					head->size_block_init = size_init_up;
					head->size_total_max = ROUND_UP(MAX_BEFORE_ROUND_UP);
					heap->head = head;
					ret_val = true;
				} else {
					heap->err = vb_err_new_oom(VB_ERR_MHP_OOM, 4, NULL);
				}
			} else {
				heap->err = vb_err_new(VB_ERR_MHP_OVERFLOW, 3, "initial size overflow", NULL);
			}
		} else {
			heap->err = vb_err_new(VB_ERR_MHP_UNDERFLOW, 3, "initial size underflow", NULL);
		}
	}
	return ret_val;
}

bool vb_mhp_new_min(vb_mhp_t *const heap, const int64_t size_init) {
	assert(heap);
	bool ret_val = false;
	if (heap->err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= MAX_BEFORE_ROUND_UP) {
				const int64_t size_init_up = ROUND_UP(size_init);
				vb_mhp_head_t *const head = (vb_mhp_head_t*)malloc((size_t)size_init_up);
				if (head) {
					vb_mhp_chunk_t *const chunk = (vb_mhp_chunk_t*)&((char*)head)[HEAD_T_SIZE + BLOCK_T_SIZE];
					vb_mhp_block_t *const block = (vb_mhp_block_t*)&((char*)head)[HEAD_T_SIZE];
					chunk->ref = NULL;
					chunk->size_total = size_init_up - HEAD_T_SIZE - BLOCK_T_SIZE;
					block->next_block = NULL;
					block->first_free_chunk = chunk;
					block->size_used = STRUCTS_INIT_SIZE;
					block->size_total = size_init_up;
					head->block = block;
					head->size_used = STRUCTS_INIT_SIZE;
					head->size_total = size_init_up;
					head->size_overhead = STRUCTS_INIT_SIZE;
					head->size_block_init = size_init_up;
					head->size_total_max = size_init_up;
					heap->head = head;
					ret_val = true;
				} else {
					heap->err = vb_err_new_oom(VB_ERR_MHP_OOM, 3, NULL);
				}
			} else {
				heap->err = vb_err_new(VB_ERR_MHP_OVERFLOW, 2, "initial size overflow", NULL);
			}
		} else {
			heap->err = vb_err_new(VB_ERR_MHP_UNDERFLOW, 2, "initial size underflow", NULL);
		}
	}
	return ret_val;
}
