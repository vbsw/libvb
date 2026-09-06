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
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mst_t))
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mst_block_t))
#define CHUNK_T_SIZE  ROUND_UP(sizeof(vb_mst_chunk_t))

#define STRUCTS_INIT_SIZE (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)
#define MAX_ROUND_UP      (MST_INT_MAX - alignof(max_align_t) + 1)

vb_mst_t *vb_mst_new(vb_err_t **const err, const int64_t size_init, const int64_t size_limit) {
	assert(err);
	vb_mst_t *mst = NULL;
	if (*err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= size_limit) {
				if (size_limit <= MAX_ROUND_UP) {
					const int64_t size_init_up = (int64_t)ROUND_UP(size_init);
					mst = (vb_mst_t*)malloc((size_t)size_init_up);
					if (mst) {
						vb_mst_chunk_t *const chunk = (vb_mst_chunk_t*)&((char*)mst)[HEAD_T_SIZE + BLOCK_T_SIZE];
						vb_mst_block_t *const block = (vb_mst_block_t*)&((char*)mst)[HEAD_T_SIZE];
						chunk->jump_back = NULL;
						chunk->counter = 0;
						block->next_block = NULL;
						block->top_chunk = chunk;
						block->size_used = (int64_t)STRUCTS_INIT_SIZE;
						block->size_total = size_init_up;
						mst->block = block;
						mst->size_used = (int64_t)STRUCTS_INIT_SIZE;
						mst->size_total = size_init_up;
						mst->size_init = size_init_up;
						mst->size_total_limit = (int64_t)ROUND_UP(size_limit);
					} else {
						*err = vb_err_new_oom(VB_ERR_MST_OOM, 1, NULL);
					}
				} else {
					*err = vb_err_new(VB_ERR_MST_OVERFLOW, 1, "size limit overflow", NULL);
				}
			} else {
				*err = vb_err_new(VB_ERR_MST_LIMIT_EXCEEDED, VB_ERR_NONE, "initial size exceeds limit", NULL);
			}
		} else {
			*err = vb_err_new(VB_ERR_MST_UNDERFLOW, 1, "initial size underflow", NULL);
		}
	}
	return mst;
}

vb_mst_t *vb_mst_new_0(vb_err_t **const err) {
	assert(err);
	vb_mst_t *mst = NULL;
	if (*err == NULL) {
		const int64_t size_init_up = (int64_t)ROUND_UP(STRUCTS_INIT_SIZE);
		mst = (vb_mst_t*)malloc((size_t)size_init_up);
		if (mst) {
			vb_mst_chunk_t *const chunk = (vb_mst_chunk_t*)&((char*)mst)[HEAD_T_SIZE + BLOCK_T_SIZE];
			vb_mst_block_t *const block = (vb_mst_block_t*)&((char*)mst)[HEAD_T_SIZE];
			chunk->jump_back = NULL;
			chunk->counter = 0;
			block->next_block = NULL;
			block->top_chunk = chunk;
			block->size_used = size_init_up;
			block->size_total = size_init_up;
			mst->block = block;
			mst->size_used = size_init_up;
			mst->size_total = size_init_up;
			mst->size_init = size_init_up;
			mst->size_total_limit = size_init_up;
		} else {
			*err = vb_err_new_oom(VB_ERR_MST_OOM, 2, NULL);
		}
	}
	return mst;
}

vb_mst_t *vb_mst_new_lmt(vb_err_t **const err, const int64_t size_init) {
	assert(err);
	vb_mst_t *mst = NULL;
	if (*err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= MAX_ROUND_UP) {
				const int64_t size_init_up = (int64_t)ROUND_UP(size_init);
				mst = (vb_mst_t*)malloc((size_t)size_init_up);
				if (mst) {
					vb_mst_chunk_t *const chunk = (vb_mst_chunk_t*)&((char*)mst)[HEAD_T_SIZE + BLOCK_T_SIZE];
					vb_mst_block_t *const block = (vb_mst_block_t*)&((char*)mst)[HEAD_T_SIZE];
					chunk->jump_back = NULL;
					chunk->counter = 0;
					block->next_block = NULL;
					block->top_chunk = chunk;
					block->size_used = (int64_t)STRUCTS_INIT_SIZE;
					block->size_total = size_init_up;
					mst->block = block;
					mst->size_used = (int64_t)STRUCTS_INIT_SIZE;
					mst->size_total = size_init_up;
					mst->size_init = size_init_up;
					mst->size_total_limit = size_init_up;
				} else {
					*err = vb_err_new_oom(VB_ERR_MST_OOM, 3, NULL);
				}
			} else {
				*err = vb_err_new(VB_ERR_MST_OVERFLOW, 2, "size limit overflow", NULL);
			}
		} else {
			*err = vb_err_new(VB_ERR_MST_UNDERFLOW, 2, "initial size underflow", NULL);
		}
	}
	return mst;
}

