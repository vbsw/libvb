/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <string.h>
#include <stdalign.h>
#include <vb/mar.h>

#define MST_INT_MAX   ((sizeof(size_t) >= sizeof(int64_t)) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mar_head_t))       // 48
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mar_block_t))      // 32

#define STRUCTS_INIT_SIZE   (HEAD_T_SIZE + BLOCK_T_SIZE)    // 80
#define MAX_BEFORE_ROUND_UP (MST_INT_MAX - alignof(max_align_t) + 1)

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

static void test_new_0(int *const err_line) {
	if (*err_line == 0) {
		vb_mar_t mar = {0};
		const int64_t init_free  = ROUND_UP(sizeof(void*));
		const int64_t init_total = STRUCTS_INIT_SIZE + init_free;

		// new instance
		ASSERT(vb_mar_new_max(&mar, init_total))
		ASSERT(mar.err == NULL)
		ASSERT(mar.head != NULL)
		ASSERT(mar.head->size_block_init == init_total)
		ASSERT(mar.head->block != NULL)
		ASSERT(mar.head->block->next_block == NULL)
		ASSERT(mar.head->size_used == init_total - init_free)
		ASSERT(mar.head->size_total == init_total)
		ASSERT(mar.head->size_overhead == STRUCTS_INIT_SIZE)
		ASSERT(mar.head->size_block_init == init_total)
		ASSERT(mar.head->size_total_max > init_total*100)
		ASSERT(mar.head->block->size_total == init_total)
		ASSERT(mar.head->block->size_used == init_total - init_free)

		// release memory
		vb_mar_destroy(&mar);
		ASSERT(mar.head == NULL);
		ASSERT(mar.err == NULL);
	}
}

static void test_alloc_0(int *const err_line) {
	if (*err_line == 0) {
		vb_mar_t mar = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));        //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free;  //  96

		// new instance (like test_new_0)
		ASSERT(vb_mar_new_max(&mar, (int64_t)init_total))
		ASSERT(mar.err == NULL)

		// allocation on first block without left over
		void *data = vb_mar_alloc(&mar, (int64_t)init_free);
		ASSERT(data != NULL)
		ASSERT(mar.err == NULL)
		// head
		ASSERT(mar.head->size_used == init_total)
		ASSERT(mar.head->size_total == init_total)
		ASSERT(mar.head->size_overhead == STRUCTS_INIT_SIZE)
		// first block
		vb_mar_block_t *const first_block = mar.head->block;
		ASSERT(first_block->next_block == NULL)
		ASSERT(first_block->size_total == init_total)
		ASSERT(first_block->size_used == init_total)

		// release memory
		vb_mar_destroy(&mar);
		ASSERT(mar.head == NULL);
		ASSERT(mar.err == NULL);
	}
}

static void test_alloc_1(int *const err_line) {
	if (*err_line == 0) {
		vb_mar_t mar = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));        //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free;  //  96

		// new instance (like test_new_0)
		ASSERT(vb_mar_new_max(&mar, (int64_t)init_total))
		ASSERT(mar.err == NULL)

		// allocation on second block with left over
		void *data = vb_mar_alloc(&mar, (int64_t)init_free*2);
		ASSERT(data != NULL)
		ASSERT(mar.err == NULL)
		// head
		ASSERT(mar.head->size_used == STRUCTS_INIT_SIZE + BLOCK_T_SIZE + init_free*2)
		ASSERT(mar.head->size_total == init_total*2)
		ASSERT(mar.head->size_overhead == STRUCTS_INIT_SIZE + BLOCK_T_SIZE)
		// first block
		vb_mar_block_t *const first_block = mar.head->block;
		ASSERT(first_block->next_block != NULL)
		ASSERT(first_block->size_total == init_total)
		ASSERT(first_block->size_used == init_total - init_free)
		// second block
		vb_mar_block_t *const second_block = first_block->next_block;
		ASSERT(second_block->size_used  == BLOCK_T_SIZE + init_free*2)
		ASSERT(second_block->size_total == mar.head->size_block_init)

		// release memory
		vb_mar_destroy(&mar);
		ASSERT(mar.head == NULL);
		ASSERT(mar.err == NULL);
	}
}

