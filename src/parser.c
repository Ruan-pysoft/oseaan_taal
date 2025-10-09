#include "parser.h"

void token_destroy(struct token tok) {
	switch (tok.type) {
		#define X(tt) case tt: { if (tok.value.s_val) free(tok.value.s_val); } break;
		S_VAL_TOKENS
		#undef X
		default: break;
	}
}
void print_token(struct token tok) {
	Nob_String_Builder sb = {0};
	sb_append_token(&sb, tok);
	printf("%s", sb.items);
	nob_sb_free(sb);
}
void sb_append_token(Nob_String_Builder *sb, struct token tok) {
	switch (tok.type) {
		case TOK_EOF: {
			nob_sb_appendf(sb, "TOK_EOF");
		} break;

		#define X(tt) case tt: { nob_sb_appendf(sb, #tt "(%s)", tok.value.s_val); } break;
		S_VAL_TOKENS
		#undef X

		#define X(tt, _) case tt: { nob_sb_append_cstr(sb, #tt); } break;
		CHAR_TOKENS

		KW_TOKENS
		#undef X
	}
}


bool is_space(char c) {
	return c <= ' ' || c >= 127;
}
bool is_alpha(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}
bool is_digit(char c) {
	return '0' <= c && c <= '9';
}
bool is_alnum(char c) {
	return is_alpha(c) || is_digit(c) || c == '_';
}
void skip_space(const char *source, size_t source_len, size_t *idx) {
	while (*idx < source_len && is_space(source[*idx])) ++*idx;
}

struct token lex_token(const char *source, size_t source_len, size_t *idx) {
	struct token res = { EOF, {0} };

	skip_space(source, source_len, idx);

	if (*idx >= source_len) return res;

	#define X(tt, ch) if (source[*idx] == ch) { ++*idx; res.type = tt; } else
	CHAR_TOKENS
	#undef X
	if (is_alpha(source[*idx]) || source[*idx] == '_') {
		const size_t begin = *idx;
		++*idx;

		while (*idx < source_len && is_alnum(source[*idx])) {
			++*idx;
		}

		#define X(tt, st) if (strncmp(&source[begin], st, strlen(st)) == 0) { res.type = tt; } else
		KW_TOKENS
		#undef X
		{
			res.type = IDENTIFIER;
			res.value.s_val = strndup(&source[begin], *idx - begin);
		}
	} else if (source[*idx] == '"') {
		const size_t begin = *idx;
		++*idx;

		while (*idx < source_len && source[*idx] != '"') {
			++*idx;
		}

		if (*idx < source_len && source[*idx] == '"') {
			++*idx;
		} else {
			nob_log(NOB_ERROR, "Unterminated string literal");
			exit(1);
		}

		res.type = STRING_LIT;
		res.value.s_val = strndup(&source[begin+1], *idx - begin - 2);
	} else {
		nob_log(NOB_ERROR, "Unexpected character %c in source", source[*idx]);
		exit(1);
	}

	return res;
}

void expr_destroy(struct expr expr) {
	switch (expr.type) {
		case EXPR_FUNCALL: {
			for (size_t i = 0; i < expr.value.funcall.count; ++i) {
				expr_destroy(expr.value.funcall.items[i]);
			}
			nob_da_free(expr.value.funcall);
			free(expr.value.funcall.naam);
		} break;
		case EXPR_COMPOUND: {
			for (size_t i = 0; i < expr.value.compound.count; ++i) {
				expr_destroy(expr.value.compound.items[i]);
			}
			nob_da_free(expr.value.compound);
		} break;
		case EXPR_STR_LIT: {
			free(expr.value.str_lit);
		}
	}
}
void print_expr(struct expr expr) {
	Nob_String_Builder sb = {0};
	sb_append_expr(&sb, expr);
	printf("%s", sb.items);
	nob_sb_free(sb);
}
void sb_append_expr(Nob_String_Builder *sb, struct expr expr) {
	switch (expr.type) {
		case EXPR_FUNCALL: {
			nob_sb_appendf(sb, "FUNCALL(%s: ", expr.value.funcall.naam);
			for (size_t i = 0; i < expr.value.funcall.count; ++i) {
				if (i) nob_sb_appendf(sb, ", ");
				sb_append_expr(sb, expr.value.funcall.items[i]);
			}
			nob_sb_appendf(sb, ")");
		} break;
		case EXPR_COMPOUND: {
			nob_sb_appendf(sb, "COMPOUND(");
			for (size_t i = 0; i < expr.value.funcall.count; ++i) {
				if (i) nob_sb_appendf(sb, ", ");
				sb_append_expr(sb, expr.value.funcall.items[i]);
			}
			nob_sb_appendf(sb, ")");
		} break;
		case EXPR_STR_LIT: {
			nob_sb_appendf(sb, "\"%s\"", expr.value.str_lit);
		} break;
	}
}

