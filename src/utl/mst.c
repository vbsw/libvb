/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <vb/mst.h>

#define MST_INT_MAX   ((sizeof(size_t) >= sizeof(int64_t)) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mst_head_t))
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mst_block_t))
#define CHUNK_T_SIZE  ROUND_UP(sizeof(vb_mst_chunk_t))

#define STRUCTS_INIT_SIZE (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)
#define MAX_ROUND_UP      (MST_INT_MAX - alignof(max_align_t) + 1)

void *vb_mst_alloc(vb_mst_t *const stack, const int64_t size) {
	assert(stack);
	assert(VB_MST_PUSHED(stack));
	void *ret_val = NULL;
	if (stack->err == NULL) {
		if (size > 0) {
			if (size <= (int64_t)(MAX_ROUND_UP - CHUNK_T_SIZE*2 - BLOCK_T_SIZE)) {
				const int64_t size_up = ROUND_UP(size);
				if (size_up <= stack->head->size_total_limit - stack->head->size_used) {
					const int64_t size_up_chunk = size_up + CHUNK_T_SIZE;
					vb_mst_block_t *block = stack->head->block;
					int64_t block_size_total_sum = 0;
					while (true) {
						const int64_t block_size_free = block->size_total - block->size_used;
						block_size_total_sum += block->size_total;
						if (block->top_chunk->counter == 0 && size_up <= block_size_free) {
							vb_mst_chunk_t *const jump_back = block->top_chunk->jump_back;
							ret_val = (void*)block->top_chunk;
							block->top_chunk = (vb_mst_chunk_t*)&((char*)block->top_chunk)[size_up];
							block->top_chunk->jump_back = jump_back;
							block->top_chunk->counter = 0;
							block->size_used += size_up;
							stack->head->size_used += size_up;
							stack->head->size_total += 0;
							stack->head->size_overhead += 0;
							break;
						} else if (block->top_chunk->counter > 0 && size_up_chunk <= block_size_free) {
							vb_mst_chunk_t *const jump_back = block->top_chunk;
							block->top_chunk->counter--;
							ret_val = (void*)&((char*)block->top_chunk)[CHUNK_T_SIZE];
							block->top_chunk = (vb_mst_chunk_t*)&((char*)block->top_chunk)[size_up_chunk];
							block->top_chunk->jump_back = jump_back;
							block->top_chunk->counter = 0;
							block->size_used += size_up_chunk;
							stack->head->size_used += size_up_chunk;
							stack->head->size_total += 0;
							stack->head->size_overhead += CHUNK_T_SIZE;
							break;
						} else if (block->next_block) {
							block = block->next_block;
						} else {
							// create new block
							const int64_t block_size_used_new = size_up_chunk + BLOCK_T_SIZE + CHUNK_T_SIZE;
							if (block_size_used_new <= stack->head->size_total_limit - block_size_total_sum) {
								const int64_t block_size_total_new = (stack->head->size_init >= block_size_used_new) ? stack->head->size_init : block_size_used_new;
								vb_mst_block_t *const block_new = (vb_mst_block_t*)malloc((size_t)block_size_total_new);
								if (block_new) {
									vb_mst_chunk_t *const chunk0 = (vb_mst_chunk_t*)&((char*)block_new)[BLOCK_T_SIZE];
									vb_mst_chunk_t *const chunk1 = (vb_mst_chunk_t*)&((char*)chunk0)[size_up_chunk];
									chunk0->jump_back = NULL; chunk0->counter = 0;
									chunk1->jump_back = chunk0; chunk1->counter = 0;
									block_new->next_block = NULL;
									block_new->top_chunk = chunk1;
									block_new->size_used = block_size_used_new;
									block_new->size_total = block_size_total_new;
									block->next_block = block_new;
									stack->head->size_used += block_size_used_new;
									stack->head->size_total += block_size_total_new;
									stack->head->size_overhead += BLOCK_T_SIZE + CHUNK_T_SIZE*2;
									ret_val = (void*)&((char*)chunk0)[CHUNK_T_SIZE];
								} else {
									stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 7, NULL);
								}
							} else {
								stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 6, NULL);
							}
							break;
						}
					}
				} else {
					stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 5, NULL);
				}
			} else {
				stack->err = vb_err_new(VB_ERR_MST_OVERFLOW, 4, "allocation size overflow", NULL);
			}
		} else {
			stack->err = vb_err_new(VB_ERR_MST_UNDERFLOW, 4, "allocation size underflow", NULL);
		}
	}
	return ret_val;
}

