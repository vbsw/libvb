
/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <vb/cml.h>

#define ASSERT(a) if (!(a)) { *err_line = __LINE__; return; }

static void test_init(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		const char *argv[] = {"aa", "bbb", "cccc"};
		const int64_t argc = (int64_t)(sizeof(argv)/sizeof(char**));
		vb_cml_t cml = {0};

		vb_cml_init(&cml, (const uint8_t**)argv, argc, NULL, &err);
		ASSERT(err == NULL)
		ASSERT(cml.err == &err)
		ASSERT(cml.args)
		ASSERT(cml.lens)
		ASSERT(cml.matched)
		ASSERT(cml.dlmt == NULL)
		ASSERT(cml.len = 3)
		ASSERT(cml.lens[0] == 2)
		ASSERT(cml.lens[1] == 3)
		ASSERT(cml.lens[2] == 4)
		for (int64_t i = 0; i < argc+1; i++)
			ASSERT(cml.matched[3] == false)
		ASSERT(strlen(cml.args[0]) == 2)
		ASSERT(strlen(cml.args[1]) == 3)
		ASSERT(strlen(cml.args[2]) == 4)
		ASSERT(strcmp(cml.args[0], "aa") == 0)
		ASSERT(strcmp(cml.args[1], "bbb") == 0)
		ASSERT(strcmp(cml.args[2], "cccc") == 0)

		vb_cml_deinit(&cml);
	}
}

static void test_new(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		const char *argv[] = {"aa", "bbb", "cccc"};
		const int64_t argc = (int64_t)(sizeof(argv)/sizeof(char**));

		vb_cml_t *const cml = vb_cml_new((const uint8_t**)argv, argc, NULL, &err);
		ASSERT(err == NULL)
		ASSERT(cml)
		ASSERT(cml->err == &err)
		ASSERT(cml->args)
		ASSERT(cml->lens)
		ASSERT(cml->matched)
		ASSERT(cml->dlmt == NULL)
		ASSERT(cml->len = 3)
		ASSERT(cml->lens[0] == 2)
		ASSERT(cml->lens[1] == 3)
		ASSERT(cml->lens[2] == 4)
		for (int64_t i = 0; i < argc+1; i++)
			ASSERT(cml->matched[3] == false)
		ASSERT(strlen(cml->args[0]) == 2)
		ASSERT(strlen(cml->args[1]) == 3)
		ASSERT(strlen(cml->args[2]) == 4)
		ASSERT(strcmp(cml->args[0], "aa") == 0)
		ASSERT(strcmp(cml->args[1], "bbb") == 0)
		ASSERT(strcmp(cml->args[2], "cccc") == 0)

		vb_cml_destroy(cml);
	}
}

static void test_dlmt_new(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		vb_cml_t cml = {0};

		vb_cml_init(&cml, NULL, 0, NULL, &err);
		ASSERT(err == NULL)
		ASSERT(cml.err == &err)
		ASSERT(cml.matched == NULL)
		ASSERT(vb_cml_dlmt_new_4(&cml, "a", "bb", "", " "));
		ASSERT(err == NULL)
		ASSERT(cml.err == &err)
		ASSERT(cml.dlmt != NULL)
		ASSERT(cml.dlmt->has_empty_sep)
		ASSERT(cml.dlmt->has_space_sep)
		ASSERT(cml.dlmt->len == 2)
		ASSERT(strlen(cml.dlmt->seps[0]) == 1)
		ASSERT(strlen(cml.dlmt->seps[1]) == 2)
		ASSERT(strcmp(cml.dlmt->seps[0], "a") == 0)
		ASSERT(strcmp(cml.dlmt->seps[1], "bb") == 0)

		free(cml.dlmt);
		ASSERT(vb_cml_dlmt_new_6(&cml, "", "aa", " ", "bbb", "", " "));
		ASSERT(err == NULL)
		ASSERT(cml.err == &err)
		ASSERT(cml.dlmt != NULL)
		ASSERT(cml.dlmt->has_empty_sep)
		ASSERT(cml.dlmt->has_space_sep)
		ASSERT(cml.dlmt->len == 2)
		ASSERT(strlen(cml.dlmt->seps[0]) == 2)
		ASSERT(strlen(cml.dlmt->seps[1]) == 3)
		ASSERT(strcmp(cml.dlmt->seps[0], "aa") == 0)
		ASSERT(strcmp(cml.dlmt->seps[1], "bbb") == 0)

		vb_cml_deinit(&cml);
		ASSERT(cml.err == &err)
		ASSERT(cml.args == NULL)
		ASSERT(cml.lens == NULL)
		ASSERT(cml.matched == NULL)
		ASSERT(cml.dlmt == NULL)
		ASSERT(cml.len == 0)
	}
}

