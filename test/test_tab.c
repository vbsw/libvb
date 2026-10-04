/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <string.h>
#include <vb/tab.h>

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

static void test_line_begin_end(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb\nccc ddd\reee fff\r\nggg\r";
	const int64_t line_len = (int64_t)strlen(line);

	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);
	ASSERT(tab.err == &err)
	ASSERT(tab.buf == &buf)
	ASSERT(VB_ERR_NULL(tab.err))
	ASSERT(tab.state == 0)
	ASSERT(tab.parse_last_line == false)

	ASSERT(vb_tab_next(&tab))
	ASSERT(VB_ERR_NULL(tab.err))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 7)
	ASSERT(tab.next_line_begin == 8)

	ASSERT(vb_tab_next(&tab))
	ASSERT(VB_ERR_NULL(tab.err))
	ASSERT(tab.line_num == 2)
	ASSERT(tab.line_begin == 8)
	ASSERT(tab.line_end == 15)
	ASSERT(tab.next_line_begin == 16)

	ASSERT(vb_tab_next(&tab))
	ASSERT(VB_ERR_NULL(tab.err))
	ASSERT(tab.line_num == 3)
	ASSERT(tab.line_begin == 16)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)

	ASSERT(vb_tab_next(&tab) == false)
	ASSERT(VB_ERR_NULL(tab.err))
	ASSERT(tab.line_num == 3)
	ASSERT(tab.line_begin == 16)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)

	tab.parse_last_line = true;
	ASSERT(vb_tab_next(&tab))
	ASSERT(VB_ERR_NULL(tab.err))
	ASSERT(tab.line_num == 4)
	ASSERT(tab.line_begin == 25)
	ASSERT(tab.line_end == 28)
	ASSERT(tab.next_line_begin == 29)
}

static void test_element_begin_end(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb\nccc ddd\reee fff\r\nggg\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 7)
	ASSERT(tab.indent == 0)
	ASSERT(tab.next_line_begin == 8)
}

static void test_element_indent(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb\n\tccc ddd\r\t\teee fff\r\n\t\t\t\tggg\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.indent == 0)

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.indent == 1)

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.indent == 2)

	tab.parse_last_line = true;
	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.indent == 4)
	ASSERT(tab.key_begin == 32)
	ASSERT(tab.key_end == 35)
	ASSERT(tab.val_begin == 35)
	ASSERT(tab.val_end == 35)
	ASSERT(tab.next_line_begin == 36)
}

static void test_element(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa   bbb   |   ccc  ddd eee   \r ";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 6)
	ASSERT(tab.val_end == 9)
	ASSERT(tab.val_len == 3)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 31)
	ASSERT(tab.next_line_begin == 32)

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 16)
	ASSERT(tab.key_end == 19)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 21)
	ASSERT(tab.val_end == 28)
	ASSERT(tab.val_len == 7)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 31)
	ASSERT(tab.next_line_begin == 32)
}

static void test_comment(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb#ccc ddd|eee fff\r\ngg#g\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 7)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)

	tab.parse_last_line = true;
	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 2)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 25)
	ASSERT(tab.key_end == 27)
	ASSERT(tab.key_len == 2)
	ASSERT(tab.val_begin == 27)
	ASSERT(tab.val_end == 27)
	ASSERT(tab.val_len == 0)
	ASSERT(tab.line_begin == 25)
	ASSERT(tab.line_end == 29)
	ASSERT(tab.next_line_begin == 30)

	ASSERT(vb_tab_next(&tab) == false)
	ASSERT(tab.line_num == 2)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 30)
	ASSERT(tab.key_end == 30)
	ASSERT(tab.key_len == 0)
	ASSERT(tab.val_begin == 30)
	ASSERT(tab.val_end == 30)
	ASSERT(tab.val_len == 0)
	ASSERT(tab.line_begin == 30)
	ASSERT(tab.line_end == 30)
	ASSERT(tab.next_line_begin == 30)
}