void insluiting_destroy(struct insluiting insluiting) {
	free(insluiting.module);
}
void print_insluiting(struct insluiting insluiting) {
	Nob_String_Builder sb = {0};
	sb_append_insluiting(&sb, insluiting);
	printf("%s", sb.items);
	nob_sb_free(sb);
}
void sb_append_insluiting(Nob_String_Builder *sb, struct insluiting insluiting) {
	nob_sb_appendf(sb, "INSLUITING(%s)", insluiting.module);
}

void funksie_destroy(struct funksie funksie) {
	free(funksie.naam);
	expr_destroy(funksie.lyf);
}
void print_funksie(struct funksie funksie) {
	Nob_String_Builder sb = {0};
	sb_append_funksie(&sb, funksie);
	printf("%s", sb.items);
	nob_sb_free(sb);
}
void sb_append_funksie(Nob_String_Builder *sb, struct funksie funksie) {
	nob_sb_appendf(sb, "FUNKSIE(%s, ", funksie.naam);
	sb_append_expr(sb, funksie.lyf);
	nob_sb_appendf(sb, ")");
}

void ast_destroy(struct ast *ast) {
	switch (ast->type) {
		case EXPR: {
			expr_destroy(ast->value.expr);
		} break;
		case INSLUITING: {
			insluiting_destroy(ast->value.insluiting);
		} break;
		case FUNKSIE_DEFINISIE: {
			funksie_destroy(ast->value.funksie_definisie);
		} break;
		case AST_EOF: break;
	}
}
void print_ast(struct ast *ast) {
	Nob_String_Builder sb = {0};
	sb_append_ast(&sb, ast);
	printf("%s", sb.items);
	nob_sb_free(sb);
}
void sb_append_ast(Nob_String_Builder *sb, struct ast *ast) {
	switch (ast->type) {
		case EXPR: {
			sb_append_expr(sb, ast->value.expr);
		} break;
		case INSLUITING: {
			sb_append_insluiting(sb, ast->value.insluiting);
		} break;
		case FUNKSIE_DEFINISIE: {
			sb_append_funksie(sb, ast->value.funksie_definisie);
		} break;
		case AST_EOF: {
			nob_sb_appendf(sb, "AST_EOF");
		} break;
	}
}

struct token *parse_peek(struct parse_state *state) {
	if (!state->has_tok) {
		state->tok = lex_token(state->source, state->source_len, &state->idx);
		state->has_tok = true;
	}
	return &state->tok;
}
struct token *parse_advance(struct parse_state *state) {
	parse_peek(state);

	if (state->has_prev) {
		token_destroy(state->prev);
	}

	state->prev = state->tok;
	state->has_prev = true;
	state->has_tok = false;

	return &state->prev;
}

#define ERR_UNEXPECTED_TOKEN(expected_t_msg) do { \
		nob_sb_append_cstr(&err, "Unexpected token in source "); \
		sb_append_token(&err, *tok); \
		nob_sb_append_cstr(&err, ", expected token of type " expected_t_msg); \
		nob_log(NOB_ERROR, "%s,%d: %s", __FILE__, __LINE__, err.items); \
		exit(1); \
	 } while (0)
#define VERB(str) if (state->verbose) nob_log(NOB_INFO, str);
#define VERBF(fmt, ...) if (state->verbose) nob_log(NOB_INFO, fmt, __VA_ARGS__);
#define PARSE_ENTER(what) \
	Nob_String_Builder err = {0}; \
	nob_sb_append_cstr(&err, what ": "); \
	VERB("Entering " what "..."); \
	do {} while (0)
#define PARSE_LEAVE(what) do { \
		nob_sb_free(err); \
		VERB("Leaving " what "..."); \
		return res; \
	} while (0)
struct expr parse_expr(struct parse_state *state);
struct expr parse_compound_expr(struct parse_state *state) {
	struct expr res;

	PARSE_ENTER("Compound Expr");

	struct token *tok = parse_advance(state);
	if (tok->type != SYM_LBRACE) {
		ERR_UNEXPECTED_TOKEN("SYM_LBRACE");
	}

	res.type = EXPR_COMPOUND;
	res.value.compound = (struct compound) {
		.items = NULL,
		.capacity = 0,
		.count = 0,
		.last_empty = true,
	};
	tok = parse_peek(state);
	if (tok->type == SYM_RBRACE) {
		parse_advance(state);
		PARSE_LEAVE("Compound Expr");
	}

	for (;;) {
		struct expr expr = parse_expr(state);
		nob_da_append(&res.value.compound, expr);

		res.value.compound.last_empty = false;
		tok = parse_peek(state);
		if (tok->type == SYM_SEMICOLON) {
			res.value.compound.last_empty = true;
			parse_advance(state);
			tok = parse_peek(state);
		}
		if (tok->type == SYM_RBRACE) {
			parse_advance(state);
			res.value.compound.last_empty = false;
			PARSE_LEAVE("Compound Expr");
		}
	}
}
struct expr parse_funk_call(struct parse_state *state) {
	struct expr res;

