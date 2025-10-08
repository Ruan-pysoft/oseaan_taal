#define NOB_IMPLEMENTATION
#include "nob.h"

enum token_type {
	TOK_EOF = -1,

	IDENTIFIER,
	STRING_LIT,

	SYM_AT,
	SYM_SEMICOLON,

	SYM_LPAREN,
	SYM_RPAREN,
	SYM_LBRACE,
	SYM_RBRACE,

	KW_FUNK,
};

union token_value {
	char *s_val;
};

struct token {
	enum token_type type;
	union token_value value;
};
void token_destroy(struct token tok) {
	if (tok.type == IDENTIFIER) {
		if (tok.value.s_val) free(tok.value.s_val);
	} else if (tok.type == STRING_LIT) {
		if (tok.value.s_val) free(tok.value.s_val);
	}
}
void print_token(struct token tok) {
	switch (tok.type) {
		case TOK_EOF: {
			printf("TOK_EOF");
		} break;

		case IDENTIFIER: {
			printf("IDENTIFIER(%s)", tok.value.s_val);
		} break;
		case STRING_LIT: {
			printf("STRING_LIT(%s)", tok.value.s_val);
		} break;

		case SYM_AT: {
			printf("SYM_AT");
		} break;
		case SYM_SEMICOLON: {
			printf("SYM_SEMICOLON");
		} break;

		case SYM_LPAREN: {
			printf("SYM_LPAREN");
		} break;
		case SYM_RPAREN: {
			printf("SYM_RPAREN");
		} break;
		case SYM_LBRACE: {
			printf("SYM_LBRACE");
		} break;
		case SYM_RBRACE: {
			printf("SYM_RBRACE");
		} break;

		case KW_FUNK: {
			printf("KW_FUNK");
		} break;
	}
}
void sb_append_token(Nob_String_Builder *sb, struct token tok) {
	switch (tok.type) {
		case TOK_EOF: {
			nob_sb_appendf(sb, "TOK_EOF");
		} break;

		case IDENTIFIER: {
			nob_sb_appendf(sb, "IDENTIFIER(%s)", tok.value.s_val);
		} break;
		case STRING_LIT: {
			nob_sb_appendf(sb, "STRING_LIT(%s)", tok.value.s_val);
		} break;

		case SYM_AT: {
			nob_sb_appendf(sb, "SYM_AT");
		} break;
		case SYM_SEMICOLON: {
			nob_sb_appendf(sb, "SYM_SEMICOLON");
		} break;

		case SYM_LPAREN: {
			nob_sb_appendf(sb, "SYM_LPAREN");
		} break;
		case SYM_RPAREN: {
			nob_sb_appendf(sb, "SYM_RPAREN");
		} break;
		case SYM_LBRACE: {
			nob_sb_appendf(sb, "SYM_LBRACE");
		} break;
		case SYM_RBRACE: {
			nob_sb_appendf(sb, "SYM_RBRACE");
		} break;

		case KW_FUNK: {
			nob_sb_appendf(sb, "KW_FUNK");
		} break;
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

	if (source[*idx] == '@') {
		++*idx;
		res.type = SYM_AT;
	} else if (source[*idx] == ';') {
		++*idx;
		res.type = SYM_SEMICOLON;
	} else if (source[*idx] == '(') {
		++*idx;
		res.type = SYM_LPAREN;
	} else if (source[*idx] == ')') {
		++*idx;
		res.type = SYM_RPAREN;
	} else if (source[*idx] == '{') {
		++*idx;
		res.type = SYM_LBRACE;
	} else if (source[*idx] == '}') {
		++*idx;
		res.type = SYM_RBRACE;
	} else if (is_alpha(source[*idx]) || source[*idx] == '_') {
		const size_t begin = *idx;
		++*idx;

		while (*idx < source_len && is_alnum(source[*idx])) {
			++*idx;
		}

		if (strncmp(&source[begin], "funk", 4) == 0) {
			res.type = KW_FUNK;
		} else {
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
	const char *str_lit;
};

struct expr {
	enum expr_type type;
	union expr_value value;
};
void expr_destroy(struct expr expr) {
	if (expr.type == EXPR_FUNCALL) {
		for (size_t i = 0; i < expr.value.funcall.count; ++i) {
			expr_destroy(expr.value.funcall.items[i]);
		}
		free(expr.value.funcall.items);
		free(expr.value.funcall.naam);
	}
}
void print_expr(struct expr expr) {
	switch (expr.type) {
		case EXPR_FUNCALL: {
			printf("FUNCALL(%s: ", expr.value.funcall.naam);
			for (size_t i = 0; i < expr.value.funcall.count; ++i) {
				if (i) printf(", ");
				print_expr(expr.value.funcall.items[i]);
			}
			printf(")");
		} break;
		case EXPR_COMPOUND: {
			printf("COMPOUND(");
			for (size_t i = 0; i < expr.value.funcall.count; ++i) {
				if (i) printf(", ");
				print_expr(expr.value.funcall.items[i]);
			}
			printf(")");
		} break;
		case EXPR_STR_LIT: {
			printf("\"%s\"", expr.value.str_lit);
		} break;
	}
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

struct insluiting {
	char *module;
};
void insluiting_destroy(struct insluiting insluiting) {
	free(insluiting.module);
}
void print_insluiting(struct insluiting insluiting) {
	printf("INSLUITING(%s)", insluiting.module);
}
void sb_append_insluiting(Nob_String_Builder *sb, struct insluiting insluiting) {
	nob_sb_appendf(sb, "INSLUITING(%s)", insluiting.module);
}

struct funksie {
	char *naam;
	struct expr lyf;
};
void funksie_destroy(struct funksie funksie) {
	free(funksie.naam);
	expr_destroy(funksie.lyf);
}
void print_funksie(struct funksie funksie) {
	printf("FUNKSIE(%s, ", funksie.naam);
	print_expr(funksie.lyf);
	printf(")");
}
void sb_append_funksie(Nob_String_Builder *sb, struct funksie funksie) {
	nob_sb_appendf(sb, "FUNKSIE(%s, ", funksie.naam);
	sb_append_expr(sb, funksie.lyf);
	nob_sb_appendf(sb, ")");
}

union ast_value {
	struct expr expr;
	struct insluiting insluiting;
	struct funksie funksie_definisie;
};

struct ast {
	enum ast_type type;
	union ast_value value;
};
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
	switch (ast->type) {
		case EXPR: {
			print_expr(ast->value.expr);
		} break;
		case INSLUITING: {
			print_insluiting(ast->value.insluiting);
		} break;
		case FUNKSIE_DEFINISIE: {
			print_funksie(ast->value.funksie_definisie);
		} break;
		case AST_EOF: {
			printf("AST_EOF");
		} break;
	}
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

struct parse_state {
	const char *source;
	size_t source_len;
	size_t idx;

	struct token prev;
	bool has_prev;
	struct token tok;
	bool has_tok;
};

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

struct expr parse_expr(struct parse_state *state);
struct expr parse_compound_expr(struct parse_state *state) {
	struct expr res;

	Nob_String_Builder err = {0};
	nob_sb_append_cstr(&err, "Compound Expr: ");

	struct token *tok = parse_advance(state);
	if (tok->type != SYM_LBRACE) {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
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
		return res;
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
			nob_sb_free(err);
			return res;
		}
	}
}
struct expr parse_funk_call(struct parse_state *state) {
	struct expr res;

	Nob_String_Builder err = {0};
	nob_sb_append_cstr(&err, "Funk Call: ");

	struct token *tok = parse_advance(state);
	if (tok->type != IDENTIFIER) {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
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
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}
	tok = parse_peek(state);
	if (tok->type == SYM_RBRACE) {
		parse_advance(state);
		return res;
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
			nob_sb_free(err);
			return res;
		} else {
			nob_sb_append_cstr(&err, "Unexpected token in source ");
			sb_append_token(&err, *tok);
			nob_log(NOB_ERROR, "%s", err.items);
			exit(1);
		}
	}
}
struct expr parse_expr(struct parse_state *state) {
	struct expr res;

	Nob_String_Builder err = {0};
	nob_sb_append_cstr(&err, "Expr: ");

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
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}

	nob_sb_free(err);
	return res;
}
struct ast *parse_atexpr(struct parse_state *state) {
	struct ast *res;

	Nob_String_Builder err = {0};
	nob_sb_append_cstr(&err, "@: ");

	struct token *tok = parse_advance(state);
	if (tok->type != SYM_AT) {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}

	tok = parse_advance(state);
	if (tok->type != IDENTIFIER) {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}

	if (strcmp(tok->value.s_val, "sluit_in") == 0) {
		res = malloc(sizeof(*res));
		*res = (struct ast) { INSLUITING, {{0}} };

		tok = parse_advance(state);

		if (tok->type != IDENTIFIER) {
			nob_sb_append_cstr(&err, "Unexpected token in source ");
			sb_append_token(&err, *tok);
			nob_log(NOB_ERROR, "%s", err.items);
			exit(1);
		}

		res->value.insluiting.module = tok->value.s_val;
		tok->value.s_val = NULL;
	} else {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}

	nob_sb_free(err);
	return res;
}
struct funksie parse_funk(struct parse_state *state) {
	struct funksie res;

	Nob_String_Builder err = {0};
	nob_sb_append_cstr(&err, "Funk: ");

	struct token *tok = parse_advance(state);
	if (tok->type != KW_FUNK) {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}

	tok = parse_advance(state);
	if (tok->type != IDENTIFIER) {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}

	res.naam = tok->value.s_val;
	tok->value.s_val = NULL;

	tok = parse_advance(state);
	if (tok->type != SYM_LPAREN) {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}
	tok = parse_advance(state);
	if (tok->type != SYM_RPAREN) {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}

	res.lyf = parse_expr(state);

	tok = parse_advance(state);
	if (tok->type != SYM_SEMICOLON) {
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}

	nob_sb_free(err);
	return res;
}
struct ast *parse_ast(struct parse_state *state) {
	struct ast *res;

	Nob_String_Builder err = {0};
	nob_sb_append_cstr(&err, "AST: ");

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
		nob_sb_append_cstr(&err, "Unexpected token in source ");
		sb_append_token(&err, *tok);
		nob_log(NOB_ERROR, "%s", err.items);
		exit(1);
	}

	nob_sb_free(err);
	return res;
}

