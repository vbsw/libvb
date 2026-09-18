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

#define STRUCTS_INIT_SIZE   (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)  // 96
#define MAX_BEFORE_ROUND_UP (MST_INT_MAX - alignof(max_align_t) + 1)

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
		ASSERT(mhp.head->size_used == init_total - init_free)
		ASSERT(mhp.head->size_total == init_total)
		ASSERT(mhp.head->size_overhead == STRUCTS_INIT_SIZE)
		ASSERT(mhp.head->size_block_init == init_total)
		ASSERT(mhp.head->size_total_max > init_total*100)
		ASSERT(mhp.head->block->size_total == init_total)
		ASSERT(mhp.head->block->size_used == init_total - init_free)

		// release memory
		vb_mhp_destroy(&mhp);
		ASSERT(mhp.head == NULL);
		ASSERT(mhp.err == NULL);
	}
}

static void test_alloc_0(int *const err_line) {
	if (*err_line == 0) {
		vb_mhp_t mhp = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));        //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free;  // 112

		// new instance (like test_new_0)
		ASSERT(vb_mhp_new_max(&mhp, (int64_t)init_total))
		ASSERT(mhp.err == NULL)

		// allocation on first block without left over
		void *data = vb_mhp_alloc(&mhp, (int64_t)init_free);
		ASSERT(data != NULL)
		ASSERT(mhp.err == NULL)
		// head
		ASSERT(mhp.head->size_used == init_total)
		ASSERT(mhp.head->size_total == init_total)
		ASSERT(mhp.head->size_overhead == STRUCTS_INIT_SIZE)
		// first block
		vb_mhp_block_t *const first_block = mhp.head->block;
		ASSERT(first_block->first_free_chunk == NULL)
		vb_mhp_chunk_t *const first_block_chunk = (vb_mhp_chunk_t*)&((char*)first_block)[BLOCK_T_SIZE];
		ASSERT(first_block_chunk->ref == (void*)first_block)
		ASSERT(first_block_chunk->size_total == CHUNK_T_SIZE + init_free)
		ASSERT(first_block->next_block == NULL)
		ASSERT(first_block->size_total == init_total)
		ASSERT(first_block->size_used == init_total)

		// release memory
		vb_mhp_destroy(&mhp);
		ASSERT(mhp.head == NULL);
		ASSERT(mhp.err == NULL);
	}
}

static void test_alloc_1(int *const err_line) {
	if (*err_line == 0) {
		vb_mhp_t mhp = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));        //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free;  // 112

		// new instance (like test_new_0)
		ASSERT(vb_mhp_new_max(&mhp, (int64_t)init_total))
		ASSERT(mhp.err == NULL)

		// allocation on second block with left over
		void *data = vb_mhp_alloc(&mhp, (int64_t)init_free*2);
		ASSERT(data != NULL)
		ASSERT(mhp.err == NULL)
		// head
		ASSERT(mhp.head->size_used == STRUCTS_INIT_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE*2 + init_free*2)
		ASSERT(mhp.head->size_total == init_total*2)
		ASSERT(mhp.head->size_overhead == STRUCTS_INIT_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE*2)
		// first block
		vb_mhp_block_t *const first_block = mhp.head->block;
		ASSERT(first_block->first_free_chunk != NULL)
		ASSERT(first_block->first_free_chunk->ref == NULL)
		ASSERT(first_block->next_block != NULL)
		ASSERT(first_block->size_total == init_total)
		ASSERT(first_block->size_used == init_total - init_free)
		// second block
		vb_mhp_block_t *const second_block = first_block->next_block;
		ASSERT(second_block->first_free_chunk != NULL)
		vb_mhp_chunk_t *const second_block_chunk = (vb_mhp_chunk_t*)&((char*)second_block)[BLOCK_T_SIZE];
		ASSERT(second_block_chunk->ref == (void*)second_block)
		ASSERT(second_block_chunk->size_total == CHUNK_T_SIZE + init_free*2)
		ASSERT(second_block->size_used  == BLOCK_T_SIZE + CHUNK_T_SIZE*2 + init_free*2)
		ASSERT(second_block->size_total == mhp.head->size_block_init)

		// release memory
		vb_mhp_destroy(&mhp);
		ASSERT(mhp.head == NULL);
		ASSERT(mhp.err == NULL);
	}
}

