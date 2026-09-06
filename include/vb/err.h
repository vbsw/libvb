/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_ERR
#define VBSW_VB_ERR

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *str;
	uint32_t num1;
	uint32_t num2;
} vb_err_t;

vb_err_t *vb_err_new(uint32_t num1, uint32_t num2, const char *str1, const char *str2);
vb_err_t *vb_err_new_oom(uint32_t num1, uint32_t num2, const char *str2);
vb_err_t *vb_err_free(vb_err_t *err);

#define VB_ERR_NONE                       0
#define VB_ERR(a)                        (a > 0 && a < 5)
#define VB_ERR_OUT_OF_MEMORY              1
#define VB_ERR_STR1_OVERFLOW              2
#define VB_ERR_STR2_OVERFLOW              3
#define VB_ERR_NUM2_OVERFLOW              4

#define VB_ERR_MST(a)                    (a > 4 && a < 16)
#define VB_ERR_MST_UNDERFLOW              5
#define VB_ERR_MST_OVERFLOW               6
#define VB_ERR_MST_LIMIT_EXCEEDED         7
#define VB_ERR_MST_OOM                    8

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_ERR */