static void test_match_has_key_at(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		const char *argv[] = {"--help", "--version", "print"};
		const int64_t argc = (int64_t)(sizeof(argv)/sizeof(char**));

		vb_cml_t *const cml = vb_cml_new((const uint8_t**)argv, argc, NULL, &err);
		ASSERT(err == NULL)
		ASSERT(cml)
		vb_cml_args_t *const args1 = vb_cml_match_3(cml, "-h", "--help", "help");
		ASSERT(args1)
		ASSERT(args1->len == 1)
		ASSERT(args1->idxs[0] == 0)
		ASSERT(strcmp(args1->keys[0], argv[0]) == 0)
		ASSERT(cml->matched[0])
		ASSERT(vb_cml_args_has_key(args1, argv[0]));
		ASSERT(vb_cml_args_has_key(args1, argv[1]) == false);
		ASSERT(vb_cml_args_has_key(args1, argv[2]) == false);
		ASSERT(vb_cml_args_index_at(args1, 0, -1) == 0);
		ASSERT(vb_cml_args_index_at(args1, 1, -1) == -1);
		ASSERT(vb_cml_args_index_at(args1, 9, -1) == -1);
		ASSERT(strcmp(vb_cml_args_key_at(args1, 0, "stub"), argv[0]) == 0);
		ASSERT(strcmp(vb_cml_args_key_at(args1, 1, "stub"), "stub") == 0);
		ASSERT(strcmp(vb_cml_args_key_at(args1, 9, "stub"), "stub") == 0);

		vb_cml_args_t *const args2 = vb_cml_match_3(cml, "-v", "version", "--version");
		ASSERT(args2)
		ASSERT(args2->len == 1)
		ASSERT(args2->idxs[0] == 1)
		ASSERT(strcmp(args2->keys[0], argv[1]) == 0)
		ASSERT(cml->matched[0])
		ASSERT(cml->matched[1])

		vb_cml_undo_matched_2(cml, args1, args2);
		ASSERT(cml->matched[0] == false)
		ASSERT(cml->matched[1] == false)

		vb_cml_args_t *const args3 = vb_cml_match_3(cml, "print", "--help", "asdf");
		ASSERT(args3)
		ASSERT(args3->len == 2)
		ASSERT(args3->idxs[0] == 0)
		ASSERT(args3->idxs[1] == 2)
		ASSERT(strcmp(args3->keys[0], argv[0]) == 0)
		ASSERT(strcmp(args3->keys[1], argv[2]) == 0)
		ASSERT(vb_cml_args_has_key(args3, argv[0]));
		ASSERT(vb_cml_args_has_key(args3, argv[1]) == false);
		ASSERT(vb_cml_args_has_key(args3, argv[2]));
		ASSERT(vb_cml_args_index_at(args3, 0, -1) == 0);
		ASSERT(vb_cml_args_index_at(args3, 1, -1) == 2);
		ASSERT(vb_cml_args_index_at(args3, 9, -1) == -1);
		ASSERT(strcmp(vb_cml_args_key_at(args3, 0, "stub"), argv[0]) == 0);
		ASSERT(strcmp(vb_cml_args_key_at(args3, 1, "stub"), argv[2]) == 0);
		ASSERT(strcmp(vb_cml_args_key_at(args3, 9, "stub"), "stub") == 0);

		free(args1);
		free(args2);
		free(args3);
		vb_cml_destroy(cml);
	}
}

