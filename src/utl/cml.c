/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <vb/sze.h>
#include <vb/cml.h>

typedef struct {
	int64_t len, cap;
	int64_t *matched_idxs;
	int64_t *keys_lens;
	int64_t *vals_lens;
	uint8_t **keys;
	uint8_t **vals;
	bool all_matched;
} dlmt_analysis_t;

#define DLMT_ANALYSIS_BUF_SIZE 8

#define FREE_MATCHED_IDX if (matched_idx != matched_buf) { if (cml->mem) VB_MEM_FREE(cml->mem, matched_idx); else free(matched_idx); }

#define HEAD_T_SIZE  VB_SZE_ROUND_UP(sizeof(vb_cml_t))
#define DLMT_T_SIZE  VB_SZE_ROUND_UP(sizeof(vb_cml_dlmt_t))
#define ARGS_T_SIZE  VB_SZE_ROUND_UP(sizeof(vb_cml_args_t))

static bool cml_init(vb_cml_t *const cml, const uint8_t **const args, const int64_t args_count, vb_mem_t *const mem, vb_err_t **const err, int64_t *const lens, const int64_t err_off) {
	bool success = true;
	size_t args_size = 0;
	for (int64_t i = 0; i < args_count && success; i++) {
		const size_t arg_size0 = strlen(args[i]) + 1;
		if (args_size <= SIZE_MAX - arg_size0) {
			args_size += arg_size0;
			if (VB_SZE_MAX_OK_I(arg_size0)) {
				lens[i] = (int64_t)arg_size0 - 1;
			} else {
				vb_err_new(err, VB_ERR_CML_OVERFLOW, 4+(err_off*6), "total arguments size overflow", NULL, VB_ERR_FFL);
				success = false;
			}
		} else {
			vb_err_new(err, VB_ERR_CML_OVERFLOW, 3+(err_off*6), "total arguments size overflow", NULL, VB_ERR_FFL);
			success = false;
		}
	}
	if (success) {
		if (VB_SZE_MAX_OK_I(args_size) && VB_SZE_MAX_OK_M(args_count, sizeof(uint8_t*))) {
			size_t args_ptrs_size = (size_t)args_count * sizeof(uint8_t*);
			if (VB_SZE_MAX_OK_A(args_size, args_ptrs_size)) {
				args_size += args_ptrs_size;
				if (VB_SZE_MAX_OK_AM(args_count, 1, sizeof(bool))) {
					const size_t matched_size1 = (size_t)(args_count+1) * sizeof(bool);
					if (VB_SZE_MAX_OK_A(args_size, matched_size1)) {
						args_size += matched_size1;
						uint8_t **const cml_args = mem ? VB_MEM_ALLOC(mem, (int64_t)args_size) : malloc(args_size);
						if (cml_args) {
							uint8_t *const strs = (uint8_t*)&(cml_args)[args_count];
							size_t off = 0;
							for (int64_t i = 0; i < args_count; i++) {
								uint8_t *const str = &strs[off];
								const size_t str_len0 = (size_t)lens[i]+1;
								memcpy(str, args[i], str_len0);
								cml_args[i] = str;
								off += str_len0;
							}
							bool *const matched = (bool*)&(strs)[off];
							memset(matched, false, args_count+1);
							cml->err = err;
							cml->args = cml_args;
							cml->lens = lens;
							cml->matched = matched;
							cml->mem = mem;
							cml->dlmt = NULL;
							cml->len = args_count;
						} else {
							vb_err_new_oom(err, VB_ERR_CML_OOM, 4+(err_off*1), NULL, VB_ERR_FFL);
							success = false;
						}
					} else {
						vb_err_new(err, VB_ERR_CML_OVERFLOW, 8+(err_off*6), "total arguments size overflow", NULL, VB_ERR_FFL);
						success = false;
					}
				} else {
					vb_err_new(err, VB_ERR_CML_OVERFLOW, 7+(err_off*6), "total arguments size overflow", NULL, VB_ERR_FFL);
					success = false;
				}
			} else {
				vb_err_new(err, VB_ERR_CML_OVERFLOW, 6+(err_off*6), "total arguments size overflow", NULL, VB_ERR_FFL);
				success = false;
			}
		} else {
			vb_err_new(err, VB_ERR_CML_OVERFLOW, 5+(err_off*6), "total arguments size overflow", NULL, VB_ERR_FFL);
			success = false;
		}
	}
	return success;
}

