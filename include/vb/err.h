/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_ERR_H
#define VBSW_VB_ERR_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *next_err;
	const char *str;
	const char *file_name;
	const char *func_name;
	int64_t num1;
	int64_t num2;
	int32_t file_line;
	int32_t file_name_len;
	int32_t func_name_len;
} vb_err_t;

vb_err_t *vb_err_apdf    (vb_err_t **err, vb_err_t *other_err);
vb_err_t *vb_err_new     (vb_err_t **err, int64_t num1, int64_t num2, const char *str1, const char *str2, const char *file_path, const char *func_name, size_t line, size_t len1, size_t len2);
vb_err_t *vb_err_new_oom (vb_err_t **err, int64_t num1, int64_t num2, const char *str2, const char *file_name, const char *file_path, size_t line, size_t len1, size_t len2);
vb_err_t *vb_err_free    (vb_err_t *err);

#define VB_ERR_FFL     (const char*)(__FILE__), (const char*)(__func__), __LINE__, sizeof(__FILE__), sizeof(__func__)
#define VB_ERR_NULL(a) ((a) == NULL || *(a) == NULL)

#define vb_err_new_STR_NULL(e,n1,n2,s1)   vb_err_new(e, n1, n2, (const char*)s1, NULL, VB_ERR_FFL)
#define vb_err_new_STR_STR(e,n1,n2,s1,s2) vb_err_new(e, n1, n2, (const char*)s1, (const char*)s2, VB_ERR_FFL)
#define vb_err_new_oom_NULL(e,n1,n2)      vb_err_new_oom(e, n1, n2, NULL, VB_ERR_FFL)

#define VB_ERR_NONE                       0
#define VB_ERR(a)                        (a > 0 && a < 5)
#define VB_ERR_OUT_OF_MEMORY              1
#define VB_ERR_SIZE_OVERFLOW              2
#define VB_ERR_RESERVED1                  3
#define VB_ERR_RESERVED2                  4

#define VB_ERR_MST(a)                    (a > 4 && a < 9)
#define VB_ERR_MST_UNDERFLOW              5
#define VB_ERR_MST_OVERFLOW               6
#define VB_ERR_MST_LIMIT_EXCEEDED         7
#define VB_ERR_MST_OOM                    8

#define VB_ERR_MHP(a)                    (a > 8 && a < 13)
#define VB_ERR_MHP_UNDERFLOW              9
#define VB_ERR_MHP_OVERFLOW              10
#define VB_ERR_MHP_LIMIT_EXCEEDED        11
#define VB_ERR_MHP_OOM                   12

#define VB_ERR_MAR(a)                    (a > 12 && a < 17)
#define VB_ERR_MAR_UNDERFLOW             13
#define VB_ERR_MAR_OVERFLOW              14
#define VB_ERR_MAR_LIMIT_EXCEEDED        15
#define VB_ERR_MAR_OOM                   16

#define VB_ERR_TME(a)                    (a > 16 && a < 19)
#define VB_ERR_TME_CLOCK                 17
#define VB_ERR_TME_MONO                  18

#define VB_ERR_BUF(a)                    (a > 18 && a < 23)
#define VB_ERR_BUF_UNDERFLOW             19
#define VB_ERR_BUF_OVERFLOW              20
#define VB_ERR_BUF_LIMIT_EXCEEDED        21
#define VB_ERR_BUF_OOM                   22

#define VB_ERR_FLE(a)                    (a > 22 && a < 32)
#define VB_ERR_FLE_CANT_READ             23
#define VB_ERR_FLE_CANT_CREATE           24
#define VB_ERR_FLE_CANT_DELETE           25
#define VB_ERR_FLE_CANT_CLOSE            26
#define VB_ERR_FLE_CANT_DETERMINE_PATH   27
#define VB_ERR_FLE_OOM                   28
#define VB_ERR_FLE_BUFFER_MISSING        29
#define VB_ERR_FLE_UNDERFLOW             30
#define VB_ERR_FLE_OVERFLOW              31

#define VB_ERR_CML(a)                    (a > 31 && a < 35)
#define VB_ERR_CML_OOM                   32
#define VB_ERR_CML_UNDERFLOW             33
#define VB_ERR_CML_OVERFLOW              34

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_ERR_H */
