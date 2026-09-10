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
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mst_head_t))       // 48
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mst_block_t))      // 32
#define CHUNK_T_SIZE  ROUND_UP(sizeof(vb_mst_chunk_t))      // 16

#define STRUCTS_INIT_SIZE (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)  // 96
#define MAX_ROUND_UP      (MST_INT_MAX - alignof(max_align_t) + 1)

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

static void test_new_0(int *const err_line) {
	if (*err_line == 0) {
		vb_mst_t mst = {0};
		const int64_t init_free  = ROUND_UP(sizeof(void*));
		const int64_t init_total = STRUCTS_INIT_SIZE + init_free;

		// new instance
		ASSERT(vb_mst_new_max(&mst, init_total))
		ASSERT(mst.err == NULL)
		ASSERT(mst.head != NULL)
		ASSERT(mst.head->size_init == init_total)
		ASSERT(mst.head->block != NULL)
		ASSERT(mst.head->block->next_block == NULL)
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->size_total == init_total)
		ASSERT(mst.head->size_used == init_total - init_free)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == init_total - init_free)

		// release memory
		vb_mst_destroy(&mst);
		ASSERT(mst.head == NULL);
		ASSERT(mst.err == NULL);
	}
}

static void test_push_alloc_0(int *const err_line) {
	if (*err_line == 0) {
		vb_mst_t mst = {0};
		const int64_t init_free  = ROUND_UP(sizeof(void*));
		const int64_t init_total = STRUCTS_INIT_SIZE + init_free;

		// new instance (like test_new_0)
		ASSERT(vb_mst_new_max(&mst, init_total))
		ASSERT(mst.err == NULL)

		// empty push
		void *data = vb_mst_push(&mst, 0);
		ASSERT(data == NULL)
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter > 0)
		ASSERT(mst.head->block->top_chunk->jump_back == NULL)
		ASSERT(mst.head->block->next_block == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == init_total - init_free)

		// allocation on second block
		data = vb_mst_alloc(&mst, init_free*2);
		ASSERT(data != NULL)
		ASSERT(mst.err == NULL)
		// first block
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter > 0)
		ASSERT(mst.head->block->top_chunk->jump_back == NULL)
		ASSERT(mst.head->block->next_block != NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == init_total - init_free)
		// second block
		ASSERT(mst.head->block->next_block->top_chunk != NULL)
		ASSERT(mst.head->block->next_block->top_chunk->counter == 0)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back->counter == 0)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->next_block->size_used  == BLOCK_T_SIZE + CHUNK_T_SIZE*2 + ROUND_UP(sizeof(void*))*2)
		ASSERT(mst.head->block->next_block->size_total == mst.head->size_init)

		// release memory
		vb_mst_destroy(&mst);
		ASSERT(mst.head == NULL);
		ASSERT(mst.err == NULL);
	}
}

static void test_push_alloc_1(int *const err_line) {
	if (*err_line == 0) {
		vb_mst_t mst = {0};
		const int64_t init_free  = ROUND_UP(sizeof(void*)) + CHUNK_T_SIZE;
		const int64_t init_total = STRUCTS_INIT_SIZE + init_free;

		// new instance (like test_new_0)
		ASSERT(vb_mst_new_max(&mst, init_total))
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->size_total - mst.head->size_used == init_free)
		ASSERT(mst.head->block->size_total - mst.head->block->size_used == init_free)

		// empty push (like test_push_alloc_0)
		void *data = vb_mst_push(&mst, 0);
		ASSERT(data == NULL)
		ASSERT(mst.err == NULL)

		// allocation on first block
		data = vb_mst_alloc(&mst, ROUND_UP(sizeof(void*)));
		ASSERT(data != NULL)
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->next_block == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == init_total)

		// release memory
		vb_mst_destroy(&mst);
		ASSERT(mst.head == NULL);
		ASSERT(mst.err == NULL);
	}
}