static void dlmt_analysis_expand(vb_cml_t *const cml, dlmt_analysis_t *const analysis, vb_err_t **const err, const int64_t err_off) {
	int64_t *const matched_idxs = analysis->matched_idxs;
	const size_t len_new = analysis->len * 2;
	const size_t ints_len = len_new * 3;
	if (VB_SZE_MAX_OK_MR(ints_len, sizeof(int64_t))) {
		const size_t ptrs_len = len_new * 2;
		if (VB_SZE_MAX_OK_MR(ptrs_len, sizeof(int64_t*))) {
			const size_t ints_size_up = VB_SZE_ROUND_UP(ints_len * sizeof(int64_t));
			const size_t ptrs_size_up = VB_SZE_ROUND_UP(ptrs_len * sizeof(int64_t*));
			if (VB_SZE_MAX_OK_A(ints_size_up, ptrs_size_up)) {
				const size_t size_up = ints_size_up + ptrs_size_up;
				int64_t *const matched_idxs_new = cml->mem ? VB_MEM_ALLOC(cml->mem, (int64_t)size_up) : malloc(size_up);
				if (matched_idxs_new) {
					uint8_t **const keys = (uint8_t**)(((int8_t*)matched_idxs_new) + ints_size_up);
					memcpy(matched_idxs_new, analysis->matched_idxs, (size_t)(analysis->len * 3) * sizeof(int64_t));
					memcpy(keys, analysis->keys, (size_t)(analysis->len * 2) * sizeof(int64_t*));
					analysis->matched_idxs = matched_idxs_new;
					analysis->keys_lens = &matched_idxs_new[len_new];
					analysis->vals_lens = &matched_idxs_new[len_new*2];
					analysis->keys = keys;
					analysis->vals = &analysis->keys[len_new];
					analysis->cap = len_new;
				} else {
					vb_err_new_oom(err, VB_ERR_CML_OOM, 11, NULL, VB_ERR_FFL);
				}
			} else {
				vb_err_new(err, VB_ERR_CML_OVERFLOW, 34+(err_off*3), "matched arguments size overflow", NULL, VB_ERR_FFL);
			}
		} else {
			vb_err_new(err, VB_ERR_CML_OVERFLOW, 33+(err_off*3), "matched arguments size overflow", NULL, VB_ERR_FFL);
		}
	} else {
		vb_err_new(err, VB_ERR_CML_OVERFLOW, 32+(err_off*3), "matched arguments size overflow", NULL, VB_ERR_FFL);
	}
	if (analysis->len != DLMT_ANALYSIS_BUF_SIZE) {
		if (cml->mem)
			VB_MEM_FREE(cml->mem, matched_idxs);
		else
			free(matched_idxs);
	}
	if (*err)
		analysis->len = 0;
}

static size_t is_prefix(const uint8_t *const str, const uint8_t *const prefix) {
	assert(str[0] != '\0' && prefix[0] != '\0'); // empty strings don't belong here
	for (size_t i = 0; str[i] == prefix[i];)
		if (prefix[++i] == '\0')
			return i;
	return 0;
}

static void match_dlmt_analysis(vb_cml_t *const cml, const uint8_t **const args, const int64_t args_count, dlmt_analysis_t *const analysis) {
	vb_err_t *err = NULL;
	const bool has_empty_sep = cml->dlmt->has_empty_sep;
	const bool has_space_sep = cml->dlmt->has_space_sep;
	for (int64_t i = 0; i < cml->len; i++) {
		if (!cml->matched[i]) {
			bool matched = false;
			for (int64_t j = 0; !matched && j < args_count; j++) {
				const uint8_t *const arg = args[j];
				const size_t prefix_len = is_prefix(cml->args[i], arg);
				if (prefix_len) {
					if (cml->lens[i] > prefix_len) {
						for (int64_t k = 0; k < cml->dlmt->len; k++) {
							const size_t sep_len = is_prefix(cml->args[i] + prefix_len, cml->dlmt->seps[k]);
							if (sep_len || has_empty_sep) {
								if (analysis->len == analysis->cap)
									dlmt_analysis_expand(cml, analysis, &err, 0);
								if (err == NULL) {
									const size_t val_off = prefix_len + sep_len;
									analysis->matched_idxs[analysis->len] = i;
									analysis->keys_lens[analysis->len] = prefix_len;
									analysis->vals_lens[analysis->len] = cml->lens[i] - (int64_t)val_off;
									analysis->keys[analysis->len] = cml->args[i];
									analysis->vals[analysis->len] = cml->args[i] + val_off;
									analysis->len++;
									matched = true;
									break;
								} else {
									vb_err_apdf(cml->err, err);
									return;
								}
							}
						}
					} else if (cml->lens[i] == prefix_len && (has_empty_sep || has_space_sep)) {
						if (analysis->len == analysis->cap)
							dlmt_analysis_expand(cml, analysis, &err, 1);
						if (err == NULL) {
							analysis->matched_idxs[analysis->len] = i;
							analysis->keys_lens[analysis->len] = prefix_len;
							analysis->keys[analysis->len] = cml->args[i];
							if (has_space_sep) {
								const int64_t i1 = i + 1;
								if (i1 < cml->len) {
									analysis->vals_lens[analysis->len] = -cml->lens[i1] - 1;
									analysis->vals[analysis->len] = cml->args[i1];
									i++;
								} else {
									analysis->vals_lens[analysis->len] = 0;
								}
							} else {
								analysis->vals_lens[analysis->len] = 0;
							}
							analysis->len++;
							matched = true;
						} else {
							vb_err_apdf(cml->err, err);
							return;
						}
					}
				}
			}
			analysis->all_matched = analysis->all_matched && matched;
		}
	}
}

