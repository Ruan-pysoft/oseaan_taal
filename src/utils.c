#include "utils.h"

#include <execinfo.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// https://stackoverflow.com/a/77336
#define MAX_BACKTRACE_DEPTH 64
void fprint_backtrace(FILE *file) {
	void *backtraces[MAX_BACKTRACE_DEPTH];
	size_t n_backtraces;

	fputs("=== BACKTRACE ===\n", file);

	n_backtraces = backtrace(backtraces, MAX_BACKTRACE_DEPTH);
	int old_errno = errno;
	int no = fileno(file);
	if (no != -1) {
		backtrace_symbols_fd(backtraces, n_backtraces, no);
	} else {
		errno = old_errno;

		char **symbols = backtrace_symbols(backtraces, n_backtraces);
		for (const char *sym = symbols[0]; sym < symbols[n_backtraces]; ++sym) {
			fputs(sym, file);
			fputc('\n', file);
		}
		free(symbols);
	}
}
extern const char *__progname;
void _bt_assert_fail(const char *assertion, const char *file, int line, const char *func) {
	fprintf(stderr, "%s: %s:%d: %s: Assertion `%s` failed.\n", __progname, file, line, func, assertion);
	fprint_backtrace(stderr);
	abort();
	(true ? fprint_backtrace(stdout) : (void)0);
}
