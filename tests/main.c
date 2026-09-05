/*
 *          Copyright 2026, Vitali Baumtrok.
 * Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE or copy at
 *        http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stdint.h>
#include <stdio.h>

#define PRINT_RESULT(a) if (!err_line) printf("%-6s PASS\n", a); else printf("%-6s FAIL (line %d)\n", a, err_line);

void test_err(int *err_line);

int main(int argc, char **argv) {
	int err_line;
	test_err(&err_line);
	PRINT_RESULT("err")
	return 0;
}