vb_mst_t *vb_mst_new_max(vb_err_t **const err, const int64_t size_init) {
	assert(err);
	vb_mst_t *mst = NULL;
	if (*err == NULL) {
		if (size_init >= STRUCTS_INIT_SIZE) {
			if (size_init <= MAX_ROUND_UP) {
				const int64_t size_init_up = (int64_t)ROUND_UP(size_init);
				mst = (vb_mst_t*)malloc((size_t)size_init_up);
				if (mst) {
					vb_mst_chunk_t *const chunk = (vb_mst_chunk_t*)&((char*)mst)[HEAD_T_SIZE + BLOCK_T_SIZE];
					vb_mst_block_t *const block = (vb_mst_block_t*)&((char*)mst)[HEAD_T_SIZE];
					chunk->jump_back = NULL;
					chunk->counter = 0;
					block->next_block = NULL;
					block->top_chunk = chunk;
					block->size_used = (int64_t)STRUCTS_INIT_SIZE;
					block->size_total = size_init_up;
					mst->block = block;
					mst->size_used = (int64_t)STRUCTS_INIT_SIZE;
					mst->size_total = size_init_up;
					mst->size_init = size_init_up;
					mst->size_total_limit = (int64_t)ROUND_UP(MAX_ROUND_UP);
				} else {
					*err = vb_err_new_oom(VB_ERR_MST_OOM, 4, NULL);
				}
			} else {
				*err = vb_err_new(VB_ERR_MST_OVERFLOW, 3, "size limit overflow", NULL);
			}
		} else {
			*err = vb_err_new(VB_ERR_MST_UNDERFLOW, 3, "initial size underflow", NULL);
		}
	}
	return mst;
}

void *vb_mst_alloc(vb_err_t **const err, vb_mst_t *const stack, const int64_t size) {
	assert(err);
	assert(stack);
	assert(VB_MST_PUSHED(stack));
	void *ret_val = NULL;
	if (*err == NULL) {
		if (size > 0) {
			if (size <= (int64_t)(MAX_ROUND_UP - CHUNK_T_SIZE*2 - BLOCK_T_SIZE)) {
				const int64_t size_up = (int64_t)ROUND_UP(size);
				if (size_up <= stack->size_total_limit - stack->size_used) {
					const int64_t size_up_chunk = (int64_t)(size_up + CHUNK_T_SIZE);
					vb_mst_block_t *block = stack->block;
					int64_t block_size_total_sum = 0;
					while (true) {
						const int64_t block_size_free = block->size_total - block->size_used;
						block_size_total_sum += block->size_total;
						if (block->top_chunk->counter == 0 && size_up >= block_size_free) {
							vb_mst_chunk_t *const jump_back = block->top_chunk->jump_back;
							ret_val = (void*)block->top_chunk;
							block->top_chunk = (vb_mst_chunk_t*)&((char*)block->top_chunk)[size_up];
							block->top_chunk->jump_back = jump_back;
							block->top_chunk->counter = 0;
							block->size_used += size_up;
							stack->size_used += size_up;
							break;
						} else if (block->top_chunk->counter > 0 && size_up_chunk <= block_size_free) {
							vb_mst_chunk_t *const jump_back = block->top_chunk;
							block->top_chunk->counter--;
							ret_val = (void*)&((char*)block->top_chunk)[CHUNK_T_SIZE];
							block->top_chunk = (vb_mst_chunk_t*)&((char*)block->top_chunk)[size_up_chunk];
							block->top_chunk->jump_back = jump_back;
							block->top_chunk->counter = 0;
							block->size_used += size_up_chunk;
							stack->size_used += size_up_chunk;
							break;
						} else if (block->next_block) {
							block = block->next_block;
						} else {
							// allocating new block
							const int64_t block_size_used_new = size_up_chunk + BLOCK_T_SIZE + CHUNK_T_SIZE;
							if (block_size_used_new <= stack->size_total_limit - block_size_total_sum) {
								const int64_t block_size_total_new = (stack->size_init >= block_size_used_new) ? stack->size_init : block_size_used_new;
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
									stack->size_used += block_size_used_new;
									stack->size_total += block_size_total_new;
									ret_val = (void*)&((char*)chunk0)[CHUNK_T_SIZE];
								} else {
									*err = vb_err_new_oom(VB_ERR_MST_OOM, 7, NULL);
								}
							} else {
								*err = vb_err_new_oom(VB_ERR_MST_OOM, 6, NULL);
							}
							break;
						}
					}
				} else {
					*err = vb_err_new_oom(VB_ERR_MST_OOM, 5, NULL);
				}
			} else {
				*err = vb_err_new(VB_ERR_MST_OVERFLOW, 4, "allocation size overflow", NULL);
			}
		} else {
			*err = vb_err_new(VB_ERR_MST_UNDERFLOW, 4, "allocation size underflow", NULL);
		}
	}
	return ret_val;
}

