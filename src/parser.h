#ifndef PARSER_H
#define PARSER_H

#include "nob.h"

#include "utils.h"

#define S_VAL_TOKENS X(IDENTIFIER) X(STRING_LIT)
#define CHAR_TOKENS \
	X(SYM_AT, '@') \
	X(SYM_SEMICOLON, ';') \
	\
	X(SYM_LPAREN, '(') \
	X(SYM_RPAREN, ')') \
	X(SYM_LBRACE, '{') \
	X(SYM_RBRACE, '}')
#define KW_TOKENS \
	X(KW_FUNK, "funk")
enum token_type {
	TOK_EOF = -1,

#define X(tt, ...) tt,
	S_VAL_TOKENS

	CHAR_TOKENS

	KW_TOKENS
#undef X
};

struct source_tracking {
	const char *source;
	size_t size;
	size_t idx;
};
struct source_tracking st_init(const char *source);
bool st_atend(const struct source_tracking *this);
char st_at(const struct source_tracking *this);
bool st_isat(const struct source_tracking *this, char c);
bool st_advat(struct source_tracking *this, char c);
bool st_checkcur(const struct source_tracking *this, bool (*check)(char));
void st_skipwhile(struct source_tracking *this, bool (*cond)(char));
bool st_canpeek(const struct source_tracking *this);
char st_peek(const struct source_tracking *this);
char st_adv(struct source_tracking *this);

union token_value {
	char *s_val;
};

struct token {
	enum token_type type;
	union token_value value;
};
DECL_STD_METHS(token);

enum ast_type {
	AST_EOF = -1,

	EXPR,
	INSLUITING,
	FUNKSIE_DEFINISIE,
};

enum expr_type {
	EXPR_FUNCALL,
	EXPR_COMPOUND,
	EXPR_STR_LIT,
};

struct funk_args {
	struct expr *items;
	size_t count;
	size_t capacity;
};
struct funcall {
	struct funk_args args;
	char *naam;
};
struct compound {
	struct expr *items;
	size_t count;
	size_t capacity;
	bool last_empty;
};

union expr_value {
	struct funcall funcall;
	struct compound compound;
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