static void test_alloc_2(int *const err_line) {
	if (*err_line == 0) {
		vb_mar_t mar = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));        //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free;  //  96

		// new instance (like test_new_0)
		ASSERT(vb_mar_new_max(&mar, (int64_t)init_total))
		ASSERT(mar.err == NULL)

		// allocation on second block without left over
		void *data = vb_mar_alloc(&mar, init_total);
		ASSERT(data != NULL)
		ASSERT(mar.err == NULL)
		// head
		ASSERT(mar.head->size_used == STRUCTS_INIT_SIZE + BLOCK_T_SIZE + init_total)
		ASSERT(mar.head->size_total == init_total + BLOCK_T_SIZE + init_total)
		ASSERT(mar.head->size_overhead == STRUCTS_INIT_SIZE + BLOCK_T_SIZE)
		// first block
		vb_mar_block_t *const first_block = mar.head->block;
		ASSERT(first_block->next_block != NULL)
		ASSERT(first_block->size_total == init_total)
		ASSERT(first_block->size_used == init_total - init_free)
		// second block
		vb_mar_block_t *const second_block = first_block->next_block;
		ASSERT(second_block->size_used  == BLOCK_T_SIZE + init_total)
		ASSERT(second_block->size_total == second_block->size_used)
		ASSERT(second_block->size_total > mar.head->size_block_init)

		// release memory
		vb_mar_destroy(&mar);
		ASSERT(mar.head == NULL);
		ASSERT(mar.err == NULL);
	}
}

static void test_alloc_3(int *const err_line) {
	if (*err_line == 0) {
		vb_mar_t mar = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));          //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free*4;  // 144

		// new instance (like test_new_0)
		ASSERT(vb_mar_new_max(&mar, (int64_t)init_total))
		ASSERT(mar.err == NULL)

		// allocation on first block with left over #1
		for (size_t i = 1; i <= 4; i++) {
			void *data = vb_mar_alloc(&mar, (int64_t)init_free);
			ASSERT(data != NULL)
			ASSERT(mar.err == NULL)
			// head
			ASSERT(mar.head->size_used == init_total - init_free*(4-i))
			ASSERT(mar.head->size_total == init_total)
			ASSERT(mar.head->size_overhead == STRUCTS_INIT_SIZE)
			// first block
			vb_mar_block_t *first_block = mar.head->block;
			ASSERT(first_block->next_block == NULL)
			ASSERT(first_block->size_total == init_total)
			ASSERT(first_block->size_used == init_total - init_free*(4-i))
		}

		// release memory
		vb_mar_destroy(&mar);
		ASSERT(mar.head == NULL);
		ASSERT(mar.err == NULL);
	}
}

static void test_alloc_mem(int *const err_line) {
	if (*err_line == 0) {
		vb_mem_t mem = {0};
		vb_mar_t mar = {0};
		const size_t init_free  = ROUND_UP(sizeof(void*));          //  16
		const size_t init_total = STRUCTS_INIT_SIZE + init_free*4;  // 144

		// new instance (like test_new_0)
		ASSERT(vb_mar_new_max(&mar, (int64_t)init_total))
		ASSERT(mar.err == NULL)

		// init allocator interface
		ASSERT(vb_mar_mem_init(&mar, &mem) == &mem)
		ASSERT(mem.alloc != NULL)
		ASSERT(mem.free != NULL)
		ASSERT(mem.destroy != NULL)
		ASSERT(mem.obj == (void*)&mar)
		ASSERT(mem.err == &mar.err)
		ASSERT(*mem.err == NULL)

		// allocation on first block with left over
		for (size_t i = 1; i <= 4; i++) {
			void *data = VB_MEM_ALLOC(&mem, (int64_t)init_free);
			ASSERT(data != NULL)
			ASSERT(mar.err == NULL)
			// head
			ASSERT(mar.head->size_used == init_total - init_free*(4-i))
			ASSERT(mar.head->size_total == init_total)
			ASSERT(mar.head->size_overhead == STRUCTS_INIT_SIZE)
			// first block
			vb_mar_block_t *first_block = mar.head->block;
			ASSERT(first_block->next_block == NULL)
			ASSERT(first_block->size_total == init_total)
			ASSERT(first_block->size_used == init_total - init_free*(4-i))
		}

		// release memory
		vb_mar_destroy(&mar);
		ASSERT(mar.head == NULL);
		ASSERT(mar.err == NULL);
	}
}

void test_mar(int *const err_line) {
	*err_line = 0;
	test_new_0(err_line);
	test_alloc_0(err_line);
	test_alloc_1(err_line);
	test_alloc_2(err_line);
	test_alloc_3(err_line);
	test_alloc_mem(err_line);
}
