#ifndef PARSER_H
#define PARSER_H

#include "nob.h"

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

union token_value {
	char *s_val;
};

struct token {
	enum token_type type;
	union token_value value;
};
void token_destroy(struct token tok);
void print_token(struct token tok);
void sb_append_token(Nob_String_Builder *sb, struct token tok);

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

struct funcall {
	struct expr *items;
	size_t count;
	size_t capacity;
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
void expr_destroy(struct expr expr);
void print_expr(struct expr expr);
void sb_append_expr(Nob_String_Builder *sb, struct expr expr);

struct insluiting {
	char *module;
};
void insluiting_destroy(struct insluiting insluiting);
void print_insluiting(struct insluiting insluiting);
void sb_append_insluiting(Nob_String_Builder *sb, struct insluiting insluiting);

struct funksie {
	char *naam;
	struct expr lyf;
};
void funksie_destroy(struct funksie funksie);
void print_funksie(struct funksie funksie);
void sb_append_funksie(Nob_String_Builder *sb, struct funksie funksie);

union ast_value {
	struct expr expr;
	struct insluiting insluiting;
	struct funksie funksie_definisie;
};

struct ast {
	enum ast_type type;
	union ast_value value;
};
void ast_destroy(struct ast *ast);
void print_ast(struct ast *ast);
void sb_append_ast(Nob_String_Builder *sb, struct ast *ast);

struct parse_state {
	const char *source;
	size_t source_len;
	size_t idx;

	bool verbose;

	struct token prev;
	bool has_prev;
	struct token tok;
	bool has_tok;
};
struct ast *parse_ast(struct parse_state *state);

#endif /* PARSER_H */