void vb_mst_destroy(vb_mst_t *const stack) {
	assert(stack);
	if (stack->err == NULL) {
		if (stack->head) {
			vb_mst_block_t *block = stack->head->block->next_block;
			free((void*)stack->head);
			while (block) {
				vb_mst_block_t *const next_block = block->next_block;
				free((void*)block);
				block = next_block;
			}
			stack->head = NULL;
		}
	}
}

void *vb_mst_free(vb_mst_t *const stack, void *const ptr) {
	// stack allocated data can not be freed individually
	return NULL;
}

vb_mem_t *vb_mst_mem_init(vb_mst_t *const stack, vb_mem_t *const mem) {
	assert(stack);
	assert(mem);
	mem->alloc = (vb_mem_alloc_t)vb_mst_alloc;
	mem->free = (vb_mem_free_t)vb_mst_free;
	mem->destroy = (vb_mem_destroy_t)vb_mst_destroy;
	mem->obj = (void*)stack;
	mem->err = &stack->err;
	return mem;
}

vb_mem_t *vb_mst_mem_new(vb_mst_t *const stack) {
	assert(stack);
	vb_mem_t *const mem = vb_mst_alloc(stack, sizeof(vb_mem_t));
	return mem ? vb_mst_mem_init(stack, mem) : NULL;
}

bool vb_mst_new(vb_mst_t *const stack, const int64_t size_init, const int64_t size_limit) {
	assert(stack);
	bool ret_val = false;
	if (stack->err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= size_limit) {
				if (size_limit <= MAX_ROUND_UP) {
					const int64_t size_init_up = ROUND_UP(size_init);
					vb_mst_head_t *const head = (vb_mst_head_t*)malloc((size_t)size_init_up);
					if (head) {
						vb_mst_chunk_t *const chunk = (vb_mst_chunk_t*)&((char*)head)[HEAD_T_SIZE + BLOCK_T_SIZE];
						vb_mst_block_t *const block = (vb_mst_block_t*)&((char*)head)[HEAD_T_SIZE];
						chunk->jump_back = NULL;
						chunk->counter = 0;
						block->next_block = NULL;
						block->top_chunk = chunk;
						block->size_used = STRUCTS_INIT_SIZE;
						block->size_total = size_init_up;
						head->block = block;
						head->size_used = STRUCTS_INIT_SIZE;
						head->size_total = size_init_up;
						head->size_overhead = STRUCTS_INIT_SIZE;
						head->size_init = size_init_up;
						head->size_total_limit = ROUND_UP(size_limit);
						stack->head = head;
						ret_val = true;
					} else {
						stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 1, NULL);
					}
				} else {
					stack->err = vb_err_new(VB_ERR_MST_OVERFLOW, 1, "size limit overflow", NULL);
				}
			} else {
				stack->err = vb_err_new(VB_ERR_MST_LIMIT_EXCEEDED, VB_ERR_NONE, "initial size exceeds limit", NULL);
			}
		} else {
			stack->err = vb_err_new(VB_ERR_MST_UNDERFLOW, 1, "initial size underflow", NULL);
		}
	}
	return ret_val;
}

bool vb_mst_new_empty(vb_mst_t *const stack) {
	assert(stack);
	bool ret_val = false;
	if (stack->err == NULL) {
		const int64_t size_init_up = STRUCTS_INIT_SIZE;
		vb_mst_head_t *const head = (vb_mst_head_t*)malloc((size_t)size_init_up);
		if (head) {
			vb_mst_chunk_t *const chunk = (vb_mst_chunk_t*)&((char*)head)[HEAD_T_SIZE + BLOCK_T_SIZE];
			vb_mst_block_t *const block = (vb_mst_block_t*)&((char*)head)[HEAD_T_SIZE];
			chunk->jump_back = NULL;
			chunk->counter = 0;
			block->next_block = NULL;
			block->top_chunk = chunk;
			block->size_used = size_init_up;
			block->size_total = size_init_up;
			head->block = block;
			head->size_used = size_init_up;
			head->size_total = size_init_up;
			head->size_overhead = size_init_up;
			head->size_init = size_init_up;
			head->size_total_limit = size_init_up;
			stack->head = head;
			ret_val = true;
		} else {
			stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 2, NULL);
		}
	}
	return ret_val;
}