static void test_alloc_2(int *const err_line) {
	if (*err_line == 0) {
		vb_mhp_t mhp = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));        //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free;  // 112

		// new instance (like test_new_0)
		ASSERT(vb_mhp_new_max(&mhp, (int64_t)init_total))
		ASSERT(mhp.err == NULL)

		// allocation on second block without left over
		void *data = vb_mhp_alloc(&mhp, init_total);
		ASSERT(data != NULL)
		ASSERT(mhp.err == NULL)
		// head
		ASSERT(mhp.head->size_used == STRUCTS_INIT_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE + init_total)
		ASSERT(mhp.head->size_total == init_total + BLOCK_T_SIZE + CHUNK_T_SIZE + init_total)
		ASSERT(mhp.head->size_overhead == STRUCTS_INIT_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)
		// first block
		vb_mhp_block_t *const first_block = mhp.head->block;
		ASSERT(first_block->first_free_chunk != NULL)
		ASSERT(first_block->first_free_chunk->ref == NULL)
		ASSERT(first_block->next_block != NULL)
		ASSERT(first_block->size_total == init_total)
		ASSERT(first_block->size_used == init_total - init_free)
		// second block
		vb_mhp_block_t *const second_block = first_block->next_block;
		ASSERT(second_block->first_free_chunk == NULL)
		vb_mhp_chunk_t *const second_block_chunk = (vb_mhp_chunk_t*)&((char*)second_block)[BLOCK_T_SIZE];
		ASSERT(second_block_chunk->ref == (void*)second_block)
		ASSERT(second_block_chunk->size_total == CHUNK_T_SIZE + init_total)
		ASSERT(second_block->size_used  == BLOCK_T_SIZE + CHUNK_T_SIZE + init_total)
		ASSERT(second_block->size_total == second_block->size_used)
		ASSERT(second_block->size_total > mhp.head->size_block_init)

		// release memory
		vb_mhp_destroy(&mhp);
		ASSERT(mhp.head == NULL);
		ASSERT(mhp.err == NULL);
	}
}

static void test_alloc_free_0(int *const err_line) {
	if (*err_line == 0) {
		vb_mhp_t mhp = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));        //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free;  // 112

		// new instance (like test_new_0)
		ASSERT(vb_mhp_new_max(&mhp, (int64_t)init_total))
		ASSERT(mhp.err == NULL)

		// allocation on first block (like test_alloc_0)
		void *data = vb_mhp_alloc(&mhp, (int64_t)init_free);
		ASSERT(data != NULL)
		ASSERT(mhp.err == NULL)

		// free
		data = vb_mhp_free(&mhp, data);
		ASSERT(data == NULL)
		ASSERT(mhp.err == NULL)
		// head
		ASSERT(mhp.head->size_used == init_total - init_free)
		ASSERT(mhp.head->size_total == init_total)
		ASSERT(mhp.head->size_overhead == STRUCTS_INIT_SIZE)
		// first block
		vb_mhp_block_t *const first_block = mhp.head->block;
		ASSERT(first_block->first_free_chunk != NULL)
		vb_mhp_chunk_t *const first_block_chunk = (vb_mhp_chunk_t*)&((char*)first_block)[BLOCK_T_SIZE];
		ASSERT(first_block_chunk == first_block->first_free_chunk)
		ASSERT(first_block_chunk->ref == NULL)
		ASSERT(first_block_chunk->size_total == CHUNK_T_SIZE + init_free)
		ASSERT(first_block->next_block == NULL)
		ASSERT(first_block->size_total == init_total)
		ASSERT(first_block->size_used == init_total - init_free)

		// release memory
		vb_mhp_destroy(&mhp);
		ASSERT(mhp.head == NULL);
		ASSERT(mhp.err == NULL);
	}
}