static void test_push_alloc_pop_0(int *const err_line) {
	if (*err_line == 0) {
		vb_mst_t mst = {0};
		const int64_t init_free  = ROUND_UP(sizeof(void*))*2 + CHUNK_T_SIZE;
		const int64_t init_total = STRUCTS_INIT_SIZE + init_free;

		// new instance (like test_new_0)
		ASSERT(vb_mst_new_max(&mst, init_total))
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->size_total - mst.head->size_used == init_free)
		ASSERT(mst.head->block->size_total - mst.head->block->size_used == init_free)

		// empty push (like test_push_alloc_0)
		void *data = vb_mst_push(&mst, 0);
		ASSERT(data == NULL)
		ASSERT(mst.err == NULL)

		// allocation on first block #1
		data = vb_mst_alloc(&mst, ROUND_UP(sizeof(void*)));
		ASSERT(data != NULL)
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->next_block == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == init_total - ROUND_UP(sizeof(void*)))

		// allocation on first block #2
		data = vb_mst_alloc(&mst, ROUND_UP(sizeof(void*)));
		ASSERT(data != NULL)
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->next_block == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == init_total)

		// pop
		vb_mst_pop(&mst);
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == init_total - init_free)

		// release memory
		vb_mst_destroy(&mst);
		ASSERT(mst.head == NULL);
		ASSERT(mst.err == NULL);
	}
}

static void test_push_alloc_pop_1(int *const err_line) {
	if (*err_line == 0) {
		vb_mst_t mst = {0};
		const int64_t init_free_1 = ROUND_UP(sizeof(void*)) + CHUNK_T_SIZE; //  32 = 16 + 16
		const int64_t init_free_2 = init_free_1 + CHUNK_T_SIZE;             //  48 = 32 + 16
		const int64_t init_total  = STRUCTS_INIT_SIZE + init_free_2;        // 144 = 96 + 48

		// new instance (like test_new_0)
		ASSERT(vb_mst_new_max(&mst, init_total))
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->size_init == init_total)
		ASSERT(mst.head->size_total - mst.head->size_used == init_free_2)
		ASSERT(mst.head->block->size_total - mst.head->block->size_used == init_free_2)

		// empty push (like test_push_alloc_0)
		void *data = vb_mst_push(&mst, 0);
		ASSERT(data == NULL)
		ASSERT(mst.err == NULL)

		// allocation on first block
		data = vb_mst_alloc(&mst, ROUND_UP(sizeof(void*)));
		ASSERT(data != NULL)
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->next_block == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == STRUCTS_INIT_SIZE + init_free_1)
		ASSERT(mst.head->block->size_total - mst.head->block->size_used < init_free_1)

		// allocation on second block
		data = vb_mst_alloc(&mst, init_free_1);
		ASSERT(data != NULL)
		ASSERT(mst.err == NULL)
		// first block
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == STRUCTS_INIT_SIZE + init_free_1)
		// second block
		ASSERT(mst.head->block->next_block != NULL)
		ASSERT(mst.head->block->next_block->top_chunk != NULL)
		ASSERT(mst.head->block->next_block->top_chunk->counter == 0)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back->counter == 0)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->next_block->size_total == init_total)
		ASSERT(mst.head->block->next_block->size_used  == init_total - HEAD_T_SIZE)

		// pop
		vb_mst_pop(&mst);
		ASSERT(mst.err == NULL)
		// first block
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == init_total - init_free_2)
		// second block (identical to first)
		ASSERT(mst.head->block->next_block->top_chunk != NULL)
		ASSERT(mst.head->block->next_block->top_chunk->counter == 0)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back == NULL)
		ASSERT(mst.head->block->next_block->size_total == init_total)
		ASSERT(mst.head->block->next_block->size_used == init_total - HEAD_T_SIZE - init_free_2)

		// release memory
		vb_mst_destroy(&mst);
		ASSERT(mst.head == NULL);
		ASSERT(mst.err == NULL);
	}
}

