/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <assert.h>
#include <vb/tab.h>

#define STATE_NEW_LINE        0
#define STATE_NEW_LINE_PREFIX 1
#define STATE_INLINE_CHILD    2
#define STATE_INLINE_SIBLING  3

static int64_t parse_value(uint8_t *data, int64_t from, int64_t to, int8_t *state);
static int64_t skip_whitespace(uint8_t *data, int64_t from, int64_t to);
static int64_t skip_whitespace_reverse(uint8_t *data, int64_t from, int64_t to);

static bool is_comment(uint8_t *const data, const int64_t from, const int64_t to) {
	for (int64_t i = from; i < to; i++) {
		if (data[i] > 32) {
			if (data[i] == '#')
				return true;
			return false;
		}
	}
	return false;
}

static void parse_indentation(vb_tab_t *const tab) {
	tab->key_begin = tab->line_begin, tab->indent = 0;
	while (tab->key_begin < tab->line_end && tab->buf->data[tab->key_begin] == '\t') {
		tab->indent++;
		tab->key_begin++;
	}
}

static bool parse_inline_child_prefix(vb_tab_t *const tab) {
	if (tab->key_begin < tab->line_end && tab->buf->data[tab->key_begin] == '\\') {
		const int64_t key_begin1 = tab->key_begin + 1;
		if (key_begin1 < tab->line_end) {
			const uint8_t byte = tab->buf->data[key_begin1];
			if (byte != '\\' && byte != '#' && byte != '|') {
				tab->key_begin += 2;
				tab->indent++;
				return true;
			}
		}
	}
	return false;
}

static int64_t parse_key(uint8_t *const data, const int64_t from, const int64_t to, int8_t *const state) {
	bool escape = false;
	for (int64_t i = from; i < to; i++) {
		const uint8_t byte = data[i];
		if (byte > 32) {
			if (byte == '\\') {
				escape = !escape;
			} else if (byte == '#') {
				if (escape) {
					escape = false;
				} else {
					*state = STATE_NEW_LINE;
					return i;
				}
			} else if (byte == '|') {
				if (escape) {
					escape = false;
				} else {
					*state = STATE_INLINE_SIBLING;
					return i;
				}
			} else if (escape) {
				*state = STATE_INLINE_CHILD;
				return i - 1;
			}
		} else { // whitespace
			return i;
		}
	}
	return to;
}

static void parse_key_value(vb_tab_t *const tab) {
	const int8_t state_old = tab->state;
	tab->key_end = parse_key(tab->buf->data, tab->key_begin, tab->line_end, &tab->state);
	// more to parse
	if (state_old == tab->state) {
		tab->val_begin = skip_whitespace(tab->buf->data, tab->key_end, tab->line_end);
		tab->val_end = parse_value(tab->buf->data, tab->val_begin, tab->line_end, &tab->state);
		tab->next_key_begin = tab->val_end + 1;
		tab->val_end = skip_whitespace_reverse(tab->buf->data, tab->val_begin, tab->val_end);
		if (state_old == tab->state)
			tab->state = STATE_NEW_LINE;
	// end here, inline element next
	} else {
		tab->val_begin = tab->key_end;
		tab->val_end = tab->key_end;
		tab->next_key_begin = tab->key_end + 1;
		tab->key_end = skip_whitespace_reverse(tab->buf->data, tab->key_begin, tab->key_end);
	}
	tab->key_len = tab->key_end - tab->key_begin;
	tab->val_len = tab->val_end - tab->val_begin;
}

