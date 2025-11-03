#ifndef UTILS_H
#define UTILS_H

#include "nob.h"

#define DESTROY_METH(typename) void typename ## _destroy(struct typename *this)
#define COPY_METH(typename) struct typename typename ## _copy(const struct typename *this)
#define SB_APPEND_FUNC(typename) void sb_append_ ## typename(Nob_String_Builder *sb, const struct typename *this)
#define FPRINT_FUNC(typename) void fprint_ ## typename(const struct typename *this, FILE *file)
#define PRINT_FUNC(typename) void print_ ## typename(const struct typename *this)
#define SB_APPEND_FUNC_ENUM(typename) void sb_append_ ## typename(Nob_String_Builder *sb, const enum typename this)
#define FPRINT_FUNC_ENUM(typename) void fprint_ ## typename(const enum typename this, FILE *file)
#define PRINT_FUNC_ENUM(typename) void print_ ## typename(const enum typename this)

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
