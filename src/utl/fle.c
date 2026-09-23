/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <assert.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vb/fle.h>

bool vb_fle_alloc_path_buffer(vb_fle_t *const file, vb_mem_t *const mem) {
	assert(file);
	file->buf = vb_buf_new_len(PATH_MAX, mem, file->err);
	return (file->buf != NULL);
}

bool vb_fle_dir_name(vb_fle_t *const file) {
	assert(file);
	bool ret_val = false;
	if (VB_ERR_IS_NULL(file)) {
		assert(file->buf);
		assert(file->buf->len > 0);
		// TODO
	}
	return ret_val;
}

bool vb_fle_base_name(vb_fle_t *const file) {
	assert(file);
	bool ret_val = false;
	if (VB_ERR_IS_NULL(file)) {
		assert(file->buf);
		assert(file->buf->len > 0);
		// TODO
	}
	return ret_val;
}

bool vb_fle_exe_path(vb_fle_t *const file) {
	assert(file);
	bool ret_val = false;
	if (VB_ERR_IS_NULL(file)) {
		if (file->buf) {
			if (file->buf->len > 1) {
				const size_t buf_len = (size_t)file->buf->len;
				if ((int64_t)buf_len == file->buf->len) {
					const ssize_t path_len = readlink("/proc/self/exe", file->buf->data, buf_len - 1);
					if (path_len >= 0) {
						file->buf->data[path_len] = '\0';
						file->buf->len = (int64_t)path_len;
						ret_val = true;
					} else {
						vb_err_new(file->err, VB_ERR_FLE_CANT_DETERMINE_PATH, 0, "can't determine path of executable", NULL);
					}
				} else {
					vb_err_new(file->err, VB_ERR_FLE_OVERFLOW, 1, "buffer size overflow", NULL);
				}
			} else {
				vb_err_new(file->err, VB_ERR_FLE_UNDERFLOW, 1, "buffer size underflow", NULL);
			}
		} else {
			vb_err_new(file->err, VB_ERR_FLE_BUFFER_MISSING, 1, "buffer missing", NULL);
		}
	}
	return ret_val;
}

bool vb_fle_mkd(vb_fle_t *const file, const char *const path) {
	assert(file);
	bool ret_val = false;
	if (VB_ERR_IS_NULL(file)) {
		file->type = VB_FLE_OTHER;
		file->exists = false;
		if (!mkdir(path, 0777)) {
			file->type = VB_FLE_DIR;
			file->exists = true;
			ret_val = true;
		} else {
			vb_err_new(file->err, VB_ERR_FLE_CANT_CREATE, 0, "can't create directory", NULL);
		}
	}
	return ret_val;
}

bool vb_fle_mkf(vb_fle_t *const file, const char *const path) {
	assert(file);
	bool ret_val = false;
	if (VB_ERR_IS_NULL(file)) {
		const int fd = open(path, O_CREAT | O_WRONLY | O_EXCL, 0666);
		file->type = VB_FLE_OTHER;
		file->exists = false;
		if (fd >= 0) {
			if (!close(fd)) {
				file->type = VB_FLE_FILE;
				file->exists = true;
				ret_val = true;
			} else {
				vb_err_new(file->err, VB_ERR_FLE_CANT_CLOSE, 0, "can't close file", NULL);
			}
		} else {
			vb_err_new(file->err, VB_ERR_FLE_CANT_CREATE, 0, "can't create file", NULL);
		}
	}
	return ret_val;
}

bool vb_fle_rm(vb_fle_t *const file, const char *const path) {
	assert(file);
	bool ret_val = false;
	if (VB_ERR_IS_NULL(file)) {
		file->type = VB_FLE_FILE;
		file->exists = false;
		if (!unlink(path)) {
			ret_val = true;
		} else if (errno == EISDIR) {
			file->type = VB_FLE_DIR;
			if (!rmdir(path)) {
				// nothing
			} else if (errno == ENOTEMPTY) {
				vb_err_new(file->err, VB_ERR_FLE_CANT_DELETE, 3, "can't delete directory", NULL);
			} else {
				vb_err_new(file->err, VB_ERR_FLE_CANT_DELETE, 2, "can't delete directory", NULL);
			}
		} else {
			vb_err_new(file->err, VB_ERR_FLE_CANT_DELETE, 1, "can't delete file", NULL);
		}
	}
	return ret_val;
}

bool vb_fle_set_base_name(vb_fle_t *const file, const char *const base_name) {
	assert(file);
	bool ret_val = false;
	if (VB_ERR_IS_NULL(file)) {
		assert(base_name);
		if (file->buf) {
			if (file->buf->len > 1) {
				size_t base_name_len = strlen(base_name);
				int64_t base_len = (int64_t)base_name_len;
				if ((size_t)base_len == base_name_len) {
					vb_buf_t *const buf = file->buf;
					int64_t buf_len_m1 = buf->len - 1;
					for (int64_t i = buf_len_m1; i >= 0; i--) {
						if (buf->data[i] == '/') {
							int64_t diff = base_len - (buf_len_m1 - i);
							if (diff <= buf->cap - buf_len_m1) {
								buf->len += diff;
								if (base_name_len > 0)
									memcpy((void*)&buf->data[i+1], (void*)base_name, base_name_len+1); // copy zero, too
								else
									buf->data[buf->len+1] = '\0';
							}
							ret_val = true;
							break;
						}
					}
				} else {
					vb_err_new(file->err, VB_ERR_FLE_OVERFLOW, 2, "base name overflow", NULL);
				}
			} else {
				vb_err_new(file->err, VB_ERR_FLE_UNDERFLOW, 2, "buffer size underflow", NULL);
			}
		} else {
			vb_err_new(file->err, VB_ERR_FLE_BUFFER_MISSING, 2, "buffer missing", NULL);
		}
	}
	return ret_val;
}

bool vb_fle_stats(vb_fle_t *const file, const char *const path) {
	assert(file);
	bool ret_val = false;
	if (VB_ERR_IS_NULL(file)) {
		struct stat info;
		if (!stat(path, &info)) {
			if (S_ISREG(info.st_mode)) {
				file->type = VB_FLE_FILE;
			} else if (S_ISDIR(info.st_mode)) {
				file->type = VB_FLE_DIR;
			} else {
				file->type = VB_FLE_OTHER;
			}
			file->exists = true;
			ret_val = true;
		} else {
			file->type = VB_FLE_OTHER;
			file->exists = false;
			vb_err_new(file->err, VB_ERR_FLE_CANT_READ, 0, "can't read file", NULL);
		}
	}
	return ret_val;
}