static void test_inline(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb\\ccc ddd|eee fff\r\nggg\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 7)
	ASSERT(tab.val_len == 3)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)
	ASSERT(tab.state == 2) // child

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 1)
	ASSERT(tab.key_begin == 8)
	ASSERT(tab.key_end == 11)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 12)
	ASSERT(tab.val_end == 15)
	ASSERT(tab.val_len == 3)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)
	ASSERT(tab.state == 3) // sibling

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 1)
	ASSERT(tab.key_begin == 16)
	ASSERT(tab.key_end == 19)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 20)
	ASSERT(tab.val_end == 23)
	ASSERT(tab.val_len == 3)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)
	ASSERT(tab.state == 0) // new line

	tab.parse_last_line = true;
	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 2)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 25)
	ASSERT(tab.key_end == 28)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 28)
	ASSERT(tab.val_end == 28)
	ASSERT(tab.val_len == 0)
	ASSERT(tab.line_begin == 25)
	ASSERT(tab.line_end == 28)
	ASSERT(tab.next_line_begin == 29)
	ASSERT(tab.state == 0) // new line
}

static void test_incomplete1(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab) == false)
	ASSERT(tab.line_num == 0)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 0)
	ASSERT(tab.key_len == 0)
	ASSERT(tab.val_begin == 0)
	ASSERT(tab.val_end == 0)
	ASSERT(tab.val_len == 0)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 0)
	ASSERT(tab.next_line_begin == 0)
	ASSERT(vb_tab_rest(&tab) == 7)

	tab.parse_last_line = true;
	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 7)
	ASSERT(tab.val_len == 3)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 7)
	ASSERT(tab.next_line_begin == 7)
	ASSERT(vb_tab_rest(&tab) == 0)
}

static void test_incomplete2(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const lineAAA = "aaa bbb";
	const char *const lineCCC = "aaa bbb\nccc ddd";
	const char *const lineEEE = "aaa bbb\nccc ddd|eee fff\r";
	const int64_t lineAAA_len = (int64_t)strlen(lineAAA);
	const int64_t lineCCC_len = (int64_t)strlen(lineCCC);
	const int64_t lineEEE_len = (int64_t)strlen(lineEEE);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)lineAAA, lineAAA_len, lineAAA_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab) == false)

	vb_buf_init(&buf, (char*)lineCCC, lineCCC_len, lineCCC_len);
	ASSERT(vb_tab_next(&tab))             // aaa bbb

	ASSERT(vb_tab_next(&tab) == false)    // ccc ddd...

	const int64_t rest = vb_tab_reset(&tab);
	ASSERT(rest == 7)
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 0)
	ASSERT(tab.key_len == 0)
	ASSERT(tab.val_begin == 0)
	ASSERT(tab.val_end == 0)
	ASSERT(tab.val_len == 0)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 0)
	ASSERT(tab.next_line_begin == 0)

	const int64_t offset = lineCCC_len - rest;
	vb_buf_init(&buf, (char*)(lineEEE + offset), lineEEE_len - offset, lineEEE_len - offset);
	tab.parse_last_line = true;
	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 2)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 7)
	ASSERT(tab.val_len == 3)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 15)
	ASSERT(tab.next_line_begin == 16)
	ASSERT(tab.state == 3) // sibling
}

static void test_incomplete3(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb#ccc ddd|eee fff\r\ngg#g\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)

	ASSERT(vb_tab_next(&tab) == false)
	ASSERT(vb_tab_rest(&tab) == 5)

	const int64_t rest = vb_tab_reset(&tab);
	ASSERT(rest == 5)
	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 2)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 7)
}

static void test_reset(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbbb#ccc ddd|eee fff\r\ngg#g\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 8)
	ASSERT(tab.val_len == 4)

	const int64_t rest = vb_tab_reset(&tab);
	ASSERT(rest == 5)
	ASSERT(tab.line_num == 1)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 0)
	ASSERT(tab.val_begin == 0)
	ASSERT(tab.val_end == 0)
}

static void test_list1(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tbl_t tbl;
	const char *const line = "one two   three  ";
	const int64_t line_len = (int64_t)strlen(line);

	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tbl_init(&tbl, &buf, 0, line_len);
	ASSERT(tbl.list_begin == 0)
	ASSERT(tbl.list_end == line_len)

	ASSERT(vb_tbl_next(&tbl, ' '))
	ASSERT(tbl.entry_begin == 0)
	ASSERT(tbl.entry_end == 3)
	ASSERT(tbl.entry_len == 3)
	ASSERT(tbl.entry_idx == 0)
	ASSERT(tbl.separator_begin > 0)

	ASSERT(vb_tbl_next(&tbl, ' '))
	ASSERT(tbl.entry_begin == 4)
	ASSERT(tbl.entry_end == 7)
	ASSERT(tbl.entry_len == 3)
	ASSERT(tbl.entry_idx == 1)
	ASSERT(tbl.separator_begin > 0)

	ASSERT(vb_tbl_next(&tbl, ' '))
	ASSERT(tbl.entry_begin == 10)
	ASSERT(tbl.entry_end == 15)
	ASSERT(tbl.entry_len == 5)
	ASSERT(tbl.entry_idx == 2)
	ASSERT(tbl.separator_begin > 0)

	ASSERT(vb_tbl_next(&tbl, ' ') == false)
	ASSERT(tbl.entry_len == 0)
	ASSERT(tbl.entry_idx == 2)
}