bool vb_cml_args_has_index(vb_cml_args_t *const args, const int64_t index) {
	if (args) {
		assert(args->idxs);
		for (int64_t i = 0; i < args->len; i++) {
			if (args->idxs[i] == index)
				return true;
		}
	}
	return false;
}

bool vb_cml_args_has_key(vb_cml_args_t *const args, const uint8_t *const key) {
	if (args) {
		assert(key);
		assert(args->keys);
		for (int64_t i = 0; i < args->len; i++) {
			assert(args->keys[i]);
			if (strcmp(args->keys[i], key) == 0)
				return true;
		}
	}
	return false;
}

bool vb_cml_args_has_value(vb_cml_args_t *const args, const uint8_t *const value) {
	if (args) {
		assert(value);
		assert(args->vals);
		for (int64_t i = 0; i < args->len; i++) {
			assert(args->vals[i]);
			if (strcmp(args->vals[i], value) == 0)
				return true;
		}
	}
	return false;
}

int64_t vb_cml_args_index_at(vb_cml_args_t *const args, const int64_t index, const int64_t fallback) {
	if (args && index >= 0 && index < args->len) {
		assert(args->idxs);
		return args->idxs[index];
	}
	return fallback;
}

const uint8_t* vb_cml_args_key_at(vb_cml_args_t *const args, const int64_t index, const uint8_t *const fallback) {
	if (args && index >= 0 && index < args->len) {
		assert(args->keys);
		return (const uint8_t*)args->keys[index];
	}
	return fallback;
}

const uint8_t* vb_cml_args_value_at(vb_cml_args_t *const args, const int64_t index, const uint8_t *const fallback) {
	if (args && index >= 0 && index < args->len) {
		assert(args->vals);
		return (const uint8_t*)args->vals[index];
	}
	return fallback;
}

vb_cml_t *vb_cml_deinit(vb_cml_t *const cml) {
	assert(cml);
	if (VB_ERR_NULL(cml->err)) {
		if (cml->mem) {
			if (cml->args) // frees matched, too
				VB_MEM_FREE(cml->mem, cml->args);
			if (cml->dlmt)
				VB_MEM_FREE(cml->mem, cml->dlmt);
			if (cml->lens)
				VB_MEM_FREE(cml->mem, cml->lens);
		} else {
			if (cml->args) // frees matched, too
				free(cml->args);
			if (cml->dlmt)
				free(cml->dlmt);
			if (cml->lens)
				free(cml->lens);
		}
		cml->args = NULL;
		cml->lens = NULL;
		cml->matched = NULL;
		cml->dlmt = NULL;
		cml->len = 0;
	}
	return cml;
}

vb_cml_t *vb_cml_destroy(vb_cml_t *const cml) {
	assert(cml);
	if (VB_ERR_NULL(cml->err)) {
		if (cml->mem) {
			if (cml->args)
				VB_MEM_FREE(cml->mem, cml->args);
			if (cml->dlmt)
				VB_MEM_FREE(cml->mem, cml->dlmt);
			VB_MEM_FREE(cml->mem, cml);
		} else {
			if (cml->args)
				free(cml->args);
			if (cml->dlmt)
				free(cml->dlmt);
			free(cml);
		}
		return NULL;
	}
	return cml;
}

