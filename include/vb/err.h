/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_ERR
#define VBSW_VB_ERR

#include <stdint.h>

typedef struct { char *str; uint32_t num1, num2; } vb_err_t;

vb_err_t *vb_err_new(uint32_t num1, uint32_t num2, const char *str1, const char *str2);
vb_err_t *vb_err_oom_new(uint32_t num2, const char *str2);
vb_err_t *vb_err_free(vb_err_t *err);

#define VB_ERR_NONE                       0
#define VB_ERR_OUT_OF_MEMORY              1

#endif /* VBSW_VB_ERR */