static void test_push_alloc_pop_mem(int *const err_line) {
	if (*err_line == 0) {
		vb_mem_t mem = {0};
		vb_mst_t mst = {0};
		const int64_t init_free_1 = ROUND_UP(sizeof(void*)) + CHUNK_T_SIZE;
		const int64_t init_free_2 = init_free_1 + CHUNK_T_SIZE;
		const int64_t init_total  = STRUCTS_INIT_SIZE + init_free_2;

		// new instance (like test_new_0)
		ASSERT(vb_mst_new_max(&mst, init_total))
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->size_init == init_total)
		ASSERT(mst.head->size_total - mst.head->size_used == init_free_2)
		ASSERT(mst.head->block->size_total - mst.head->block->size_used == init_free_2)

		// init allocator interface
		ASSERT(vb_mst_mem_init(&mst, &mem) == &mem)
		ASSERT(mem.alloc != NULL)
		ASSERT(mem.free != NULL)
		ASSERT(mem.destroy != NULL)
		ASSERT(mem.obj == (void*)&mst)
		ASSERT(mem.err == &mst.err)
		ASSERT(*mem.err == NULL)

		// empty push (like test_push_alloc_0)
		void *data = vb_mst_push(&mst, 0);
		ASSERT(data == NULL)
		ASSERT(mst.err == NULL)

		// allocation on first block
		data = VB_MEM_ALLOC(&mem, ROUND_UP(sizeof(void*)));
		ASSERT(data != NULL)
		ASSERT(mst.err == NULL)
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->next_block == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == STRUCTS_INIT_SIZE + init_free_1)
		ASSERT(mst.head->block->size_total - mst.head->block->size_used < init_free_1)

		// allocation on second block
		data = VB_MEM_ALLOC(&mem, init_free_1);
		ASSERT(data != NULL)
		ASSERT(mst.err == NULL)
		// first block
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == STRUCTS_INIT_SIZE + init_free_1)
		// second block
		ASSERT(mst.head->block->next_block != NULL)
		ASSERT(mst.head->block->next_block->top_chunk != NULL)
		ASSERT(mst.head->block->next_block->top_chunk->counter == 0)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back != NULL)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back->counter == 0)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back->jump_back == NULL)
		ASSERT(mst.head->block->next_block->size_total == init_total)
		ASSERT(mst.head->block->next_block->size_used  == init_total - HEAD_T_SIZE)

		// pop
		vb_mst_pop(&mst);
		ASSERT(mst.err == NULL)
		// first block
		ASSERT(mst.head->block->top_chunk != NULL)
		ASSERT(mst.head->block->top_chunk->counter == 0)
		ASSERT(mst.head->block->top_chunk->jump_back == NULL)
		ASSERT(mst.head->block->size_total == init_total)
		ASSERT(mst.head->block->size_used == init_total - init_free_2)
		// second block (identical to first)
		ASSERT(mst.head->block->next_block->top_chunk != NULL)
		ASSERT(mst.head->block->next_block->top_chunk->counter == 0)
		ASSERT(mst.head->block->next_block->top_chunk->jump_back == NULL)
		ASSERT(mst.head->block->next_block->size_total == init_total)
		ASSERT(mst.head->block->next_block->size_used == init_total - HEAD_T_SIZE - init_free_2)

		// release memory
		vb_mst_destroy(&mst);
		ASSERT(mst.head == NULL);
		ASSERT(mst.err == NULL);
	}
}

void test_mst(int *const err_line) {
	*err_line = 0;
	test_new_0(err_line);
	test_push_alloc_0(err_line);
	test_push_alloc_1(err_line);
	test_push_alloc_pop_0(err_line);
	test_push_alloc_pop_1(err_line);
	test_push_alloc_pop_mem(err_line);
}
