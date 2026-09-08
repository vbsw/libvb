/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef VBSW_VB_UTL_H
#define VBSW_VB_UTL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*vb_mem_alloc_t)(void *obj, int64_t size);
typedef void *(*vb_mem_free_t)(void *obj, void *ptr);
typedef void (*vb_mem_destroy_t)(void *obj);

typedef struct {
	vb_mem_alloc_t alloc;
	vb_mem_free_t free;
	vb_mem_destroy_t destroy;
	void *obj;
} vb_mem_t;

#ifdef __cplusplus
}
#endif

#endif /* VBSW_VB_UTL_H */