static void test_list2(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tbl_t tbl;
	const char *const line = ",one, two  , three  , ";
	const int64_t line_len = (int64_t)strlen(line);

	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tbl_init(&tbl, &buf, 0, line_len);

	ASSERT(vb_tbl_next(&tbl, ','))     // <empty>
	ASSERT(tbl.entry_begin == 0)
	ASSERT(tbl.entry_end == 0)
	ASSERT(tbl.entry_len == 0)
	ASSERT(tbl.entry_idx == 0)
	ASSERT(tbl.separator_begin == 0)

	ASSERT(vb_tbl_next(&tbl, ','))     // one
	ASSERT(tbl.entry_begin == 1)
	ASSERT(tbl.entry_end == 4)
	ASSERT(tbl.entry_len == 3)
	ASSERT(tbl.entry_idx == 1)
	ASSERT(tbl.separator_begin == 4)

	ASSERT(vb_tbl_next(&tbl, ','))     // two
	ASSERT(tbl.entry_begin == 6)
	ASSERT(tbl.entry_end == 9)
	ASSERT(tbl.entry_len == 3)
	ASSERT(tbl.entry_idx == 2)
	ASSERT(tbl.separator_begin == 11)

	ASSERT(vb_tbl_next(&tbl, ','))     // three
	ASSERT(tbl.entry_begin == 13)
	ASSERT(tbl.entry_end == 18)
	ASSERT(tbl.entry_len == 5)
	ASSERT(tbl.entry_idx == 3)
	ASSERT(tbl.separator_begin == 20)

	ASSERT(vb_tbl_next(&tbl, ','))     // <empty>
	ASSERT(tbl.entry_begin == line_len)
	ASSERT(tbl.entry_end == line_len)
	ASSERT(tbl.entry_len == 0)
	ASSERT(tbl.entry_idx == 4)
	ASSERT(tbl.separator_begin == line_len)

	ASSERT(vb_tbl_next(&tbl, ',') == false)
	ASSERT(tbl.entry_len == 0)
	ASSERT(tbl.entry_idx == 4)
}

static void test_list3(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tbl_t tbl;
	const char *const line = ",,,  ";
	const int64_t line_len = (int64_t)strlen(line);

	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tbl_init(&tbl, &buf, 0, line_len);

	ASSERT(vb_tbl_next(&tbl, ','))
	ASSERT(tbl.entry_begin == 0)
	ASSERT(tbl.entry_end == 0)
	ASSERT(tbl.entry_len == 0)
	ASSERT(tbl.entry_idx == 0)
	ASSERT(tbl.separator_begin == 0)

	ASSERT(vb_tbl_next(&tbl, ','))
	ASSERT(tbl.entry_begin == 1)
	ASSERT(tbl.entry_end == 1)
	ASSERT(tbl.entry_len == 0)
	ASSERT(tbl.entry_idx == 1)
	ASSERT(tbl.separator_begin == 1)

	ASSERT(vb_tbl_next(&tbl, ','))
	ASSERT(tbl.entry_begin == 2)
	ASSERT(tbl.entry_end == 2)
	ASSERT(tbl.entry_len == 0)
	ASSERT(tbl.entry_idx == 2)
	ASSERT(tbl.separator_begin == 2)

	ASSERT(vb_tbl_next(&tbl, ','))
	ASSERT(tbl.entry_begin == line_len)
	ASSERT(tbl.entry_end == line_len)
	ASSERT(tbl.entry_len == 0)
	ASSERT(tbl.entry_idx == 3)
	ASSERT(tbl.separator_begin == line_len)

	ASSERT(vb_tbl_next(&tbl, ' ') == false)
	ASSERT(tbl.entry_len == 0)
	ASSERT(tbl.entry_idx == 3)
}

