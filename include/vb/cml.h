/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_CML_H
#define VBSW_VB_CML_H

#include <stdbool.h>
#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char **keys;
	char **vals;
	int64_t *idxs;
	int64_t len;
} vb_cml_args_t;

typedef struct {
	char **seps;
	int64_t *lens;
	int64_t len;
	bool has_empty_sep;
	bool has_space_sep;
} vb_cml_dlmt_t;

typedef struct {
	vb_err_t **err;
	char **args;
	int64_t *lens;
	bool *matched;
	vb_mem_t *mem;
	vb_cml_dlmt_t *dlmt;
	int64_t len;
} vb_cml_t;

bool           vb_cml_args_has_index (vb_cml_args_t *args, int64_t index);
bool           vb_cml_args_has_key   (vb_cml_args_t *args, const char *key);
bool           vb_cml_args_has_value (vb_cml_args_t *args, const char *value);
int64_t        vb_cml_args_index_at  (vb_cml_args_t *args, int64_t index, int64_t fallback);
const char*    vb_cml_args_key_at    (vb_cml_args_t *args, int64_t index, const char *fallback);
const char*    vb_cml_args_value_at  (vb_cml_args_t *args, int64_t index, const char *fallback);

bool           vb_cml_dlmt_new     (vb_cml_t *cml, const char **seps, int64_t seps_count);
vb_cml_t*      vb_cml_deinit       (vb_cml_t *cml);
vb_cml_t*      vb_cml_destroy      (vb_cml_t *cml);
void           vb_cml_init         (vb_cml_t *cml, const char **args, int64_t args_count, vb_mem_t *mem, vb_err_t **err);
vb_cml_args_t* vb_cml_match        (vb_cml_t *cml, const char **args, int64_t args_count);
vb_cml_args_t* vb_cml_match_dlmt   (vb_cml_t *cml, const char **args, int64_t args_count);
vb_cml_t*      vb_cml_new          (const char **args, int64_t args_count, vb_mem_t *mem, vb_err_t **err);
void           vb_cml_undo_matched (vb_cml_t *cml, vb_cml_args_t **args, int64_t args_count);
vb_cml_args_t* vb_cml_unmatched    (vb_cml_t *cml);

static inline bool vb_cml_dlmt_new_1(vb_cml_t *cml, const char *sep1) {
	return vb_cml_dlmt_new(cml, (const char *[]){sep1}, 1);
}
static inline bool vb_cml_dlmt_new_2(vb_cml_t *cml, const char *sep1, const char *sep2) {
	return vb_cml_dlmt_new(cml, (const char *[]){sep1, sep2}, 2);
}
static inline bool vb_cml_dlmt_new_3(vb_cml_t *cml, const char *sep1, const char *sep2, const char *sep3) {
	return vb_cml_dlmt_new(cml, (const char *[]){sep1, sep2, sep3}, 3);
}
static inline bool vb_cml_dlmt_new_4(vb_cml_t *cml, const char *sep1, const char *sep2, const char *sep3, const char *sep4) {
	return vb_cml_dlmt_new(cml, (const char *[]){sep1, sep2, sep3, sep4}, 4);
}
static inline bool vb_cml_dlmt_new_5(vb_cml_t *cml, const char *sep1, const char *sep2, const char *sep3, const char *sep4, const char *sep5) {
	return vb_cml_dlmt_new(cml, (const char *[]){sep1, sep2, sep3, sep4, sep5}, 5);
}
static inline bool vb_cml_dlmt_new_6(vb_cml_t *cml, const char *sep1, const char *sep2, const char *sep3, const char *sep4, const char *sep5, const char *sep6) {
	return vb_cml_dlmt_new(cml, (const char *[]){sep1, sep2, sep3, sep4, sep5, sep6}, 6);
}
static inline bool vb_cml_dlmt_new_7(vb_cml_t *cml, const char *sep1, const char *sep2, const char *sep3, const char *sep4, const char *sep5, const char *sep6, const char *sep7) {
	return vb_cml_dlmt_new(cml, (const char *[]){sep1, sep2, sep3, sep4, sep5, sep6, sep7}, 7);
}
static inline bool vb_cml_dlmt_new_8(vb_cml_t *cml, const char *sep1, const char *sep2, const char *sep3, const char *sep4, const char *sep5, const char *sep6, const char *sep7, const char *sep8) {
	return vb_cml_dlmt_new(cml, (const char *[]){sep1, sep2, sep3, sep4, sep5, sep6, sep7, sep8}, 8);
}

static inline vb_cml_args_t* vb_cml_match_1(vb_cml_t *cml, const char *arg1) {
	return vb_cml_match(cml, (const char *[]){arg1}, 1);
}
static inline vb_cml_args_t* vb_cml_match_2(vb_cml_t *cml, const char *arg1, const char *arg2) {
	return vb_cml_match(cml, (const char *[]){arg1, arg2}, 2);
}
static inline vb_cml_args_t* vb_cml_match_3(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3) {
	return vb_cml_match(cml, (const char *[]){arg1, arg2, arg3}, 3);
}
static inline vb_cml_args_t* vb_cml_match_4(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4) {
	return vb_cml_match(cml, (const char *[]){arg1, arg2, arg3, arg4}, 4);
}
static inline vb_cml_args_t* vb_cml_match_5(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5) {
	return vb_cml_match(cml, (const char *[]){arg1, arg2, arg3, arg4, arg5}, 5);
}
static inline vb_cml_args_t* vb_cml_match_6(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6) {
	return vb_cml_match(cml, (const char *[]){arg1, arg2, arg3, arg4, arg5, arg6}, 6);
}
static inline vb_cml_args_t* vb_cml_match_7(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7) {
	return vb_cml_match(cml, (const char *[]){arg1, arg2, arg3, arg4, arg5, arg6, arg7}, 7);
}
static inline vb_cml_args_t* vb_cml_match_8(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7, const char *arg8) {
	return vb_cml_match(cml, (const char *[]){arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8}, 8);
}