bool vb_cml_dlmt_new (vb_cml_t *const cml, const uint8_t **const seps, const int64_t seps_len) {
	bool ret_val = false;
	if (VB_ERR_NULL(cml->err) && (seps || seps_len > 0)) {
		assert(seps);
		assert(seps_len > 0);
		int64_t lens_buf[8] = {0};
		int64_t *lens = NULL;
		size_t lens_size = 0;
		if (seps_len <= 8) {
			lens = lens_buf;
			lens_size = (size_t)seps_len * sizeof(int64_t);
		} else if (VB_SZE_MAX_OK_M(seps_len, sizeof(int64_t))) {
			lens_size = (size_t)seps_len * sizeof(int64_t);
			lens = cml->mem ? VB_MEM_ALLOC(cml->mem, lens_size) : malloc((size_t)lens_size);
			if (!lens)
				vb_err_new_oom(cml->err, VB_ERR_CML_OOM, 6, NULL, VB_ERR_FFL);
		} else {
			vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 15, "separators count overflow", NULL, VB_ERR_FFL);
		}
		if (lens) {
			size_t seps_size = 0;
			size_t count = 0;
			bool has_empty_sep = false;
			bool has_space_sep = false;
			for (int64_t i = 0; i < seps_len; i++) {
				const uint8_t *const separator = seps[i];
				assert(separator);
				const size_t sep_size = strlen(separator);
				if (VB_SZE_MAX_OK_A(seps_size, sep_size)) {
					if (sep_size == 0) {
						has_empty_sep = true;
					} else if (sep_size == 1 && separator[0] == ' ') {
						has_space_sep = true;
					} else {
						seps_size += sep_size;
						count++;
					}
					lens[i] = (int64_t)sep_size;
				} else {
					vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 16, "separators total size overflow", NULL, VB_ERR_FFL);
					seps_size = 0;
					break;
				}
			}
			if (seps_size) {
				if (VB_SZE_MAX_OK_A(seps_size, lens_size)) {
					size_t dlmt_size = seps_size + lens_size;
					if (VB_SZE_MAX_OK_MR(count, sizeof(uint8_t*))) {
						const int64_t ptrs_size_up = VB_SZE_ROUND_UP(count * sizeof(uint8_t*));
						if (VB_SZE_MAX_OK_AR(dlmt_size, ptrs_size_up)) {
							dlmt_size += ptrs_size_up;
							if (VB_SZE_MAX_OK_AR(dlmt_size, DLMT_T_SIZE)) {
								dlmt_size += (int64_t)DLMT_T_SIZE;
								vb_cml_dlmt_t *const dlmt = cml->mem ? VB_MEM_ALLOC(cml->mem, dlmt_size) : malloc((size_t)dlmt_size);
								if (dlmt) {
									if (count) {
										uint8_t **const dlmt_seps = (uint8_t**)((int8_t*)dlmt + DLMT_T_SIZE);
										int64_t *const dlmt_lens = (int64_t*)((int8_t*)dlmt_seps + ptrs_size_up);
										uint8_t *const strs = (uint8_t*)&dlmt_lens[count];
										for (int64_t i = 0, j = 0, strs_off = 0; i < seps_len; i++) {
											const int64_t sep_len = lens[i];
											const int64_t sep_len0 = sep_len+1;
											if (sep_len > 0 && !(sep_len == 1 && seps[i][0] == ' ')) {
												uint8_t *const str = &strs[strs_off];
												memcpy(str, seps[i], (size_t)sep_len0);
												dlmt_seps[j] = str;
												dlmt_lens[j++] = sep_len;
												strs_off += sep_len0;
											}
										}
										dlmt->seps = dlmt_seps;
										dlmt->lens = dlmt_lens;
									} else {
										dlmt->seps = NULL;
										dlmt->lens = NULL;
									}
									dlmt->len = (int64_t)count;
									dlmt->has_empty_sep = has_empty_sep;
									dlmt->has_space_sep = has_space_sep;
									cml->dlmt = dlmt;
									ret_val = true;
								} else {
									vb_err_new_oom(cml->err, VB_ERR_CML_OOM, 7, NULL, VB_ERR_FFL);
								}
							} else {
								vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 20, "separators total size overflow", NULL, VB_ERR_FFL);
							}
						} else {
							vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 19, "separators total size overflow", NULL, VB_ERR_FFL);
						}
					} else {
						vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 18, "separators total size overflow", NULL, VB_ERR_FFL);
					}
				} else {
					vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 17, "separators total size overflow", NULL, VB_ERR_FFL);
				}
			}
			if (lens != lens_buf)
				free(lens);
		}
	}
	return ret_val;
}

