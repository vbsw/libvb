/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <string.h>
#include <vb/mar.h>

#define MST_INT_MAX   ((sizeof(size_t) >= sizeof(int64_t)) ? INT64_MAX : SIZE_MAX)

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define HEAD_T_SIZE   ROUND_UP(sizeof(vb_mst_head_t))       // 48
#define BLOCK_T_SIZE  ROUND_UP(sizeof(vb_mst_block_t))      // 32
#define CHUNK_T_SIZE  ROUND_UP(sizeof(vb_mst_chunk_t))      // 16

#define STRUCTS_INIT_SIZE (HEAD_T_SIZE + BLOCK_T_SIZE + CHUNK_T_SIZE)  // 96
#define MAX_ROUND_UP      (MST_INT_MAX - alignof(max_align_t) + 1)

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

void test_mar(int *const err_line) {
	*err_line = 0;
}
