/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <string.h>
#include <vb/mhp.h>

#define MST_INT_MAX   ((sizeof(size_t) >= sizeof(int64_t)) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mhp_head_t))       // 48
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mhp_block_t))      // 32
#define CHUNK_T_SIZE  ROUND_UP(sizeof(vb_mhp_chunk_t))      // 16

#define STRUCTS_INIT_SIZE (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)  // 96
#define MAX_BEFORE_ROUND_UP      (MST_INT_MAX - alignof(max_align_t) + 1)

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

static void test_new_0(int *const err_line) {
	if (*err_line == 0) {
		vb_mhp_t mhp = {0};
		const int64_t init_free  = ROUND_UP(sizeof(void*));
		const int64_t init_total = STRUCTS_INIT_SIZE + init_free;

		// new instance
		ASSERT(vb_mhp_new_max(&mhp, init_total))
		ASSERT(mhp.err == NULL)
		ASSERT(mhp.head != NULL)
		ASSERT(mhp.head->size_block_init == init_total)
		ASSERT(mhp.head->block != NULL)
		ASSERT(mhp.head->block->next_block == NULL)
		ASSERT(mhp.head->block->first_free_chunk != NULL)
		ASSERT(mhp.head->size_total == init_total)
		ASSERT(mhp.head->size_used == init_total - init_free)
		ASSERT(mhp.head->block->size_total == init_total)
		ASSERT(mhp.head->block->size_used == init_total - init_free)

		// release memory
		vb_mhp_destroy(&mhp);
		ASSERT(mhp.head == NULL);
		ASSERT(mhp.err == NULL);
	}
}

static void test_push_alloc_0(int *const err_line) {
	if (*err_line == 0) {
		vb_mhp_t mhp = {0};
		const int64_t init_free  = ROUND_UP(sizeof(void*));
		const int64_t init_total = STRUCTS_INIT_SIZE + init_free;

		// new instance (like test_new_0)
		ASSERT(vb_mhp_new_max(&mhp, init_total))
		ASSERT(mhp.err == NULL)

		// allocation on second block
		void *data = vb_mhp_alloc(&mhp, init_free*4);
		ASSERT(data != NULL)
		ASSERT(mhp.err == NULL)
		// first block
		ASSERT(mhp.head->block->first_free_chunk != NULL)
		ASSERT(mhp.head->block->first_free_chunk->ref == NULL)
		ASSERT(mhp.head->block->next_block != NULL)
		ASSERT(mhp.head->block->size_total == init_total)
		ASSERT(mhp.head->block->size_used == init_total - init_free)
		// second block
		ASSERT(mhp.head->block->next_block->first_free_chunk == NULL)
		vb_mhp_chunk_t *const second_block_chunk = (vb_mhp_chunk_t*)&((char*)mhp.head->block->next_block)[BLOCK_T_SIZE];
		ASSERT(second_block_chunk->ref == (void*)mhp.head->block->next_block)
		ASSERT(second_block_chunk->size_total == CHUNK_T_SIZE + (size_t)init_free*4)
		ASSERT(mhp.head->block->next_block->size_used  == BLOCK_T_SIZE + CHUNK_T_SIZE + (size_t)init_free*4)
		ASSERT(mhp.head->block->next_block->size_total == mhp.head->size_block_init)

		// release memory
		vb_mhp_destroy(&mhp);
		ASSERT(mhp.head == NULL);
		ASSERT(mhp.err == NULL);
	}
}

void test_mhp(int *const err_line) {
	*err_line = 0;
	test_new_0(err_line);
	test_push_alloc_0(err_line);
}
