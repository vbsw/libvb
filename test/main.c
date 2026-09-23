/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stdint.h>
#include <stdio.h>

#define PRINT_RESULT_CLM_A(unt) if (!err_line) printf("%-6s %-21s", unt, "PASS"); else printf("%-6s FAIL line %-11d", unt, err_line);
#define PRINT_RESULT_CLM_B(unt) if (!err_line) printf("%-6s PASS\n", unt); else printf("%-6s FAIL line %d\n", unt, err_line);

void test_err(int *err_line);
void test_mst(int *err_line);
void test_mhp(int *err_line);
void test_mar(int *err_line);
void test_tme(int *err_line);
void test_fle(int *err_line);
void test_buf(int *err_line);

int main(int argc, char **argv) {
	int err_line;
	test_err(&err_line); PRINT_RESULT_CLM_A("err")
	test_mst(&err_line); PRINT_RESULT_CLM_B("mst")
	test_mhp(&err_line); PRINT_RESULT_CLM_A("mhp")
	test_mar(&err_line); PRINT_RESULT_CLM_B("mar")
	test_buf(&err_line); PRINT_RESULT_CLM_A("buf")
	test_tme(&err_line); PRINT_RESULT_CLM_B("tme")
	test_fle(&err_line); PRINT_RESULT_CLM_A("fle")
	return 0;
}