void vb_cml_init(vb_cml_t *const cml, const uint8_t **const args, const int64_t args_count, vb_mem_t *const mem, vb_err_t **const err) {
	assert(cml);
	if (VB_ERR_NULL(err)) {
		if (args_count > 0) {
			assert(args);
			if (VB_SZE_MAX_OK_M(args_count, sizeof(int64_t))) {
				const size_t lens_size = (size_t)args_count * sizeof(int64_t);
				int64_t *const lens = mem ? VB_MEM_ALLOC(mem, (int64_t)lens_size) : malloc(lens_size);
				if (lens) {
					if (!cml_init(cml, args, args_count, mem, err, lens, 1)) {
						if (mem)
							VB_MEM_FREE(mem, lens);
						else
							free(lens);
					}
				} else {
					vb_err_new_oom(err, VB_ERR_CML_OOM, 3, NULL, VB_ERR_FFL);
				}
			} else {
				vb_err_new(err, VB_ERR_CML_OVERFLOW, 2, "arguments count overflow", NULL, VB_ERR_FFL);
			}
		} else if (args_count == 0) {
			assert(args == NULL);
			*cml = (vb_cml_t){.err = err, .mem = mem};
		} else {
			vb_err_new(err, VB_ERR_CML_UNDERFLOW, 2, "arguments count underflow", NULL, VB_ERR_FFL);
		}
	}
}

vb_cml_args_t *vb_cml_match(vb_cml_t *const cml, const uint8_t **const args, const int64_t args_count) {
	assert(cml);
	vb_cml_args_t *ret_val = NULL;
	if (VB_ERR_NULL(cml->err)) {
		if (args_count > 0) {
			assert(args);
			if (cml->len && !cml->matched[cml->len]) {
				bool all_matched = true;
				size_t matched_buf[8], matched_len = 0, matched_cap = 8;
				size_t *matched_idx = matched_buf;
				for (size_t i = 0; i < cml->len; i++) {
					if (!cml->matched[i]) {
						bool matched = false;
						for (int64_t j = 0; j < args_count; j++) {
							const uint8_t *const arg = args[j];
							if (strcmp(cml->args[i], arg) == 0) {
								matched = true;
								if (matched_len < matched_cap) {
									matched_idx[matched_len++] = i;
								} else {
									if (VB_SZE_MAX_OK_M(matched_cap, 2*sizeof(int64_t))) {
										matched_cap *= 2;
										const size_t matched_idx_size = matched_cap * sizeof(int64_t);
										size_t *const matched_idx_new = cml->mem ? VB_MEM_ALLOC(cml->mem, (int64_t)matched_idx_size) : malloc(matched_idx_size);
										if (matched_idx_new) {
											memcpy(matched_idx_new, matched_idx, matched_len*sizeof(int64_t));
											FREE_MATCHED_IDX
											matched_idx = matched_idx_new;
											matched_idx[matched_len++] = i;
											break;
										} else {
											FREE_MATCHED_IDX
											vb_err_new_oom(cml->err, VB_ERR_CML_OOM, 8, NULL, VB_ERR_FFL);
											return NULL;
										}
									} else {
										FREE_MATCHED_IDX
										vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 21, "matched count overflow", NULL, VB_ERR_FFL);
										return NULL;
									}
								}
							}
						}
						all_matched = all_matched && matched;
					}
				}
				if (matched_len) {
					const size_t ptr_size = sizeof(uint8_t*) * matched_len;
					if (VB_SZE_MAX_OK_MR(ptr_size, 2)) {
						const size_t ptr_size_up = VB_SZE_ROUND_UP(ptr_size*2);
						size_t cml_args_size = ptr_size_up;
						const size_t idxs_size = sizeof(int64_t) * matched_len;
						if (VB_SZE_MAX_OK_A(cml_args_size, idxs_size)) {
							cml_args_size += idxs_size;
							if (VB_SZE_MAX_OK_A(cml_args_size, ARGS_T_SIZE)) {
								cml_args_size += ARGS_T_SIZE;
								for (size_t i = 0; i < matched_len; i++) {
									const size_t cml_idx = matched_idx[i];
									const size_t str_len0 = (size_t)cml->lens[cml_idx]+1;
									if (VB_SZE_MAX_OK_A(cml_args_size, str_len0)) {
										cml_args_size += str_len0;
									} else {
										FREE_MATCHED_IDX
										vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 22, "matched arguments size overflow", NULL, VB_ERR_FFL);
										return NULL;
									}
								}
								ret_val = cml->mem ? VB_MEM_ALLOC(cml->mem, (int64_t)cml_args_size) : malloc(cml_args_size);
								if (ret_val) {
									uint8_t **const keys = (uint8_t**)((int8_t*)ret_val + ARGS_T_SIZE);
									uint8_t **const vals = &keys[matched_len];
									int64_t *const idxs = (int64_t*)((int8_t*)keys + ptr_size_up);
									uint8_t *const strs = (uint8_t*)((int8_t*)idxs + idxs_size);
									for (size_t i = 0, strs_off = 0; i < matched_len; i++) {
										const size_t cml_idx = matched_idx[i];
										uint8_t *const str = &strs[strs_off];
										const size_t str_size0 = (size_t)(cml->lens[cml_idx]+1);
										memcpy(str, cml->args[cml_idx], str_size0);
										keys[i] = str;
										vals[i] = &str[cml->lens[cml_idx]]; // '\0'
										idxs[i] = (int64_t)cml_idx;
										strs_off += str_size0;
										cml->matched[cml_idx] = true;
									}
									ret_val->keys = keys;
									ret_val->vals = vals;
									ret_val->idxs = idxs;
									ret_val->len = (int64_t)matched_len;
									cml->matched[cml->len] = all_matched;
									FREE_MATCHED_IDX
								} else {
									FREE_MATCHED_IDX
									vb_err_new_oom(cml->err, VB_ERR_CML_OOM, 9, NULL, VB_ERR_FFL);
								}
							} else {
								FREE_MATCHED_IDX
								vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 25, "matched arguments size overflow", NULL, VB_ERR_FFL);
							}
						} else {
							FREE_MATCHED_IDX
							vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 24, "matched arguments size overflow", NULL, VB_ERR_FFL);
						}
					} else {
						FREE_MATCHED_IDX
						vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 23, "matched arguments size overflow", NULL, VB_ERR_FFL);
					}
				}
			}
		} else if (args_count == 0) {
			assert(args == NULL);
		} else {
			vb_err_new(cml->err, VB_ERR_CML_UNDERFLOW, 3, "arguments count underflow", NULL, VB_ERR_FFL);
		}
	}
	return ret_val;
}

