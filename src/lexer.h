#ifndef LEXER_H
#define LEXER_H

#include "utils.h"

#define S_VAL_TOKENS X(IDENTIFIER) X(STRING_LIT) X(NUMBER_LIT)
#define SYMBOL_TOKENS \
	X(SYM_AT, "@") \
	X(SYM_COMMA, ",") \
	X(SYM_SEMICOLON, ";") \
	X(SYM_COLON, ":") \
	\
	X(SYM_LPAREN, "(") \
	X(SYM_RPAREN, ")") \
	X(SYM_LBRACE, "{") \
	X(SYM_RBRACE, "}") \
	X(SYM_ARROW, "->") \
	X(SYM_EQUAL, "=") \
	X(SYM_PLUS, "+") \
	X(SYM_STAR, "*")
#define KW_TOKENS \
	X(KW_EKSTERN, "eksterne") \
	X(KW_FUNK, "funk") \
	X(KW_LAAT, "laat") \
	X(KW_KON, "kon") \
	X(KW_VER, "ver") \
	X(KW_ERF, "erf") \
	X(KW_HEEL, "heel")
enum token_type {
	TOK_EOF = 0,
	TOK_UNKNOWN,

#define X(tt, ...) tt,
	S_VAL_TOKENS

	SYMBOL_TOKENS

	KW_TOKENS
#undef X
};

struct source_tracking {
	const char *filename;
	const char *source;
	size_t size;
	size_t idx;
	size_t line_start;
	size_t line;
};
struct source_tracking st_init(const char *filename, const char *source);
bool st_atend(const struct source_tracking *this);
char st_at(const struct source_tracking *this);
bool st_isat(const struct source_tracking *this, char c);
bool st_match(const struct source_tracking *this, const char *str);
bool st_advat(struct source_tracking *this, char c);
bool st_checkcur(const struct source_tracking *this, bool (*check)(char));
void st_skipwhile(struct source_tracking *this, bool (*cond)(char));
bool st_canpeek(const struct source_tracking *this);
char st_peek(const struct source_tracking *this);
char st_adv(struct source_tracking *this);
void st_advby(struct source_tracking *this, size_t by);

struct token {
	enum token_type type;
	struct source_tracking pos;
	size_t len;
};
DECL_STD_METHS(token);

struct token lex_token(struct source_tracking *src);

#endif /* LEXER_H */