static inline vb_cml_args_t* vb_cml_match_dlmt_1(vb_cml_t *cml, const char *arg1) {
	return vb_cml_match_dlmt(cml, (const char *[]){arg1}, 1);
}
static inline vb_cml_args_t* vb_cml_match_dlmt_2(vb_cml_t *cml, const char *arg1, const char *arg2) {
	return vb_cml_match_dlmt(cml, (const char *[]){arg1, arg2}, 2);
}
static inline vb_cml_args_t* vb_cml_match_dlmt_3(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3) {
	return vb_cml_match_dlmt(cml, (const char *[]){arg1, arg2, arg3}, 3);
}
static inline vb_cml_args_t* vb_cml_match_dlmt_4(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4) {
	return vb_cml_match_dlmt(cml, (const char *[]){arg1, arg2, arg3, arg4}, 4);
}
static inline vb_cml_args_t* vb_cml_match_dlmt_5(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5) {
	return vb_cml_match_dlmt(cml, (const char *[]){arg1, arg2, arg3, arg4, arg5}, 5);
}
static inline vb_cml_args_t* vb_cml_match_dlmt_6(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6) {
	return vb_cml_match_dlmt(cml, (const char *[]){arg1, arg2, arg3, arg4, arg5, arg6}, 6);
}
static inline vb_cml_args_t* vb_cml_match_dlmt_7(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7) {
	return vb_cml_match_dlmt(cml, (const char *[]){arg1, arg2, arg3, arg4, arg5, arg6, arg7}, 7);
}
static inline vb_cml_args_t* vb_cml_match_dlmt_8(vb_cml_t *cml, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7, const char *arg8) {
	return vb_cml_match_dlmt(cml, (const char *[]){arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8}, 8);
}

static inline void vb_cml_undo_matched_1(vb_cml_t *const cml, vb_cml_args_t *args1) {
	vb_cml_undo_matched(cml, (vb_cml_args_t*[]){args1}, 1);
}
static inline void vb_cml_undo_matched_2(vb_cml_t *const cml, vb_cml_args_t *args1, vb_cml_args_t *args2) {
	vb_cml_undo_matched(cml, (vb_cml_args_t*[]){args1, args2}, 2);
}
static inline void vb_cml_undo_matched_3(vb_cml_t *const cml, vb_cml_args_t *args1, vb_cml_args_t *args2, vb_cml_args_t *args3) {
	vb_cml_undo_matched(cml, (vb_cml_args_t*[]){args1, args2, args3}, 3);
}
static inline void vb_cml_undo_matched_4(vb_cml_t *const cml, vb_cml_args_t *args1, vb_cml_args_t *args2, vb_cml_args_t *args3, vb_cml_args_t *args4) {
	vb_cml_undo_matched(cml, (vb_cml_args_t*[]){args1, args2, args3, args4}, 4);
}
static inline void vb_cml_undo_matched_5(vb_cml_t *const cml, vb_cml_args_t *args1, vb_cml_args_t *args2, vb_cml_args_t *args3, vb_cml_args_t *args4, vb_cml_args_t *args5) {
	vb_cml_undo_matched(cml, (vb_cml_args_t*[]){args1, args2, args3, args4, args5}, 5);
}
static inline void vb_cml_undo_matched_6(vb_cml_t *const cml, vb_cml_args_t *args1, vb_cml_args_t *args2, vb_cml_args_t *args3, vb_cml_args_t *args4, vb_cml_args_t *args5, vb_cml_args_t *args6) {
	vb_cml_undo_matched(cml, (vb_cml_args_t*[]){args1, args2, args3, args4, args5, args6}, 6);
}
static inline void vb_cml_undo_matched_7(vb_cml_t *const cml, vb_cml_args_t *args1, vb_cml_args_t *args2, vb_cml_args_t *args3, vb_cml_args_t *args4, vb_cml_args_t *args5, vb_cml_args_t *args6, vb_cml_args_t *args7) {
	vb_cml_undo_matched(cml, (vb_cml_args_t*[]){args1, args2, args3, args4, args5, args6, args7}, 7);
}
static inline void vb_cml_undo_matched_8(vb_cml_t *const cml, vb_cml_args_t *args1, vb_cml_args_t *args2, vb_cml_args_t *args3, vb_cml_args_t *args4, vb_cml_args_t *args5, vb_cml_args_t *args6, vb_cml_args_t *args7, vb_cml_args_t *args8) {
	vb_cml_undo_matched(cml, (vb_cml_args_t*[]){args1, args2, args3, args4, args5, args6, args7, args8}, 8);
}

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_CML_H */
