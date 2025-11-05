#include "utils.h"

#include <execinfo.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const char *src_pos_getbuf(const struct src_pos *this) {
	bt_assert(this != NULL);
	bt_assert(this->source != NULL);

	return &this->source[this->idx];
}
const char *src_pos_getline(const struct src_pos *this) {
	bt_assert(this != NULL);
	bt_assert(this->source != NULL);

	return &this->source[this->line_start];
}
size_t src_pos_comp_linelen(const struct src_pos *this) {
	bt_assert(this != NULL);
	bt_assert(this->source != NULL);

	const char *line = src_pos_getline(this);
	size_t len = 0;
	while (line[len] != '\n' && line[len] != 0) ++len;
	return len;
}
int src_pos_strcmp(const struct src_pos *this, const char *str) {
	bt_assert(this != NULL && str != NULL);
	bt_assert(this->source != NULL);

	const char *buf = src_pos_getbuf(this);
	for (size_t i = 0; i < this->len; ++i) {
		if (buf[i] < str[i]) return -1;
		if (buf[i] > str[i]) return 1;
	}
	if (str[this->len] != 0) return 1;
	return 0;
}
bool src_pos_eq(const struct src_pos *this, const struct src_pos *other) {
	bt_assert(this != NULL && other != NULL);
	bt_assert(this->source != NULL && other->source != NULL);

	if (this == other) return true;
	if (this->len != other->len) return false;
	return memcmp(src_pos_getbuf(this), src_pos_getbuf(other), this->len) == 0;
}

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
