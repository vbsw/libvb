/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <string.h>
#include <vb/mst.h>

#define MST_INT_MAX   ((sizeof(size_t) >= sizeof(int64_t)) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mst_t))            // 64
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mst_block_t))      // 32
#define CHUNK_T_SIZE  ROUND_UP(sizeof(vb_mst_chunk_t))      // 16

#define STRUCTS_INIT_SIZE (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)  // 112
#define MAX_ROUND_UP      (MST_INT_MAX - alignof(max_align_t) + 1)

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

static void test_new(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;

		// new instance
		vb_mst_t *mst = vb_mst_new_max(&err, STRUCTS_INIT_SIZE + sizeof(void*));
		ASSERT(err == NULL)
		ASSERT(mst != NULL)
		ASSERT(mst->block != NULL)
		ASSERT(mst->block->next_block == NULL)
		ASSERT(mst->block->top_chunk != NULL)
		ASSERT(mst->size_total - mst->size_used == ROUND_UP(sizeof(void*)))
		ASSERT(mst->block->size_total - mst->block->size_used == ROUND_UP(sizeof(void*)))

		// first push (empty)
		void *data = vb_mst_push(&err, mst, 0);
		ASSERT(err == NULL)
		ASSERT(data == NULL)
		ASSERT(mst->block->top_chunk->counter > 0)
		ASSERT(mst->block->top_chunk->jump_back == NULL)
		ASSERT(mst->block->next_block == NULL)
		ASSERT(mst->block->size_total == STRUCTS_INIT_SIZE + ROUND_UP(sizeof(void*)))
		ASSERT(mst->block->size_used == STRUCTS_INIT_SIZE)

		// allocation (on second block)
		data = vb_mst_alloc(&err, mst, ROUND_UP(sizeof(void*))*2);
		ASSERT(err == NULL)
		ASSERT(data != NULL)
		ASSERT(mst->block->top_chunk->counter > 0)
		ASSERT(mst->block->top_chunk->jump_back == NULL)
		ASSERT(mst->block->next_block != NULL)
		ASSERT(mst->block->size_total == STRUCTS_INIT_SIZE + ROUND_UP(sizeof(void*)))
		ASSERT(mst->block->size_used == STRUCTS_INIT_SIZE)
		ASSERT(mst->block->next_block->top_chunk->counter == 0)
		ASSERT(mst->block->next_block->top_chunk->jump_back != NULL)
		ASSERT(mst->block->next_block->top_chunk->jump_back->counter == 0)
		ASSERT(mst->block->next_block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst->block->next_block->size_used  == BLOCK_T_SIZE + CHUNK_T_SIZE*2 + ROUND_UP(sizeof(void*))*2)
		ASSERT(mst->block->next_block->size_total == mst->size_init)
		ASSERT(mst->block->size_total - mst->block->size_used == ROUND_UP(sizeof(void*)))

		// release memory
		mst = vb_mst_destroy(&err, mst);
		ASSERT(mst == NULL);
	}
}

static void test_alloc(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		const int64_t init_size = STRUCTS_INIT_SIZE + ROUND_UP(sizeof(void*)) + CHUNK_T_SIZE;

		// new instance
		vb_mst_t *mst = vb_mst_new_max(&err, init_size);
		ASSERT(err == NULL)
		ASSERT(mst != NULL)
		ASSERT(mst->block != NULL)
		ASSERT(mst->block->next_block == NULL)
		ASSERT(mst->block->top_chunk != NULL)
		ASSERT(mst->size_total - mst->size_used == ROUND_UP(sizeof(void*)) + CHUNK_T_SIZE)
		ASSERT(mst->block->size_total - mst->block->size_used == ROUND_UP(sizeof(void*)) + CHUNK_T_SIZE)

		// first push (empty)
		void *data = vb_mst_push(&err, mst, 0);
		ASSERT(err == NULL)
		ASSERT(data == NULL)
		ASSERT(mst->block->top_chunk->counter > 0)
		ASSERT(mst->block->top_chunk->jump_back == NULL)
		ASSERT(mst->block->next_block == NULL)
		ASSERT(mst->block->size_total == init_size)
		ASSERT(mst->block->size_used == STRUCTS_INIT_SIZE)

		// allocation (on first block)
		data = vb_mst_alloc(&err, mst, ROUND_UP(sizeof(void*)));
		ASSERT(err == NULL)
		ASSERT(data != NULL)
		ASSERT(mst->block->top_chunk->counter == 0)
		ASSERT(mst->block->top_chunk->jump_back != NULL)
		ASSERT(mst->block->next_block == NULL)
		ASSERT(mst->block->size_total == init_size)
		ASSERT(mst->block->size_used == init_size)
		ASSERT(mst->block->size_total - mst->block->size_used == 0)

		// release memory
		mst = vb_mst_destroy(&err, mst);
		ASSERT(mst == NULL);
	}
}

void test_mst(int *const err_line) {
	*err_line = 0;
	test_new(err_line);
	test_alloc(err_line);
}