bool vb_mst_new_max(vb_mst_t *const stack, const int64_t size_init) {
	assert(stack);
	bool ret_val = false;
	if (stack->err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= MAX_ROUND_UP) {
				const int64_t size_init_up = (int64_t)ROUND_UP(size_init);
				vb_mst_head_t *const head = (vb_mst_head_t*)malloc((size_t)size_init_up);
				if (head) {
					vb_mst_chunk_t *const chunk = (vb_mst_chunk_t*)&((char*)head)[HEAD_T_SIZE + BLOCK_T_SIZE];
					vb_mst_block_t *const block = (vb_mst_block_t*)&((char*)head)[HEAD_T_SIZE];
					chunk->jump_back = NULL;
					chunk->counter = 0;
					block->next_block = NULL;
					block->top_chunk = chunk;
					block->size_used = STRUCTS_INIT_SIZE;
					block->size_total = size_init_up;
					head->block = block;
					head->size_used = STRUCTS_INIT_SIZE;
					head->size_total = size_init_up;
					head->size_overhead = STRUCTS_INIT_SIZE;
					head->size_init = size_init_up;
					head->size_total_limit = ROUND_UP(MAX_ROUND_UP);
					stack->head = head;
					ret_val = true;
				} else {
					stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 4, NULL);
				}
			} else {
				stack->err = vb_err_new(VB_ERR_MST_OVERFLOW, 3, "size limit overflow", NULL);
			}
		} else {
			stack->err = vb_err_new(VB_ERR_MST_UNDERFLOW, 3, "initial size underflow", NULL);
		}
	}
	return ret_val;
}

bool vb_mst_new_min(vb_mst_t *const stack, const int64_t size_init) {
	assert(stack);
	bool ret_val = false;
	if (stack->err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= MAX_ROUND_UP) {
				const int64_t size_init_up = (int64_t)ROUND_UP(size_init);
				vb_mst_head_t *const head = (vb_mst_head_t*)malloc((size_t)size_init_up);
				if (head) {
					vb_mst_chunk_t *const chunk = (vb_mst_chunk_t*)&((char*)head)[HEAD_T_SIZE + BLOCK_T_SIZE];
					vb_mst_block_t *const block = (vb_mst_block_t*)&((char*)head)[HEAD_T_SIZE];
					chunk->jump_back = NULL;
					chunk->counter = 0;
					block->next_block = NULL;
					block->top_chunk = chunk;
					block->size_used = STRUCTS_INIT_SIZE;
					block->size_total = size_init_up;
					head->block = block;
					head->size_used = STRUCTS_INIT_SIZE;
					head->size_total = size_init_up;
					head->size_overhead = STRUCTS_INIT_SIZE;
					head->size_init = size_init_up;
					head->size_total_limit = size_init_up;
					stack->head = head;
					ret_val = true;
				} else {
					stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 3, NULL);
				}
			} else {
				stack->err = vb_err_new(VB_ERR_MST_OVERFLOW, 2, "size limit overflow", NULL);
			}
		} else {
			stack->err = vb_err_new(VB_ERR_MST_UNDERFLOW, 2, "initial size underflow", NULL);
		}
	}
	return ret_val;
}

void vb_mst_pop(vb_mst_t *const stack) {
	assert(stack);
	if (stack->err == NULL) {
		vb_mst_block_t *block = stack->head->block;
		do {
			if (block->top_chunk->counter)
				block->top_chunk->counter--;
			else if (block->top_chunk->jump_back) {
				const int64_t diff = (int64_t)((char*)block->top_chunk - (char*)block->top_chunk->jump_back);
				block->top_chunk = block->top_chunk->jump_back;
				block->size_used -= diff;
				stack->head->size_used -= diff;
				stack->head->size_overhead -= CHUNK_T_SIZE;
			}
			block = block->next_block;
		} while (block);
	}
}