static bool parse_line_bounds(vb_tab_t *const tab) {
	const int64_t bytes_len = tab->buf->size;
	for (int64_t i = tab->next_line_begin; i < bytes_len; i++) {
		const uint8_t byte = tab->buf->data[i];
		if (byte == '\r') {
			const int64_t i1 = i + 1;
			if (i1 < bytes_len) {
				tab->line_num++;
				tab->line_begin = tab->next_line_begin, tab->line_end = i;
				if (tab->buf->data[i1] == '\n')
					tab->next_line_begin = i1 + 1;
				else
					tab->next_line_begin = i1;
				return true;
			} else if (tab->parse_last_line) {
				tab->line_num++;
				tab->line_begin = tab->next_line_begin, tab->line_end = i, tab->next_line_begin = i+1;
				return true;
			}
			return false;
		} else if (byte == '\n') {
			tab->line_num++;
			tab->line_begin = tab->next_line_begin, tab->line_end = i, tab->next_line_begin = i+1;
			return true;
		}
	}
	if (tab->parse_last_line) {
		if (tab->next_line_begin < bytes_len) {
			tab->line_begin = tab->next_line_begin;
			tab->line_end = tab->next_line_begin = bytes_len;
			tab->line_num++;
			return true;
		} else {
			tab->key_begin = tab->key_end = bytes_len;
			tab->val_begin = tab->val_end = bytes_len;
			tab->key_len = tab->val_len = 0;
			tab->line_begin = tab->line_end = tab->next_line_begin = bytes_len;
		}
	}
	return false;
}

static int64_t parse_value(uint8_t *const data, const int64_t from, const int64_t to, int8_t *const state) {
	bool escape = false;
	for (int64_t i = from; i < to; i++) {
		const uint8_t byte = data[i];
		if (byte > 32) {
			if (byte == '\\') {
				escape = !escape;
			} else if (byte == '#') {
				if (escape) {
					escape = false;
				} else {
					*state = STATE_NEW_LINE;
					return i;
				}
			} else if (byte == '|') {
				if (escape) {
					escape = false;
				} else {
					*state = STATE_INLINE_SIBLING;
					return i;
				}
			} else if (escape) {
				*state = STATE_INLINE_CHILD;
				return i - 1;
			}
		} else if (escape) {
			*state = STATE_INLINE_CHILD;
			return i - 1;
		}
	}
	return to;
}

static int64_t skip_non_whitespace(uint8_t *const data, const int64_t from, const int64_t to) {
	for (int64_t i = from; i < to; i++)
		if (data[i] <= 32)
			return i;
	return to;
}

static int64_t skip_whitespace(uint8_t *const data, const int64_t from, const int64_t to) {
	for (int64_t i = from; i < to; i++)
		if (data[i] > 32)
			return i;
	return to;
}

static int64_t skip_whitespace_and_char(uint8_t *const data, const int64_t from, const int64_t to, const uint8_t char_to_skip) {
	for (int64_t i = from; i < to; i++) {
		const uint8_t byte = data[i];
		if (byte > 32 && byte != char_to_skip)
			return i;
	}
	return to;
}

static int64_t skip_whitespace_reverse(uint8_t *const data, const int64_t from, const int64_t to) {
	for (int64_t i = to - 1; i >= from; i--)
		if (data[i] > 32)
			return i+1;
	return from;
}