enum oseaan_type {
	TP_NIKS = 0,
	TP_STR,
};
union oseaan_value_union {
	char *s_value;
};
struct oseaan_value {
	enum oseaan_type type;
	union oseaan_value_union value;
};
void value_destroy(struct oseaan_value value) {
	switch (value.type) {
		case TP_NIKS: break;
		case TP_STR: {
			free(value.value.s_value);
		} break;
	}
}
void print_value(struct oseaan_value value) {
	switch (value.type) {
		case TP_NIKS: {
			printf("niks");
		} break;
		case TP_STR: {
			printf("teks(%s)", value.value.s_value);
		} break;
	}
}
void sb_append_value(Nob_String_Builder *sb, struct oseaan_value value) {
	switch (value.type) {
		case TP_NIKS: {
			nob_sb_appendf(sb, "niks");
		} break;
		case TP_STR: {
			nob_sb_appendf(sb, "teks(%s)", value.value.s_value);
		} break;
	}
}

struct program_state {
};

struct oseaan_value eval_expr(struct program_state *state, struct expr expr) {
	struct oseaan_value res = {0};

	switch (expr.type) {
		case EXPR_FUNCALL: {
			if (strcmp(expr.value.funcall.naam, "druk_lyn") == 0) {
				for (size_t i = 0; i < expr.value.funcall.count; ++i) {
					struct oseaan_value val = eval_expr(state, expr.value.funcall.items[i]);
					print_value(val);
					value_destroy(val);
				}
				putchar('\n');
			} else {
				printf("TODO: parse funksie call\n");
				printf("Naam: %s\n", expr.value.funcall.naam);
			}
		} break;
		case EXPR_COMPOUND: {
			for (size_t i = 0; i < expr.value.compound.count; ++i) {
				struct oseaan_value val = eval_expr(state, expr.value.compound.items[i]);
				printf("In compound expression, got expression value: ");
				print_value(val);
				putchar('\n');
				if (!expr.value.compound.last_empty) {
					value_destroy(res);
					res = val;
				} else {
					value_destroy(val);
				}
			}
		} break;
		case EXPR_STR_LIT: {
			res.type = TP_STR;
			res.value.s_value = strdup(expr.value.str_lit);
		} break;
	}