void *vb_mst_push(vb_mst_t *const stack, const int64_t size) {
	assert(stack);
	void *ret_val = NULL;
	if (stack->err == NULL) {
		vb_mst_block_t *block = stack->head->block;
		if (size > 0) {
			if (size <= (int64_t)(MAX_ROUND_UP - CHUNK_T_SIZE*2 - BLOCK_T_SIZE)) {
				const int64_t size_up_chunk = ROUND_UP(size) + CHUNK_T_SIZE;
				if (size_up_chunk <= stack->head->size_total_limit - stack->head->size_used) {
					vb_mst_block_t *block_mod = NULL;
					int64_t block_size_total_sum = 0;
					// searching for block with enough free memory
					do {
						const int64_t block_size_free = block->size_total - block->size_used;
						block_size_total_sum += block->size_total;
						if (ret_val == NULL && size_up_chunk <= block_size_free) {
							ret_val = (void*)&((char*)block->top_chunk)[CHUNK_T_SIZE];
							vb_mst_chunk_t *const chunk_new = (vb_mst_chunk_t*)&((char*)block->top_chunk)[size_up_chunk];
							chunk_new->jump_back = block->top_chunk;
							chunk_new->counter = 0;
							block_mod = block;
							block_mod->top_chunk = chunk_new;
							block_mod->size_used += size_up_chunk;
							stack->head->size_used += size_up_chunk;
							stack->head->size_total += 0;
							stack->head->size_overhead += CHUNK_T_SIZE;
						} else {
							if (INT64_MAX > block->top_chunk->counter) {
								block->top_chunk->counter++;
							} else {
								vb_mst_block_t *block_rev = stack->head->block;
								// revert increments to current block
								while (block_rev != block) {
									if (block_rev != block_mod)
										block_rev->top_chunk->counter--;
									block_rev = block_rev->next_block;
								};
								stack->err = vb_err_new(VB_ERR_MST_OVERFLOW, 5, "push stack overflow", NULL);
								break;
							}
						}
						block = block->next_block;
					} while (block);
					// allocating new block
					if (stack->err == NULL && ret_val == NULL) {
						const int64_t block_size_used_new = size_up_chunk + BLOCK_T_SIZE + CHUNK_T_SIZE;
						if (block_size_used_new <= stack->head->size_total_limit - block_size_total_sum) {
							const int64_t block_size_total_new = (stack->head->size_init >= block_size_used_new) ? stack->head->size_init : block_size_used_new;
							vb_mst_block_t *const block_new = (vb_mst_block_t*)malloc((size_t)block_size_total_new);
							if (block_new) {
								vb_mst_chunk_t *const chunk0 = (vb_mst_chunk_t*)&((char*)block_new)[BLOCK_T_SIZE];
								vb_mst_chunk_t *const chunk1 = (vb_mst_chunk_t*)&((char*)chunk0)[size_up_chunk];
								chunk0->jump_back = NULL; chunk0->counter = 0;
								chunk1->jump_back = chunk0; chunk1->counter = 0;
								block_new->next_block = NULL;
								block_new->top_chunk = chunk1;
								block_new->size_used = block_size_used_new;
								block_new->size_total = block_size_total_new;
								block->next_block = block_new;
								stack->head->size_used += block_size_used_new;
								stack->head->size_total += block_size_total_new;
								stack->head->size_overhead += BLOCK_T_SIZE + CHUNK_T_SIZE*2;
								ret_val = (void*)&((char*)chunk0)[CHUNK_T_SIZE];
							} else {
								stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 10, NULL);
							}
						} else {
							stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 9, NULL);
						}
						if (stack->err) {
							vb_mst_block_t *block_rev = stack->head->block;
							// revert all increments
							do {
								block_rev->top_chunk->counter--;
								block_rev = block_rev->next_block;
							} while (block_rev);
						}
					}
				} else {
					stack->err = vb_err_new_oom(VB_ERR_MST_OOM, 8, NULL);
				}
			} else {
				stack->err = vb_err_new(VB_ERR_MST_OVERFLOW, 6, "allocation size overflow", NULL);
			}
		} else if (size == 0) {
			do {
				if (INT64_MAX > block->top_chunk->counter) {
					block->top_chunk->counter++;
					block = block->next_block;
				} else {
					vb_mst_block_t *block_rev = stack->head->block;
					// revert increments to current block
					while (block_rev != block) {
						block_rev->top_chunk->counter--;
						block_rev = block_rev->next_block;
					};
					stack->err = vb_err_new(VB_ERR_MST_OVERFLOW, 5, "push stack overflow", NULL);
					break;
				}
			} while (block);
		} else {
			stack->err = vb_err_new(VB_ERR_MST_UNDERFLOW, 5, "allocation size underflow", NULL);
		}
	}
	return ret_val;
}