static bool tab_next(vb_tab_t *const tab, const bool no_inline) {
	assert(tab);
	assert(tab->buf);
	if (VB_ERR_NULL(tab->err)) {
		while (true) {
			switch (tab->state) {
			case STATE_NEW_LINE:
				if (parse_line_bounds(tab)) {
					tab->line_len = tab->line_end - tab->line_begin;
					parse_indentation(tab);
					tab->state = STATE_NEW_LINE_PREFIX;
					break;
				} else {
					return false;
				}
			case STATE_NEW_LINE_PREFIX:
				if (no_inline) {
					tab->key_begin = skip_whitespace(tab->buf->data, tab->key_begin, tab->line_end);
					vb_tab_key_val_no_inl(tab);
					return true;
				} else {
					tab->key_begin = skip_whitespace_and_char(tab->buf->data, tab->key_begin, tab->line_end, '|');
					if (is_comment(tab->buf->data, tab->key_begin, tab->line_end)) {
						tab->state = STATE_NEW_LINE;
						break;
					} else {
						if (parse_inline_child_prefix(tab)) {
							tab->state = STATE_NEW_LINE_PREFIX;
							break;
						} else {
							parse_key_value(tab);
							return true;
						}
					}
				}
			case STATE_INLINE_CHILD:
				tab->key_begin = skip_whitespace(tab->buf->data, tab->next_key_begin, tab->line_end);
				if (parse_inline_child_prefix(tab)) {
					tab->key_begin = skip_whitespace(tab->buf->data, tab->key_begin, tab->line_end);
				}
				if (is_comment(tab->buf->data, tab->key_begin, tab->line_end)) {
					tab->state = STATE_NEW_LINE;
					break;
				} else {
					tab->indent++;
					parse_key_value(tab);
					return true;
				}
			case STATE_INLINE_SIBLING:
				tab->key_begin = skip_whitespace_and_char(tab->buf->data, tab->next_key_begin, tab->line_end, '|');
				if (is_comment(tab->buf->data, tab->key_begin, tab->line_end)) {
					tab->state = STATE_NEW_LINE;
					break;
				} else {
					parse_key_value(tab);
					return true;
				}
			}
		}
	}
	return false;
}

void vb_tab_init(vb_tab_t *const tab, vb_buf_t *const buf, vb_err_t **const err) {
	assert(tab);
	*tab = (vb_tab_t){.err = err, .buf = buf};
}

void vb_tab_key_no_inl(vb_tab_t *const tab) {
	assert(tab);
	assert(tab->buf);
	const int8_t state_old = tab->state;
	tab->key_end = skip_non_whitespace(tab->buf->data, tab->key_begin, tab->line_end);
	tab->val_begin = skip_whitespace(tab->buf->data, tab->key_end, tab->line_end);
	tab->val_end = parse_value(tab->buf->data, tab->val_begin, tab->line_end, &tab->state);
	tab->next_key_begin = tab->val_end + 1;
	tab->val_end = skip_whitespace_reverse(tab->buf->data, tab->val_begin, tab->line_end);
	if (state_old == tab->state)
		tab->state = STATE_NEW_LINE;
	tab->key_len = tab->key_end - tab->key_begin;
	tab->val_len = tab->val_end - tab->val_begin;
}

void vb_tab_key_val_no_inl(vb_tab_t *const tab) {
	tab->key_end = skip_non_whitespace(tab->buf->data, tab->key_begin, tab->line_end);
	tab->val_begin = skip_whitespace(tab->buf->data, tab->key_end, tab->line_end);
	tab->key_len = tab->key_end - tab->key_begin;
	vb_tab_val_no_inl(tab);
}

void vb_tab_line_no_inl(vb_tab_t *const tab) {
	parse_indentation(tab);
	tab->key_begin = skip_whitespace(tab->buf->data, tab->key_begin, tab->line_end);
	vb_tab_key_val_no_inl(tab);
}

void vb_tab_list_init(vb_tab_list_t *const tab_list, vb_buf_t *const buf, const int64_t list_begin, const int64_t list_end) {
	assert(tab_list);
	assert(buf);
	assert(list_begin >= 0 && list_begin < buf->size);
	assert(list_begin <= list_end);
	tab_list->buf = buf;
	tab_list->list_begin = list_begin;
	tab_list->list_end = list_end;
	tab_list->entry_begin = list_begin;
	tab_list->entry_end = list_begin;
	tab_list->entry_len = 0;
	tab_list->entry_idx = -1;
	tab_list->separator_begin = list_begin - 1;
}