static void test_match_dlmt0(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		const char *argv[] = {"--help=10", "--version=20", "print="};
		const int64_t argc = (int64_t)(sizeof(argv)/sizeof(char**));
		vb_cml_t cml = {0};

		vb_cml_init(&cml, (const uint8_t**)argv, argc, NULL, &err);
		ASSERT(cml.dlmt == NULL)
		ASSERT(vb_cml_dlmt_new_2(&cml, "=", ""));
		ASSERT(cml.dlmt != NULL)

		vb_cml_args_t *const args1 = vb_cml_match_dlmt_3(&cml, "-h", "--help", "help");
		ASSERT(args1)
		ASSERT(args1->len == 1)
		ASSERT(args1->idxs)
		ASSERT(args1->keys)
		ASSERT(args1->vals)
		ASSERT(args1->idxs[0] == 0)
		ASSERT(strcmp(args1->keys[0], "--help") == 0)
		ASSERT(strcmp(args1->vals[0], "10") == 0)
		ASSERT(cml.matched[0] && !cml.matched[1] && !cml.matched[2] && !cml.matched[3])

		cml.matched[0] = false;
		vb_cml_args_t *const args2 = vb_cml_match_dlmt_3(&cml, "-v", "version", "--version");
		ASSERT(args2)
		ASSERT(args2->len == 1)
		ASSERT(args2->idxs)
		ASSERT(args2->keys)
		ASSERT(args2->vals)
		ASSERT(args2->idxs[0] == 1)
		ASSERT(strcmp(args2->keys[0], "--version") == 0)
		ASSERT(strcmp(args2->vals[0], "20") == 0)
		ASSERT(!cml.matched[0] && cml.matched[1] && !cml.matched[2] && !cml.matched[3])

		cml.matched[1] = false;
		vb_cml_args_t *const args3 = vb_cml_match_dlmt_3(&cml, "--help", "version", "--version");
		ASSERT(args3)
		ASSERT(args3->len == 2)
		ASSERT(args3->idxs)
		ASSERT(args3->keys)
		ASSERT(args3->vals)
		ASSERT(args3->idxs[0] == 0)
		ASSERT(args3->idxs[1] == 1)
		ASSERT(strcmp(args3->keys[0], "--help") == 0)
		ASSERT(strcmp(args3->vals[0], "10") == 0)
		ASSERT(strcmp(args3->keys[1], "--version") == 0)
		ASSERT(strcmp(args3->vals[1], "20") == 0)
		ASSERT(cml.matched[0] && cml.matched[1] && !cml.matched[2] && !cml.matched[3])

		cml.matched[0] = false;
		cml.matched[1] = false;
		vb_cml_args_t *const args4 = vb_cml_match_dlmt_3(&cml, "-v", "print", "--version");
		ASSERT(args4)
		ASSERT(args4->len == 2)
		ASSERT(args4->idxs)
		ASSERT(args4->keys)
		ASSERT(args4->vals)
		ASSERT(args4->idxs[0] == 1)
		ASSERT(args4->idxs[1] == 2)
		ASSERT(strcmp(args4->keys[0], "--version") == 0)
		ASSERT(strcmp(args4->vals[0], "20") == 0)
		ASSERT(strcmp(args4->keys[1], "print") == 0)
		ASSERT(strcmp(args4->vals[1], "") == 0)
		ASSERT(!cml.matched[0] && cml.matched[1] && cml.matched[2] && !cml.matched[3])

		free(args1);
		free(args2);
		free(args3);
		free(args4);
		vb_cml_deinit(&cml);
	}
}

