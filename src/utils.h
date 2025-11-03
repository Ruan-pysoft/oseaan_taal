#ifndef UTILS_H
#define UTILS_H

#include "nob.h"

#define DESTROY_METH(typename) void typename ## _destroy(struct typename *this)
#define COPY_METH(typename) struct typename typename ## _copy(const struct typename *this)
#define SB_APPEND_FUNC(typename) void sb_append_ ## typename(Nob_String_Builder *sb, const struct typename *this)
#define FPRINT_FUNC(typename) void fprint_ ## typename(const struct typename *this, FILE *file)
#define PRINT_FUNC(typename) void print_ ## typename(const struct typename *this)

#define DECL_STD_METHS(typename) \
DESTROY_METH(typename); \
COPY_METH(typename); \
SB_APPEND_FUNC(typename); \
FPRINT_FUNC(typename); \
PRINT_FUNC(typename)

#endif /* UTILS_H */
