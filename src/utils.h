#ifndef UTILS_H
#define UTILS_H

#include "nob.h"

struct src_pos {
	const char *filename;
	const char *source;
	size_t size;
	size_t idx;
	size_t line_start;
	size_t line;
	size_t len;
};
const char *src_pos_getbuf(const struct src_pos *this);
const char *src_pos_getline(const struct src_pos *this);
size_t src_pos_comp_linelen(const struct src_pos *this);
int src_pos_strcmp(const struct src_pos *this, const char *str);
bool src_pos_eq(const struct src_pos *this, const struct src_pos *other);
#define SRC_POS_FMT "%.*s"
#define SRC_POS_FARGS(src_pos) (int)(src_pos).len, src_pos_getbuf(&(src_pos))
#define SRC_POS_LOC_FMT "%s:%lu,%lu"
#define SRC_POS_LOC_FARGS(src_pos) (src_pos).filename, (src_pos).line, ((src_pos).idx - (src_pos).line_start + 1)

void fprint_backtrace(FILE *file);
void _bt_assert_fail(const char *assertion, const char *file, int line, const char *func);
#ifdef NDEBUG
#define bt_assert(expr) ((void)0)
#else
#define bt_assert(expr) ((expr) ? (void)0 : _bt_assert_fail(#expr, __FILE__, __LINE__, __func__))
#endif

#define UTIL_METH
#define DESTROY_METH(typename) UTIL_METH void typename ## _destroy(struct typename *this)
#define COPY_METH(typename) UTIL_METH struct typename typename ## _copy(const struct typename *this)
#define SB_APPEND_FUNC(typename) UTIL_METH void sb_append_ ## typename(Nob_String_Builder *sb, const struct typename *this)
#define FPRINT_FUNC(typename) UTIL_METH void fprint_ ## typename(const struct typename *this, FILE *file)
#define PRINT_FUNC(typename) UTIL_METH void print_ ## typename(const struct typename *this)
#define SB_APPEND_FUNC_ENUM(typename) UTIL_METH void sb_append_ ## typename(Nob_String_Builder *sb, const enum typename this)
#define FPRINT_FUNC_ENUM(typename) UTIL_METH void fprint_ ## typename(const enum typename this, FILE *file)
#define PRINT_FUNC_ENUM(typename) UTIL_METH void print_ ## typename(const enum typename this)

#define PRINT_IMPL(typename) \
FPRINT_FUNC(typename) { \
	Nob_String_Builder sb = {0}; \
	sb_append_ ## typename(&sb, this); \
	fprintf(file, "%.*s", (int)sb.count, sb.items); \
	nob_sb_free(sb); \
} \
PRINT_FUNC(typename) { fprint_ ## typename(this, stdout); }

#define PRINT_IMPL_ENUM(typename) \
FPRINT_FUNC_ENUM(typename) { \
	Nob_String_Builder sb = {0}; \
	sb_append_ ## typename(&sb, this); \
	fprintf(file, "%.*s", (int)sb.count, sb.items); \
	nob_sb_free(sb); \
} \
PRINT_FUNC_ENUM(typename) { fprint_ ## typename(this, stdout); }

#define DECL_STD_METHS(typename) \
DESTROY_METH(typename); \
COPY_METH(typename); \
SB_APPEND_FUNC(typename); \
FPRINT_FUNC(typename); \
PRINT_FUNC(typename)

#define DECL_DISPLAY_METHS_ENUM(typename) \
SB_APPEND_FUNC_ENUM(typename); \
FPRINT_FUNC_ENUM(typename); \
PRINT_FUNC_ENUM(typename)

#endif /* UTILS_H */