vb_cml_args_t *vb_cml_match_dlmt(vb_cml_t *const cml, const uint8_t **const args, const int64_t args_count) {
	assert(cml);
	vb_cml_args_t *ret_val = NULL;
	if (VB_ERR_NULL(cml->err)) {
		if (args_count > 0) {
			assert(args);
			if (cml->len && !cml->matched[cml->len]) {
				assert(cml->dlmt);
				int64_t  buf_ints[DLMT_ANALYSIS_BUF_SIZE*3];
				uint8_t* buf_ptrs[DLMT_ANALYSIS_BUF_SIZE*2];
				dlmt_analysis_t dlmt_analysis = {0, DLMT_ANALYSIS_BUF_SIZE, buf_ints, &buf_ints[DLMT_ANALYSIS_BUF_SIZE], &buf_ints[DLMT_ANALYSIS_BUF_SIZE*2], buf_ptrs, &buf_ptrs[DLMT_ANALYSIS_BUF_SIZE], true};
				match_dlmt_analysis(cml, args, args_count, &dlmt_analysis);
				assert(dlmt_analysis.len >= 0);

				if (dlmt_analysis.len > 0) {
					const size_t ptr_size_up = VB_SZE_ROUND_UP((size_t)dlmt_analysis.len * sizeof(uint8_t*) * 2);
					const size_t idxs_size = (size_t)dlmt_analysis.len * sizeof(int64_t);
					size_t cml_args_size = ptr_size_up + idxs_size;
					vb_err_t *err = NULL;

					for (int64_t i = 0; i < dlmt_analysis.len; i++) {
						const size_t key_len0 = (size_t)(dlmt_analysis.keys_lens[i]+1);
						if (VB_SZE_MAX_OK_A(cml_args_size, key_len0)) {
							const size_t val_len0 = (size_t)(dlmt_analysis.vals_lens[i]+1) + dlmt_analysis.matched_idxs[i] >= 0 ? 0 : 1;
							cml_args_size += key_len0;
							if (VB_SZE_MAX_OK_A(cml_args_size, val_len0)) {
								cml_args_size += val_len0;
							} else {
								vb_err_new(&err, VB_ERR_CML_OVERFLOW, 30, "matched arguments size overflow", NULL, VB_ERR_FFL);
								break;
							}
						} else {
							vb_err_new(&err, VB_ERR_CML_OVERFLOW, 29, "matched arguments size overflow", NULL, VB_ERR_FFL);
							break;
						}
					}
					if (err == NULL) {
						if (VB_SZE_MAX_OK_A(cml_args_size, ARGS_T_SIZE)) {
							cml_args_size += ARGS_T_SIZE;
							ret_val = malloc(cml_args_size);
							if (ret_val) {
								ret_val->keys = (uint8_t**)(((int8_t*)ret_val) + ARGS_T_SIZE);
								ret_val->vals = &ret_val->keys[dlmt_analysis.len];
								ret_val->idxs = (int64_t*)(((int8_t*)ret_val->keys) + ptr_size_up);
								uint8_t *strs = (int8_t*)&ret_val->idxs[dlmt_analysis.len];
								for (int64_t i = 0; i < dlmt_analysis.len; i++) {
									const int64_t idx = dlmt_analysis.matched_idxs[i];
									const size_t key_len = dlmt_analysis.keys_lens[i];
									int64_t val_len = dlmt_analysis.vals_lens[i];
									int64_t val_len_nxtflag = 0;
									memcpy(strs, dlmt_analysis.keys[i], key_len);
									strs[key_len] = '\0';
									ret_val->keys[i] = strs;
									strs = &strs[key_len+1];
									cml->matched[idx] = true;
									if (val_len < 0) {
										val_len = -(val_len+1);
										val_len_nxtflag = 1;
										cml->matched[idx+1] = true;
									}
									if (val_len > 0) {
										strs[0] = (uint8_t)val_len_nxtflag;
										strs = &strs[val_len_nxtflag];
										memcpy(strs, dlmt_analysis.vals[i], val_len);
									}
									strs[val_len] = '\0';
									ret_val->vals[i] = strs;
									strs = &strs[val_len+1];
									ret_val->idxs[i] = idx;
								}
								ret_val->len = dlmt_analysis.len;
								cml->matched[cml->len] = dlmt_analysis.all_matched;
							} else {
								vb_err_new_oom(&err, VB_ERR_CML_OOM, 12, NULL, VB_ERR_FFL);
							}
						} else {
							vb_err_new(&err, VB_ERR_CML_OVERFLOW, 31, "matched arguments size overflow", NULL, VB_ERR_FFL);
						}
					}
					if (err)
						vb_err_apdf(cml->err, err);
					if (dlmt_analysis.cap != DLMT_ANALYSIS_BUF_SIZE) {
						if (cml->mem)
							VB_MEM_FREE(cml->mem, dlmt_analysis.matched_idxs);
						else
							free(dlmt_analysis.matched_idxs);
					}
				}
			}
		} else if (args_count == 0) {
			assert(args == NULL);
		} else {
			vb_err_new(cml->err, VB_ERR_CML_UNDERFLOW, 4, "arguments count underflow", NULL, VB_ERR_FFL);
		}
	}
	return ret_val;
}