static void test_no_inline1(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb\\ccc ddd|eee fff\r\nggg\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)

	vb_tab_line_no_inl(&tab);
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 23)
	ASSERT(tab.val_len == 19)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)
	ASSERT(tab.state == 0) // new line

	tab.parse_last_line = true;
	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 2)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 25)
	ASSERT(tab.key_end == 28)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 28)
	ASSERT(tab.val_end == 28)
	ASSERT(tab.val_len == 0)
	ASSERT(tab.line_begin == 25)
	ASSERT(tab.line_end == 28)
	ASSERT(tab.next_line_begin == line_len)
	ASSERT(tab.state == 0) // new line
}

static void test_no_inline2(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb\\ccc ddd|eee fff\r\nggg\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 1)

	vb_tab_val_no_inl(&tab);
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 23)
	ASSERT(tab.val_len == 19)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)
	ASSERT(tab.state == 0) // new line

	tab.parse_last_line = true;
	ASSERT(vb_tab_next(&tab))
	ASSERT(tab.line_num == 2)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 25)
	ASSERT(tab.key_end == 28)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 28)
	ASSERT(tab.val_end == 28)
	ASSERT(tab.val_len == 0)
	ASSERT(tab.line_begin == 25)
	ASSERT(tab.line_end == 28)
	ASSERT(tab.next_line_begin == line_len)
	ASSERT(tab.state == 0) // new line
}

static void test_no_inline3(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb\\ccc ddd|eee fff\r\ng\\gg hhh\\iii jjj\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next_no_inl(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 23)
	ASSERT(tab.val_len == 19)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)
	ASSERT(tab.state == 0) // new line

	tab.parse_last_line = true;
	ASSERT(vb_tab_next_no_inl(&tab))
	ASSERT(tab.line_num == 2)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 25)
	ASSERT(tab.key_end == 29)
	ASSERT(tab.key_len == 4)
	ASSERT(tab.val_begin == 30)
	ASSERT(tab.val_end == 41)
	ASSERT(tab.val_len == 11)
	ASSERT(tab.line_begin == 25)
	ASSERT(tab.line_end == 41)
	ASSERT(tab.next_line_begin == line_len)
	ASSERT(tab.state == 0) // new line
}

static void test_no_inline4(int *const err_line) {
	vb_err_t *err = NULL;
	vb_buf_t buf;
	vb_tab_t tab;
	const char *const line = "aaa bbb\\ccc ddd|eee fff\r\ng\\gg hhh\\iii jjj\r";
	const int64_t line_len = (int64_t)strlen(line);

	// init (like test_line_begin_end)
	vb_buf_init(&buf, (char*)line, line_len, line_len);
	vb_tab_init(&tab, &buf, &err);

	ASSERT(vb_tab_next_no_inl(&tab))
	ASSERT(tab.line_num == 1)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 0)
	ASSERT(tab.key_end == 3)
	ASSERT(tab.key_len == 3)
	ASSERT(tab.val_begin == 4)
	ASSERT(tab.val_end == 23)
	ASSERT(tab.val_len == 19)
	ASSERT(tab.line_begin == 0)
	ASSERT(tab.line_end == 23)
	ASSERT(tab.next_line_begin == 25)
	ASSERT(tab.state == 0) // new line

	tab.parse_last_line = true;
	ASSERT(vb_tab_next(&tab))
	vb_tab_key_no_inl(&tab);
	ASSERT(tab.line_num == 2)
	ASSERT(tab.indent == 0)
	ASSERT(tab.key_begin == 25)
	ASSERT(tab.key_end == 29)
	ASSERT(tab.key_len == 4)
	ASSERT(tab.val_begin == 30)
	ASSERT(tab.val_end == 41)
	ASSERT(tab.val_len == 11)
	ASSERT(tab.line_begin == 25)
	ASSERT(tab.line_end == 41)
	ASSERT(tab.next_line_begin == line_len)
	ASSERT(tab.state == 0) // new line
}

void test_tab(int *const err_line) {
	*err_line = 0;
	test_line_begin_end(err_line);
	test_element_begin_end(err_line);
	test_element_indent(err_line);
	test_element(err_line);
	test_comment(err_line);
	test_inline(err_line);
	test_incomplete1(err_line);
	test_incomplete2(err_line);
	test_incomplete3(err_line);
	test_reset(err_line);
	test_list1(err_line);
	test_list2(err_line);
	test_list3(err_line);
	test_no_inline1(err_line);
	test_no_inline2(err_line);
	test_no_inline3(err_line);
	test_no_inline4(err_line);
}
