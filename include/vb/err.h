/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_ERR_H
#define VBSW_VB_ERR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *str;
	int64_t num1;
	int64_t num2;
} vb_err_t;

vb_err_t *vb_err_new     (int64_t num1, int64_t num2, const char *str1, const char *str2);
vb_err_t *vb_err_new_oom (int64_t num1, int64_t num2, const char *str2);
vb_err_t *vb_err_free    (vb_err_t *err);

#define VB_ERR_NONE                       0
#define VB_ERR(a)                        (a > 0 && a < 5)
#define VB_ERR_OUT_OF_MEMORY              1
#define VB_ERR_STR1_OVERFLOW              2
#define VB_ERR_STR2_OVERFLOW              3
#define VB_ERR_NUM2_OVERFLOW              4

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

#define VB_ERR_FLE(a)                    (a > 22 && a < 29)
#define VB_ERR_FLE_CANT_READ             23
#define VB_ERR_FLE_CANT_CREATE           24
#define VB_ERR_FLE_CANT_DELETE           25
#define VB_ERR_FLE_CANT_CLOSE            26
#define VB_ERR_FLE_CANT_DETERMINE_PATH   27
#define VB_ERR_FLE_OOM                   28

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_ERR_H */