vb_cml_t *vb_cml_new(const uint8_t **const args, const int64_t args_count, vb_mem_t *const mem, vb_err_t **const err) {
	vb_cml_t *cml = NULL;
	if (VB_ERR_NULL(err)) {
		if (args_count > 0) {
			assert(args);
			if (VB_SZE_MAX_OK_MA(args_count, sizeof(int64_t), HEAD_T_SIZE)) {
				const size_t head_lens_size = (size_t)args_count * sizeof(int64_t) + HEAD_T_SIZE;
				cml = mem ? VB_MEM_ALLOC(mem, (int64_t)head_lens_size) : malloc(head_lens_size);
				if (cml) {
					int64_t *const lens = (int64_t*)&((uint8_t*)cml)[HEAD_T_SIZE];
					if (!cml_init(cml, args, args_count, mem, err, lens, 0)) {
						if (mem)
							VB_MEM_FREE(mem, cml);
						else
							free(cml);
						cml = NULL;
					}
				} else {
					vb_err_new_oom(err, VB_ERR_CML_OOM, 1, NULL, VB_ERR_FFL);
				}
			} else {
				vb_err_new(err, VB_ERR_CML_OVERFLOW, 1, "arguments count overflow", NULL, VB_ERR_FFL);
			}
		} else if (args_count == 0) {
			assert(args == NULL);
			cml = mem ? VB_MEM_ALLOC(mem, (int64_t)HEAD_T_SIZE) : malloc(HEAD_T_SIZE);
			if (cml)
				*cml = (vb_cml_t){0};
			else
				vb_err_new_oom(err, VB_ERR_CML_OOM, 2, NULL, VB_ERR_FFL);
		} else {
			vb_err_new(err, VB_ERR_CML_UNDERFLOW, 1, "arguments count underflow", NULL, VB_ERR_FFL);
		}
	}
	return cml;
}