void *vb_mst_push(vb_err_t **const err, vb_mst_t *const stack, const int64_t size) {
	assert(err);
	assert(stack);
	void *ret_val = NULL;
	if (*err == NULL) {
		vb_mst_block_t *block = stack->block;
		if (size > 0) {
			if (size <= (int64_t)(MAX_ROUND_UP - CHUNK_T_SIZE*2 - BLOCK_T_SIZE)) {
				const int64_t size_up_chunk = (int64_t)(ROUND_UP(size) + CHUNK_T_SIZE);
				if (size_up_chunk <= stack->size_total_limit - stack->size_used) {
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
							stack->size_used += size_up_chunk;
						} else {
							if (INT64_MAX > block->top_chunk->counter) {
								block->top_chunk->counter++;
							} else {
								vb_mst_block_t *block_rev = stack->block;
								// revert increments until current block
								while (block_rev != block) {
									if (block_rev != block_mod)
										block_rev->top_chunk->counter--;
									block_rev = block_rev->next_block;
								};
								*err = vb_err_new(VB_ERR_MST_OVERFLOW, 5, "push stack overflow", NULL);
								break;
							}
						}
						block = block->next_block;
					} while (block);
					// allocating new block
					if (*err == NULL && ret_val == NULL) {
						const int64_t block_size_used_new = size_up_chunk + BLOCK_T_SIZE + CHUNK_T_SIZE;
						if (block_size_used_new <= stack->size_total_limit - block_size_total_sum) {
							const int64_t block_size_total_new = (stack->size_init >= block_size_used_new) ? stack->size_init : block_size_used_new;
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
								stack->size_used += block_size_used_new;
								stack->size_total += block_size_total_new;
								ret_val = (void*)&((char*)chunk0)[CHUNK_T_SIZE];
							} else {
								*err = vb_err_new_oom(VB_ERR_MST_OOM, 10, NULL);
							}
						} else {
							*err = vb_err_new_oom(VB_ERR_MST_OOM, 9, NULL);
						}
						if (*err) {
							vb_mst_block_t *block_rev = stack->block;
							// revert all increments
							do {
								block_rev->top_chunk->counter--;
								block_rev = block_rev->next_block;
							} while (block_rev);
						}
					}
				} else {
					*err = vb_err_new_oom(VB_ERR_MST_OOM, 8, NULL);
				}
			} else {
				*err = vb_err_new(VB_ERR_MST_OVERFLOW, 6, "allocation size overflow", NULL);
			}
		} else if (size == 0) {
			do {
				if (INT64_MAX > block->top_chunk->counter) {
					block->top_chunk->counter++;
					block = block->next_block;
				} else {
					vb_mst_block_t *block_rev = stack->block;
					// revert increments until current block
					while (block_rev != block) {
						block_rev->top_chunk->counter--;
						block_rev = block_rev->next_block;
					};
					*err = vb_err_new(VB_ERR_MST_OVERFLOW, 5, "push stack overflow", NULL);
					break;
				}
			} while (block);
		} else {
			*err = vb_err_new(VB_ERR_MST_UNDERFLOW, 5, "allocation size underflow", NULL);
		}
	}
	return ret_val;
}

void vb_mst_pop(vb_err_t **const err, vb_mst_t *const stack) {
	assert(err);
	assert(stack);
	if (*err == NULL) {
		vb_mst_block_t *block = stack->block;
		do {
			if (block->top_chunk->counter)
				block->top_chunk->counter--;
			else if (block->top_chunk->jump_back) {
				const int64_t diff = (int64_t)((char*)block->top_chunk - (char*)block->top_chunk->jump_back);
				block->top_chunk = block->top_chunk->jump_back;
				block->size_used -= diff;
				stack->size_used -= diff;
			}
			block = block->next_block;
		} while (block);
	}
}

vb_mst_t *vb_mst_destroy(vb_err_t **const err, vb_mst_t *const stack) {
	assert(err);
	if (*err == NULL) {
		if (stack) {
			vb_mst_block_t *block = stack->block->next_block;
			free((void*)stack);
			while (block) {
				vb_mst_block_t *const next_block = block->next_block;
				free((void*)block);
				block = next_block;
			}
		}
	}
	return NULL;
}
