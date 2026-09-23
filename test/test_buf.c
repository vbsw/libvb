
/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdalign.h>
#include <vb/buf.h>
#include <vb/mst.h>

#define ROUND_UP(a)   ((a + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))

#define CHUNK_T_SIZE  ROUND_UP(sizeof(vb_mst_chunk_t))

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

static void test_new(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		vb_buf_t *const buf = vb_buf_new(10, 20, NULL, &err);

		ASSERT(err == NULL)
		ASSERT(buf);
		ASSERT(buf->data);
		ASSERT(buf->len == 10);
		ASSERT(buf->cap == 20);
		free(buf);
	}
}

static void test_new_mem(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		vb_mst_t stack = {.err = &err};
		vb_mem_t mem;

		vb_mst_new_max(&stack, 1024);
		vb_mst_push(&stack);
		vb_mst_mem_init(&stack, &mem);
		char *stack_data = (char*)stack.head->block->top_chunk + CHUNK_T_SIZE + ROUND_UP(sizeof(vb_buf_t));
		ASSERT(err == NULL)
		ASSERT(stack.err == &err)
		ASSERT(stack.head)
		ASSERT(mem.alloc != NULL)
		ASSERT(mem.free != NULL)
		ASSERT(mem.destroy != NULL)
		ASSERT(mem.obj != NULL)
		ASSERT(mem.err != NULL)
		ASSERT(stack_data)

		int64_t size_used_prev = stack.head->size_used;
		vb_buf_t *const buf1 = vb_buf_new(10, 20, &mem, &err);
		ASSERT(err == NULL)
		ASSERT(buf1)
		ASSERT(buf1->data)
		ASSERT(buf1->len == 10)
		ASSERT(buf1->cap == 20)
		ASSERT(size_used_prev != stack.head->size_used)
		for (int i = 0; i < 5; i++) {
			stack_data[i] = 'A'+(char)i;
			ASSERT(buf1->data[i] == stack_data[i])
		}

		stack_data = stack_data + ROUND_UP(20) + ROUND_UP(sizeof(vb_buf_t));
		size_used_prev = stack.head->size_used;
		vb_buf_t *const buf2 = vb_buf_new(30, 40, &mem, &err);
		ASSERT(err == NULL)
		ASSERT(buf2)
		ASSERT(buf2->data)
		ASSERT(buf2->len == 30)
		ASSERT(buf2->cap == 40)
		ASSERT(buf1 < buf2)
		ASSERT(size_used_prev != stack.head->size_used)
		for (int i = 0; i < 5; i++) {
			stack_data[i] = 'A'+(char)i;
			ASSERT(buf1->data[i] == stack_data[i])
		}

		vb_mst_destroy(&stack);
	}
}

static void test_new_err(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		vb_buf_t *buf = vb_buf_new(-10, 20, NULL, &err);

		ASSERT(err);
		ASSERT(err->num1 == VB_ERR_BUF_UNDERFLOW);
		ASSERT(err->num2 == 1);
		ASSERT(buf == NULL);
		err = vb_err_free(err);
		ASSERT(err == NULL)

		buf = vb_buf_new(40, 20, NULL, &err);
		ASSERT(err);
		ASSERT(err->num1 == VB_ERR_BUF_LIMIT_EXCEEDED);
		ASSERT(err->num2 == 1);
		ASSERT(buf == NULL);
		err = vb_err_free(err);
		ASSERT(err == NULL)
	}
}

static void test_init_new(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		vb_buf_t buf = {0};

		ASSERT(vb_buf_init_new(&buf, 10, 20, NULL, &err));
		ASSERT(err == NULL)
		ASSERT(buf.data);
		ASSERT(buf.len == 10);
		ASSERT(buf.cap == 20);
		free(buf.data);

		ASSERT(vb_buf_init_new(&buf, 0, 20, NULL, &err));
		ASSERT(err == NULL)
		ASSERT(buf.data);
		ASSERT(buf.len == 0);
		ASSERT(buf.cap == 20);
		free(buf.data);

		ASSERT(vb_buf_init_new_cap(&buf, 20, NULL, &err));
		ASSERT(err == NULL)
		ASSERT(buf.data);
		ASSERT(buf.len == 0);
		ASSERT(buf.cap == 20);
		free(buf.data);
	}
}

void test_buf(int *const err_line) {
	*err_line = 0;
	test_new(err_line);
	test_new_mem(err_line);
	test_new_err(err_line);
	test_init_new(err_line);
}
