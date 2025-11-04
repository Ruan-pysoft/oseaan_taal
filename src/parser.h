#ifndef PARSER_H
#define PARSER_H

#include "nob.h"

#include "types.h"
#include "utils.h"

enum ast_type {
	AST_EOF = -1,

	EXPR,
	INSLUITING,
	FUNKSIE_DEFINISIE,
	VERANDERLIKE_DEKLARASIE,
	VERANDERLIKE_DEFINISIE,
};

enum expr_type {
	EXPR_FUNCALL,
	EXPR_COMPOUND,
	EXPR_STR_LIT,
};

struct e_funk_args {
	struct expr *items;
	size_t count;
	size_t capacity;
};
struct e_funcall {
	struct e_funk_args args;
	char *naam;
};
struct e_compound {
	struct e_expr *items;
	size_t count;
	size_t capacity;
	bool last_empty;
};

union expr_value {
	struct e_funcall funcall;
	struct e_compound compound;
	char *str_lit;
};

struct expr {
	enum expr_type type;
	union expr_value value;
};
DECL_STD_METHS(expr);

struct insluiting {
	char *module;
};
DECL_STD_METHS(insluiting);

struct funksie {
	char *naam;
	struct expr lyf;
};
DECL_STD_METHS(funksie);

struct vreanderlike_deklarasie {
	char *naam;
};

union ast_value {
	struct expr expr;
	struct insluiting insluiting;
	struct funksie funksie_definisie;
};

struct ast {
	enum ast_type type;
	union ast_value value;
};
DECL_STD_METHS(ast);

struct parse_state {
	struct source_tracking src;

	bool verbose;

	struct token prev;
	bool has_prev;
	struct token tok;
	bool has_tok;
};
struct parse_state parse_state_init(const char *source);
struct ast parse_ast(struct parse_state *state);

struct program {
	struct ast *items;
	size_t count;
	size_t capacity;
};
DECL_STD_METHS(program);
struct program parse_program(struct parse_state *state);

#endif /* PARSER_H */