static void test_alloc_free_1(int *const err_line) {
	if (*err_line == 0) {
		vb_mhp_t mhp = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));        //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free;  // 112

		// new instance (like test_new_0)
		ASSERT(vb_mhp_new_max(&mhp, (int64_t)init_total))
		ASSERT(mhp.err == NULL)

		// allocation on second block (like test_alloc_1)
		void *data = vb_mhp_alloc(&mhp, (int64_t)init_free*2);
		ASSERT(data != NULL)
		ASSERT(mhp.err == NULL)

		// free
		data = vb_mhp_free(&mhp, data);
		ASSERT(data == NULL)
		ASSERT(mhp.err == NULL)
		// head
		ASSERT(mhp.head->size_used == init_total*2 - init_free*2 - HEAD_T_SIZE)
		ASSERT(mhp.head->size_total == init_total*2)
		ASSERT(mhp.head->size_overhead == STRUCTS_INIT_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)
		// first block
		vb_mhp_block_t *const first_block = mhp.head->block;
		ASSERT(first_block->first_free_chunk != NULL)
		vb_mhp_chunk_t *const first_block_chunk = (vb_mhp_chunk_t*)&((char*)first_block)[BLOCK_T_SIZE];
		ASSERT(first_block_chunk == first_block->first_free_chunk)
		ASSERT(first_block_chunk->ref == NULL)
		ASSERT(first_block_chunk->size_total == CHUNK_T_SIZE + init_free)
		ASSERT(first_block->next_block != NULL)
		ASSERT(first_block->size_total == init_total)
		ASSERT(first_block->size_used == init_total - init_free)
		// second block
		vb_mhp_block_t *const second_block = first_block->next_block;
		ASSERT(second_block->first_free_chunk != NULL)
		vb_mhp_chunk_t *const second_block_chunk = (vb_mhp_chunk_t*)&((char*)second_block)[BLOCK_T_SIZE];
		ASSERT(second_block_chunk == second_block->first_free_chunk)
		ASSERT(second_block_chunk->ref == NULL)
		ASSERT(second_block_chunk->size_total == init_total - BLOCK_T_SIZE)
		ASSERT(second_block->next_block == NULL)
		ASSERT(second_block->size_total == init_total)
		ASSERT(second_block->size_used == BLOCK_T_SIZE + CHUNK_T_SIZE)

		// release memory
		vb_mhp_destroy(&mhp);
		ASSERT(mhp.head == NULL);
		ASSERT(mhp.err == NULL);
	}
}

static void test_alloc_mem(int *const err_line) {
	if (*err_line == 0) {
		vb_mem_t mem = {0};
		vb_mhp_t mhp = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));        //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free;  // 112

		// new instance (like test_new_0)
		ASSERT(vb_mhp_new_max(&mhp, (int64_t)init_total))
		ASSERT(mhp.err == NULL)

		// init allocator interface
		ASSERT(vb_mhp_mem_init(&mhp, &mem) == &mem)
		ASSERT(mem.alloc != NULL)
		ASSERT(mem.free != NULL)
		ASSERT(mem.destroy != NULL)
		ASSERT(mem.obj == (void*)&mhp)
		ASSERT(mem.err == &mhp.err)
		ASSERT(*mem.err == NULL)

		// allocation on second block
		void *data = VB_MEM_ALLOC(&mem, (int64_t)init_total);
		ASSERT(data != NULL)
		ASSERT(mhp.err == NULL)
		// first block
		vb_mhp_block_t *const first_block = mhp.head->block;
		ASSERT(first_block->first_free_chunk != NULL)
		ASSERT(first_block->first_free_chunk->ref == NULL)
		ASSERT(first_block->next_block != NULL)
		ASSERT(first_block->size_total == init_total)
		ASSERT(first_block->size_used == init_total - init_free)
		// second block
		vb_mhp_block_t *const second_block = first_block->next_block;
		ASSERT(second_block->first_free_chunk == NULL)
		vb_mhp_chunk_t *const second_block_chunk = (vb_mhp_chunk_t*)&((char*)second_block)[BLOCK_T_SIZE];
		ASSERT(second_block_chunk->ref == (void*)second_block)
		ASSERT(second_block_chunk->size_total == CHUNK_T_SIZE + init_total)
		ASSERT(second_block->size_used  == BLOCK_T_SIZE + CHUNK_T_SIZE + init_total)
		ASSERT(second_block->size_total == second_block->size_used)
		ASSERT(second_block->size_total > mhp.head->size_block_init)

		// release memory
		vb_mhp_destroy(&mhp);
		ASSERT(mhp.head == NULL);
		ASSERT(mhp.err == NULL);
	}
}

void test_mhp(int *const err_line) {
	*err_line = 0;
	test_new_0(err_line);
	test_alloc_0(err_line);
	test_alloc_1(err_line);
	test_alloc_2(err_line);
	test_alloc_free_0(err_line);
	test_alloc_free_1(err_line);
	test_alloc_mem(err_line);
}