	return res;
}
void eval_insluiting(struct program_state *state, struct insluiting insluiting) {
	(void) state;
	(void) insluiting;
	printf("TODO: parse insluiting\n");
}
void eval_funksie_definisie(struct program_state *state, struct funksie funksie) {
	if (strcmp(funksie.naam, "hoof") == 0) {
		struct oseaan_value val = eval_expr(state, funksie.lyf);
		printf("Program result (should be niks): ");
		print_value(val);
		putchar('\n');
		value_destroy(val);
	} else {
		printf("TODO: parse funksie definisie\n");
		printf("Naam: %s\n", funksie.naam);
	}
}
void eval(struct program_state *state, struct ast *ast) {
	switch (ast->type) {
		case AST_EOF: break;
		case EXPR: {
			struct oseaan_value val = eval_expr(state, ast->value.expr);
			printf("Got expression value: ");
			print_value(val);
			putchar('\n');
			value_destroy(val);
		} break;
		case INSLUITING: {
			eval_insluiting(state, ast->value.insluiting);
		} break;
		case FUNKSIE_DEFINISIE: {
			eval_funksie_definisie(state, ast->value.funksie_definisie);
		} break;
	}
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	Nob_String_Builder sb = {0};

	if (!nob_read_entire_file("examples/hallo_wereld.os", &sb)) exit(1);

	struct parse_state state = {
		.source = sb.items,
		.source_len = sb.count,
		.idx = 0,

		.tok = {0},
		.has_tok = false,
	};
	struct program_state pstate = {
	};

	while (state.idx < state.source_len) {
		struct ast *ast = parse_ast(&state);
		printf("AST: ");
		print_ast(ast);
		putchar('\n');
		eval(&pstate, ast);
		ast_destroy(ast);
		free(ast);
	}
	/*
	while (idx < sb.count) {
		struct token tok = lex_token(sb.items, sb.count, &idx);
		printf("TOKEN: %d", tok.type);
		if (tok.type == IDENTIFIER || tok.type == STRING_LIT) {
			printf(" (%s)", tok.value.s_val);
		}
		putchar('\n');
	}
	*/
}
