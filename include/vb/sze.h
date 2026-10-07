/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_SZE_H
#define VBSW_VB_SZE_H

#include <stddef.h>
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VB_SZE_ROUND_UP(a)       (((size_t)(a) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))

#define VB_SZE_MAX_OK_I(a)       ((sizeof(size_t) >= sizeof(int64_t)) ? (size_t)(a) <= (size_t)INT64_MAX               : (int64_t)(a) <= (int64_t)SIZE_MAX)
#define VB_SZE_MAX_OK_A(a,b)     ((sizeof(size_t) >= sizeof(int64_t)) ? (size_t)(a) <= (size_t)INT64_MAX - (size_t)(b) : (int64_t)(a) <= (int64_t)SIZE_MAX - (int64_t)(b))
#define VB_SZE_MAX_OK_M(a,b)     ((sizeof(size_t) >= sizeof(int64_t)) ? (size_t)(a) <= (size_t)INT64_MAX / (size_t)(b) : (int64_t)(a) <= (int64_t)SIZE_MAX / (int64_t)(b))
#define VB_SZE_MAX_OK_AM(a,b,c)  ((sizeof(size_t) >= sizeof(int64_t)) ? (size_t)(a) <= (size_t)INT64_MAX / (size_t)(b) - (size_t)(c)  : (int64_t)(a) <= (int64_t)SIZE_MAX / (int64_t)(b) - (int64_t)(c))
#define VB_SZE_MAX_OK_R(a)       ((sizeof(size_t) >= sizeof(int64_t)) ? (size_t)(a) <= (size_t)INT64_MAX - (alignof(max_align_t) + 1) : (int64_t)(a) <= (int64_t)SIZE_MAX - ((int64_t)alignof(max_align_t) + 1))
#define VB_SZE_MAX_OK_RA(a,b)    (VB_SZE_MAX_OK_R(a) && VB_SZE_MAX_OK_A(VB_SZE_ROUND_UP(a),b))

#define VB_SZE_MAX_ROUNDED       ((sizeof(size_t) >= sizeof(int64_t)) ? (size_t)INT64_MAX & ~(alignof(max_align_t) - 1) : SIZE_MAX & ~(alignof(max_align_t) - 1))

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_SZE_H */