	PARSE_ENTER("Funk Call");

	struct token *tok = parse_advance(state);
	if (tok->type != IDENTIFIER) {
		ERR_UNEXPECTED_TOKEN("IDENTIFIER");
	}

	res.type = EXPR_FUNCALL;
	res.value.funcall = (struct funcall) {
		.items = NULL,
		.capacity = 0,
		.count = 0,
		.naam = tok->value.s_val,
	};
	tok->value.s_val = NULL;

	tok = parse_advance(state);
	if (tok->type != SYM_LPAREN) {
		ERR_UNEXPECTED_TOKEN("SYM_LPAREN");
	}
	tok = parse_peek(state);
	if (tok->type == SYM_RBRACE) {
		parse_advance(state);
		PARSE_LEAVE("Funk Call");
	}

	for (;;) {
		struct expr expr = parse_expr(state);
		nob_da_append(&res.value.funcall, expr);

		tok = parse_peek(state);
		/*if (tok->type == SYM_COMMA) {
			parse_advance(state);
			tok = parse_peek(state);
		} else*/ if (tok->type == SYM_RPAREN) {
			parse_advance(state);
			PARSE_LEAVE("Funk Call");
		} else {
			ERR_UNEXPECTED_TOKEN("SYM_RPAREN");
		}
	}
}
struct expr parse_expr(struct parse_state *state) {
	struct expr res;

	PARSE_ENTER("Expr");

	struct token *tok = parse_peek(state);

	if (tok->type == SYM_LBRACE) {
		res = parse_compound_expr(state);
	} else if (tok->type == IDENTIFIER) {
		res = parse_funk_call(state);
	} else if (tok->type == STRING_LIT) {
		res.type = EXPR_STR_LIT;
		res.value.str_lit = tok->value.s_val;
		tok->value.s_val = NULL;
		parse_advance(state);
	} else {
		ERR_UNEXPECTED_TOKEN("SYM_LBRACE or IDENTIFIER or STRING_LIT");
	}

	PARSE_LEAVE("Expr");
}
struct ast *parse_atexpr(struct parse_state *state) {
	struct ast *res;

	PARSE_ENTER("@");

	struct token *tok = parse_advance(state);
	if (tok->type != SYM_AT) {
		ERR_UNEXPECTED_TOKEN("SYM_AT");
	}

	tok = parse_advance(state);
	if (tok->type != IDENTIFIER) {
		ERR_UNEXPECTED_TOKEN("IDENTIFIER");
	}

	if (strcmp(tok->value.s_val, "sluit_in") == 0) {
		res = malloc(sizeof(*res));
		*res = (struct ast) { INSLUITING, {{0}} };

		tok = parse_advance(state);

		if (tok->type != IDENTIFIER) {
			ERR_UNEXPECTED_TOKEN("IDENTIFIER");
		}

		res->value.insluiting.module = tok->value.s_val;
		tok->value.s_val = NULL;
	} else {
		ERR_UNEXPECTED_TOKEN("IDENTIFIER");
	}

	PARSE_LEAVE("@");
}
struct funksie parse_funk(struct parse_state *state) {
	struct funksie res;

	PARSE_ENTER("Funk");

	struct token *tok = parse_advance(state);
	if (tok->type != KW_FUNK) {
		ERR_UNEXPECTED_TOKEN("KW_FUNK");
	}

	tok = parse_advance(state);
	if (tok->type != IDENTIFIER) {
		ERR_UNEXPECTED_TOKEN("IDENTIFIER");
	}

	res.naam = tok->value.s_val;
	tok->value.s_val = NULL;

	tok = parse_advance(state);
	if (tok->type != SYM_LPAREN) {
		ERR_UNEXPECTED_TOKEN("SYM_LPAREN");
	}
	tok = parse_advance(state);
	if (tok->type != SYM_RPAREN) {
		ERR_UNEXPECTED_TOKEN("SYM_RPAREN");
	}

	res.lyf = parse_expr(state);

	tok = parse_advance(state);
	if (tok->type != SYM_SEMICOLON) {
		ERR_UNEXPECTED_TOKEN("SYM_SEMICOLON");
	}

	PARSE_LEAVE("Funk");
}
struct ast *parse_ast(struct parse_state *state) {
	struct ast *res;

	PARSE_ENTER("AST");

	struct token *tok = parse_peek(state);

	if (tok->type == SYM_AT) {
		res = parse_atexpr(state);
	} else if (tok->type == KW_FUNK) {
		res = malloc(sizeof(*res));
		*res = (struct ast) {
			FUNKSIE_DEFINISIE,
			{ .funksie_definisie
			 = parse_funk(state) }
		};
	} else if (tok->type == TOK_EOF) {
		res = malloc(sizeof(*res));
		*res = (struct ast) { AST_EOF, {{0}} };
	} else {
		ERR_UNEXPECTED_TOKEN("SYM_AT or KW_FUNK or TOK_EOF");
	}

	PARSE_LEAVE("AST");
}