void vb_tab_list_key_init(vb_tab_list_t *const tab_list, vb_tab_t *const tab) {
	assert(tab_list);
	assert(tab);
	assert(tab->buf);
	assert(tab->key_begin >= 0 && tab->key_begin < tab->buf->size);
	assert(tab->key_begin <= tab->key_end);
	tab_list->buf = tab->buf;
	tab_list->list_begin = tab->key_begin;
	tab_list->list_end = tab->key_end;
	tab_list->entry_begin = tab->key_begin;
	tab_list->entry_end = tab->key_begin;
	tab_list->entry_len = 0;
	tab_list->entry_idx = -1;
	tab_list->separator_begin = tab->key_begin - 1;
}

bool vb_tab_list_next(vb_tab_list_t *const tab_list, const uint8_t separator) {
	assert(tab_list);
	assert(tab_list->buf);
	for (int64_t i = tab_list->separator_begin + 1; i < tab_list->list_end; i++) {
		const uint8_t byte = tab_list->buf->data[i];
		if (byte == separator && separator != ' ') {
			tab_list->entry_begin = i, tab_list->entry_end = i, tab_list->separator_begin = i, tab_list->entry_len = 0;
			tab_list->entry_idx++;
			return true;
		} else if (byte > 32) {
			tab_list->entry_begin = i;
			tab_list->separator_begin = tab_list->list_end;
			for (int64_t j = i + 1; j < tab_list->list_end; j++) {
				if (tab_list->buf->data[j] == separator) {
					if (separator == ' ') {
						const int64_t next_entry_begin = skip_whitespace(tab_list->buf->data, j+1, tab_list->list_end);
						if (next_entry_begin < tab_list->list_end)
							tab_list->separator_begin = next_entry_begin - 1;
					} else {
						tab_list->separator_begin = j;
					}
					break;
				}
			}
			tab_list->entry_end = skip_whitespace_reverse(tab_list->buf->data, i+1, tab_list->separator_begin);
			tab_list->entry_len = tab_list->entry_end - tab_list->entry_begin;
			tab_list->entry_idx++;
			return true;
		}
	}
	tab_list->entry_begin = tab_list->list_end, tab_list->entry_end = tab_list->list_end, tab_list->entry_len = 0;
	if (tab_list->separator_begin < tab_list->list_end) {
		tab_list->separator_begin = tab_list->list_end;
		tab_list->entry_idx++;
		return true;
	}
	return false;
}

void vb_tab_list_val_init(vb_tab_list_t *const tab_list, vb_tab_t *const tab) {
	assert(tab_list);
	assert(tab);
	assert(tab->buf);
	assert(tab->val_begin >= 0 && tab->val_begin < tab->buf->size);
	assert(tab->val_begin <= tab->val_end);
	tab_list->buf = tab->buf;
	tab_list->list_begin = tab->val_begin;
	tab_list->list_end = tab->val_end;
	tab_list->entry_begin = tab->val_begin;
	tab_list->entry_end = tab->val_begin;
	tab_list->entry_len = 0;
	tab_list->entry_idx = -1;
	tab_list->separator_begin = tab->val_begin - 1;
}

bool vb_tab_next(vb_tab_t *const tab) {
	return tab_next(tab, false);
}

bool vb_tab_next_no_inl(vb_tab_t *const tab) {
	return tab_next(tab, true);
}

int64_t vb_tab_reset(vb_tab_t *const tab) {
	const int64_t rest = tab->buf->size - tab->next_line_begin;
	*tab = (vb_tab_t){.err = tab->err, .buf = tab->buf, .line_num = tab->line_num};
	return rest;
}

int64_t vb_tab_rest(vb_tab_t *const tab) {
	return tab->buf->size - tab->next_line_begin;
}

void vb_tab_val_no_inl(vb_tab_t *const tab) {
	tab->val_end = skip_whitespace_reverse(tab->buf->data, tab->val_begin, tab->line_end);
	tab->next_key_begin = tab->line_end + 1;
	tab->val_len = tab->val_end - tab->val_begin;
	tab->state = STATE_NEW_LINE;
}