static void test_match_dlmt1(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		const char *argv[] = {"--delta", "10", "--version=20", "yo"};
		const int64_t argc = (int64_t)(sizeof(argv)/sizeof(char**));
		vb_cml_t cml = {0};

		vb_cml_init(&cml, (const uint8_t**)argv, argc, NULL, &err);
		ASSERT(cml.dlmt == NULL)
		ASSERT(vb_cml_dlmt_new_3(&cml, "=", "", " "));
		ASSERT(cml.dlmt != NULL)
		ASSERT(cml.dlmt->has_empty_sep)
		ASSERT(cml.dlmt->has_space_sep)

		vb_cml_args_t *const args1 = vb_cml_match_dlmt_3(&cml, "--delta", "--help", "yo");
		ASSERT(args1)
		ASSERT(args1->len == 2)
		ASSERT(args1->idxs)
		ASSERT(args1->keys)
		ASSERT(args1->vals)
		ASSERT(args1->idxs[0] == 0)
		ASSERT(args1->idxs[1] == 3)
		ASSERT(strcmp(args1->keys[0], "--delta") == 0)
		ASSERT(strcmp(args1->vals[0], "10") == 0)
		ASSERT(strcmp(args1->keys[1], "yo") == 0)
		ASSERT(strcmp(args1->vals[1], "") == 0)

		free(args1);
		vb_cml_deinit(&cml);
	}
}

static void test_undo_matched(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		const char *argv[] = {"--delta", "10", "--version=20", "yo"};
		const int64_t argc = (int64_t)(sizeof(argv)/sizeof(char**));
		vb_cml_t cml = {0};

		vb_cml_init(&cml, (const uint8_t**)argv, argc, NULL, &err);
		ASSERT(cml.dlmt == NULL)
		ASSERT(vb_cml_dlmt_new_3(&cml, "=", "", " "));
		ASSERT(cml.dlmt != NULL)
		ASSERT(cml.dlmt->has_empty_sep)
		ASSERT(cml.dlmt->has_space_sep)

		vb_cml_args_t *const args1 = vb_cml_match_dlmt_3(&cml, "--delta", "--help", "yo");
		ASSERT(args1)
		ASSERT(cml.matched[0])
		ASSERT(cml.matched[1])
		ASSERT(!cml.matched[2])
		ASSERT(cml.matched[3])
		ASSERT(!cml.matched[4])

		vb_cml_undo_matched_1(&cml, args1);
		ASSERT(!cml.matched[0])
		ASSERT(!cml.matched[1])
		ASSERT(!cml.matched[2])
		ASSERT(!cml.matched[3])
		ASSERT(!cml.matched[4])

		free(args1);
		vb_cml_deinit(&cml);
	}
}

static void test_unmatched(int *const err_line) {
	if (*err_line == 0) {
		vb_err_t *err = NULL;
		const char *argv[] = {"--help", "--version", "print"};
		const int64_t argc = (int64_t)(sizeof(argv)/sizeof(char**));

		vb_cml_t *const cml = vb_cml_new((const uint8_t**)argv, argc, NULL, &err);
		ASSERT(err == NULL)
		ASSERT(cml)
		vb_cml_args_t *const args1 = vb_cml_match_3(cml, "-h", "--help", "help");
		ASSERT(args1)
		ASSERT(args1->len == 1)

		vb_cml_args_t *const args_unmatched = vb_cml_unmatched(cml);
		ASSERT(args_unmatched)
		ASSERT(args_unmatched->keys)
		ASSERT(args_unmatched->vals == NULL)
		ASSERT(args_unmatched->idxs)
		ASSERT(args_unmatched->len == 2)
		ASSERT(strcmp(args_unmatched->keys[0], argv[1]) == 0)
		ASSERT(strcmp(args_unmatched->keys[1], argv[2]) == 0)
		ASSERT(args_unmatched->idxs[0] == 1)
		ASSERT(args_unmatched->idxs[1] == 2)

		free(args1);
		free(args_unmatched);
		vb_cml_destroy(cml);
	}
}

void test_cml(int *const err_line) {
	*err_line = 0;
	test_init(err_line);
	test_new(err_line);
	test_dlmt_new(err_line);
	test_match_has_key_at(err_line);
	test_match_dlmt0(err_line);
	test_match_dlmt1(err_line);
	test_undo_matched(err_line);
	test_unmatched(err_line);
}
