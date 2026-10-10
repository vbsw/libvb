/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_TAB_H
#define VBSW_VB_TAB_H

#include <stdbool.h>
#include <vb/buf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	vb_err_t **err;
	vb_buf_t *buf;
	int64_t key_begin, key_end, key_len;
	int64_t val_begin, val_end, val_len;
	int64_t indent, line_num;
	int64_t line_begin, line_end, line_len;
	int64_t next_key_begin, next_line_begin;
	int8_t state;
	bool parse_last_line;
} vb_tab_t;

typedef struct {
	vb_buf_t *buf;
	int64_t list_begin, list_end;
	int64_t entry_begin, entry_end;
	int64_t entry_len, entry_idx;
	int64_t separator_begin;
} vb_tab_list_t;

void    vb_tab_init           (vb_tab_t *tab, vb_buf_t *buf, vb_err_t **err);
void    vb_tab_key_no_inl     (vb_tab_t *tab);
void    vb_tab_key_val_no_inl (vb_tab_t *tab);
void    vb_tab_line_no_inl    (vb_tab_t *tab);
bool    vb_tab_next           (vb_tab_t *tab);
bool    vb_tab_next_no_inl    (vb_tab_t *tab);
int64_t vb_tab_reset          (vb_tab_t *tab);
int64_t vb_tab_rest           (vb_tab_t *tab);
void    vb_tab_val_no_inl     (vb_tab_t *tab);

void    vb_tab_list_init     (vb_tab_list_t *tab_list, vb_buf_t *buf, int64_t list_begin, int64_t list_end);
void    vb_tab_list_key_init (vb_tab_list_t *tab_list, vb_tab_t *tab);
bool    vb_tab_list_next     (vb_tab_list_t *tab_list, uint8_t separator);
void    vb_tab_list_val_init (vb_tab_list_t *tab_list, vb_tab_t *tab);

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_TAB_H */