void vb_cml_undo_matched(vb_cml_t *const cml, vb_cml_args_t **const args, const int64_t args_count) {
	assert(cml);
	if (VB_ERR_NULL(cml->err)) {
		if (cml->matched && cml->len) {
			assert(args);
			bool all_matched = cml->matched[cml->len];
			for (int64_t i = 0; i < args_count; i++) {
				vb_cml_args_t *const aargs = args[i];
				if (aargs) {
					for (int64_t j = 0; j < aargs->len; j++) {
						const int64_t cml_idx = aargs->idxs[j];
						assert(cml->len > cml_idx);
						cml->matched[cml_idx] = false;
						all_matched = false;
						if (aargs->vals[j][0] != '\0' && &aargs->vals[j][-1]) {
							assert(cml->len > cml_idx+1);
							cml->matched[cml_idx+1] = false;
						}
					}
				}
			}
			cml->matched[cml->len] = all_matched;
		}
	}
}

vb_cml_args_t* vb_cml_unmatched(vb_cml_t *const cml) {
	assert(cml);
	vb_cml_args_t *ret_val = NULL;
	if (VB_ERR_NULL(cml->err)) {
		assert(cml->matched);
		if (!cml->matched[cml->len]) {
			int64_t args_size = (int64_t)ARGS_T_SIZE;
			size_t unmatched_count = 0;
			for (int64_t i = 0; i < cml->len; i++) {
				if (!cml->matched[i]) {
					const size_t str_size0 = cml->lens[i]+1;
					if (VB_SZE_MAX_OK_A(args_size, str_size0)) {
						args_size += str_size0;
						unmatched_count++;
					} else {
						vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 26, "arguments size overflow", NULL, VB_ERR_FFL);
						unmatched_count = 0;
						break;
					}
				}
			}
			if (unmatched_count) {
				const size_t ptr_size_up = VB_SZE_ROUND_UP(unmatched_count * sizeof(int64_t*));
				if (VB_SZE_MAX_OK_A(args_size, ptr_size_up)) {
					const size_t idxs_size = unmatched_count * sizeof(int64_t);
					args_size += (int64_t)ptr_size_up;
					if (VB_SZE_MAX_OK_A(args_size, idxs_size)) {
						args_size += (int64_t)idxs_size;
						ret_val = cml->mem ? VB_MEM_ALLOC(cml->mem, (int64_t)args_size) : malloc(args_size);
						if (ret_val) {
							uint8_t **const keys = (uint8_t**)((int8_t*)ret_val + ARGS_T_SIZE);
							int64_t *const idxs = (int64_t*)((int8_t*)keys + ptr_size_up);
							uint8_t *const strs = (uint8_t*)((int8_t*)idxs + idxs_size);
							for (int64_t i = 0, j = 0, strs_off = 0; i < cml->len; i++) {
								if (!cml->matched[i]) {
									uint8_t *const str = &strs[strs_off];
									const size_t str_size0 = (size_t)(cml->lens[i]+1);
									memcpy(str, cml->args[i], str_size0);
									keys[j] = str;
									idxs[j] = i;
									strs_off += (int64_t)str_size0;
									j++;
								}
							}
							ret_val->keys = keys;
							ret_val->vals = NULL;
							ret_val->idxs = idxs;
							ret_val->len = unmatched_count;
						} else {
							vb_err_new_oom(cml->err, VB_ERR_CML_OOM, 10, NULL, VB_ERR_FFL);
						}
					} else {
						vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 28, "arguments size overflow", NULL, VB_ERR_FFL);
					}
				} else {
					vb_err_new(cml->err, VB_ERR_CML_OVERFLOW, 27, "arguments size overflow", NULL, VB_ERR_FFL);
				}
			}
		}
	}
	return ret_val;
}
