#include "parser.h"

struct source_tracking st_init(const char *source) {
	return (struct source_tracking) {
		.source = source,
		.size = strlen(source),
		.idx = 0,
	};
}
#define src_atend() st_atend(src)
bool st_atend(const struct source_tracking *this) {
	return this->idx >= this->size;
}
#define src_at() st_at(src)
char st_at(const struct source_tracking *this) {
	return this->source[this->idx];
}
#define src_isat(c) st_isat(src, c)
bool st_isat(const struct source_tracking *this, char c) {
	return !st_atend(this) && st_at(this) == c;
}
#define src_advat(c) st_advat(src, c)
bool st_advat(struct source_tracking *this, char c) {
	if (st_isat(this, c)) {
		st_adv(this);
		return true;
	} else return false;
}
#define src_checkcur(check) st_checkcur(src, check)
bool st_checkcur(const struct source_tracking *this, bool (*check)(char)) {
	return !st_atend(this) && check(st_at(this));
}
#define src_skipwhile(cond) st_skipwhile(src, cond)
#define src_skipwhile_expr(cond_expr) do { \
		while (cond_expr) src_adv(); \
	} while (0)
void st_skipwhile(struct source_tracking *this, bool (*cond)(char)) {
	while (st_checkcur(this, cond)) st_adv(this);
}
#define src_canpeek() st_canpeek(src)
bool st_canpeek(const struct source_tracking *this) {
	return this->idx+1 < this->size;
}
#define src_peek() st_peek(src)
char st_peek(const struct source_tracking *this) {
	return this->source[this->idx+1];
}
#define src_adv() st_adv(src)
char st_adv(struct source_tracking *this) {
	return this->source[this->idx++];
}

#define PRINT_IMPL(typename) \
FPRINT_FUNC(typename) { \
	Nob_String_Builder sb = {0}; \
	sb_append_ ## typename(&sb, this); \
	fprintf(file, "%.*s", (int)sb.count, sb.items); \
	nob_sb_free(sb); \
} \
PRINT_FUNC(typename) { fprint_ ## typename(this, stdout); }

DESTROY_METH(token) {
	switch (this->type) {
#define X(tt) case tt: { if (this->value.s_val) free(this->value.s_val); } break;
		S_VAL_TOKENS
#undef X
		default: break;
	}
}
COPY_METH(token) {
	struct token res = {
		.type = this->type,
		.value = {0},
	};

	switch (this->type) {
#define X(tt) case tt: { if (this->value.s_val) res.value.s_val = strdup(this->value.s_val); } break;
		S_VAL_TOKENS
#undef X
		default: break;
	}

	return res;
}
SB_APPEND_FUNC(token) {
	switch (this->type) {
		case TOK_EOF: {
			nob_sb_appendf(sb, "TOK_EOF");
		} break;

#define X(tt) case tt: { nob_sb_appendf(sb, #tt "(%s)", this->value.s_val); } break;
		S_VAL_TOKENS
#undef X

#define X(tt, _) case tt: { nob_sb_append_cstr(sb, #tt); } break;
		CHAR_TOKENS

		KW_TOKENS
#undef X
	}
}
PRINT_IMPL(token)

struct token lex_token(struct source_tracking *src);

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

struct token lex_token(struct source_tracking *src) {
	struct token res = { EOF, {0} };

	src_skipwhile(is_space);

	if (src_atend()) return res;

#define X(tt, ch) if (src_advat(ch)) { res.type = tt; } else
	CHAR_TOKENS
#undef X
	if (src_checkcur(is_alpha) || src_isat('_')) {
		const size_t begin = src->idx;
		src_adv();

		src_skipwhile(is_alnum);

#define X(tt, st) if (strncmp(&src->source[begin], st, strlen(st)) == 0) { res.type = tt; } else
		KW_TOKENS
#undef X
		{
			res.type = IDENTIFIER;
			res.value.s_val = strndup(&src->source[begin], src->idx - begin);
		}
	} else if (src_isat('"')) {
		const size_t begin = src->idx;
		src_adv();

		src_skipwhile_expr(!src_atend() && src_at() != '"');

		if (src_isat('"')) src_adv();
		else {
			nob_log(NOB_ERROR, "Unterminated string literal");
			exit(1);
		}

		res.type = STRING_LIT;
		res.value.s_val = strndup(&src->source[begin+1], src->idx - begin - 2);
	} else {
		nob_log(NOB_ERROR, "Unexpected character %c in source", src_at());
		exit(1);
	}

	return res;
}

DESTROY_METH(expr) {
	switch (this->type) {
		case EXPR_FUNCALL: {
			for (size_t i = 0; i < this->value.funcall.count; ++i) {
				expr_destroy(&this->value.funcall.items[i]);
			}
			nob_da_free(this->value.funcall);
			free(this->value.funcall.naam);
		} break;
		case EXPR_COMPOUND: {
			for (size_t i = 0; i < this->value.compound.count; ++i) {
				expr_destroy(&this->value.compound.items[i]);
			}
			nob_da_free(this->value.compound);
		} break;
		case EXPR_STR_LIT: {
			free(this->value.str_lit);
		}
	}
}
COPY_METH(expr) {
	struct expr res = {
		.type = this->type,
		.value = {{0}},
	};
	switch (this->type) {
		case EXPR_FUNCALL: {
			for (size_t i = 0; i < this->value.funcall.count; ++i) {
				nob_da_append(&res.value.funcall, expr_copy(&this->value.funcall.items[i]));
			}
			res.value.funcall.naam = strdup(this->value.funcall.naam);
		} break;
		case EXPR_COMPOUND: {
			for (size_t i = 0; i < this->value.compound.count; ++i) {
				nob_da_append(&res.value.compound, expr_copy(&this->value.compound.items[i]));
			}
		} break;
		case EXPR_STR_LIT: {
			res.value.str_lit = strdup(this->value.str_lit);
		}
	}
	return res;
}
SB_APPEND_FUNC(expr) {
	switch (this->type) {
		case EXPR_FUNCALL: {
			nob_sb_appendf(sb, "FUNCALL(%s: ", this->value.funcall.naam);
			for (size_t i = 0; i < this->value.funcall.count; ++i) {
				if (i) nob_sb_appendf(sb, ", ");
				sb_append_expr(sb, &this->value.funcall.items[i]);
			}
			nob_sb_appendf(sb, ")");
		} break;
		case EXPR_COMPOUND: {
			nob_sb_appendf(sb, "COMPOUND(");
			for (size_t i = 0; i < this->value.funcall.count; ++i) {
				if (i) nob_sb_appendf(sb, ", ");
				sb_append_expr(sb, &this->value.funcall.items[i]);
			}
			nob_sb_appendf(sb, ")");
		} break;
		case EXPR_STR_LIT: {
			nob_sb_appendf(sb, "\"%s\"", this->value.str_lit);
		} break;
	}
}
PRINT_IMPL(expr)

DESTROY_METH(insluiting) {
	free(this->module);
}
COPY_METH(insluiting) {
	struct insluiting res = {0};
	res.module = strdup(this->module);
	return res;
}
SB_APPEND_FUNC(insluiting) {
	nob_sb_appendf(sb, "INSLUITING(%s)", this->module);
}
PRINT_IMPL(insluiting)

DESTROY_METH(funksie) {
	free(this->naam);
	expr_destroy(&this->lyf);
}
COPY_METH(funksie) {
	struct funksie res = {0};
	res.naam = strdup(this->naam);
	res.lyf = expr_copy(&this->lyf);
	return res;
}
SB_APPEND_FUNC(funksie) {
	nob_sb_appendf(sb, "FUNKSIE(%s, ", this->naam);
	sb_append_expr(sb, &this->lyf);
	nob_sb_appendf(sb, ")");
}
PRINT_IMPL(funksie)

DESTROY_METH(ast) {
	switch (this->type) {
		case EXPR: {
			expr_destroy(&this->value.expr);
		} break;
		case INSLUITING: {
			insluiting_destroy(&this->value.insluiting);
		} break;
		case FUNKSIE_DEFINISIE: {
			funksie_destroy(&this->value.funksie_definisie);
		} break;
		case AST_EOF: break;
	}
}
COPY_METH(ast) {
	struct ast res = {0};
	switch (this->type) {
		case EXPR: {
			res.value.expr = expr_copy(&this->value.expr);
		} break;
		case INSLUITING: {
			res.value.insluiting = insluiting_copy(&this->value.insluiting);
		} break;
		case FUNKSIE_DEFINISIE: {
			res.value.funksie_definisie = funksie_copy(&this->value.funksie_definisie);
		} break;
		case AST_EOF: break;
	}
	return res;
}
SB_APPEND_FUNC(ast) {
	switch (this->type) {
		case EXPR: {
			sb_append_expr(sb, &this->value.expr);
		} break;
		case INSLUITING: {
			sb_append_insluiting(sb, &this->value.insluiting);
		} break;
		case FUNKSIE_DEFINISIE: {
			sb_append_funksie(sb, &this->value.funksie_definisie);
		} break;
		case AST_EOF: {
			nob_sb_appendf(sb, "AST_EOF");
		} break;
	}
}
PRINT_IMPL(ast)

struct token *parse_peek(struct parse_state *state) {
	if (!state->has_tok) {
		state->tok = lex_token(&state->src);
		state->has_tok = true;
	}
	return &state->tok;
}
struct token *parse_advance(struct parse_state *state) {
	parse_peek(state);

	if (state->has_prev) {
		token_destroy(&state->prev);
	}

	state->prev = state->tok;
	state->has_prev = true;
	state->has_tok = false;

	return &state->prev;
}

struct parse_state parse_state_init(const char *source) {
	return (struct parse_state) {
		.src = st_init(source),

		.verbose = false,

		.prev = {0},
		.has_prev = false,
		.tok = {0},
		.has_tok = false,
	};
}
#define ERR_UNEXPECTED_TOKEN(expected_t_msg) do { \
		nob_sb_append_cstr(&err, "Unexpected token in source "); \
		sb_append_token(&err, tok); \
		nob_sb_append_cstr(&err, ", expected token of type " expected_t_msg); \
		nob_log(NOB_ERROR, "%s,%d: %s", __FILE__, __LINE__, err.items); \
		exit(1); \
	 } while (0)
#define VERB(str) if (state->verbose) nob_log(NOB_INFO, str)
#define VERBF(fmt, ...) if (state->verbose) nob_log(NOB_INFO, fmt, __VA_ARGS__)
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
	struct expr res = {0};

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
	struct expr res = {0};

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
	struct expr res = {0};

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
struct ast parse_atexpr(struct parse_state *state) {
	struct ast res = {0};

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
		res.type = INSLUITING;

		tok = parse_advance(state);

		if (tok->type != IDENTIFIER) {
			ERR_UNEXPECTED_TOKEN("IDENTIFIER");
		}

		res.value.insluiting.module = tok->value.s_val;
		tok->value.s_val = NULL;
	} else {
		ERR_UNEXPECTED_TOKEN("IDENTIFIER");
	}

	PARSE_LEAVE("@");
}
struct funksie parse_funk(struct parse_state *state) {
	struct funksie res = {0};

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
struct ast parse_ast(struct parse_state *state) {
	struct ast res = {0};

	PARSE_ENTER("AST");

	struct token *tok = parse_peek(state);

	if (tok->type == SYM_AT) {
		res = parse_atexpr(state);
	} else if (tok->type == KW_FUNK) {
		res.type = FUNKSIE_DEFINISIE;
		res.value.funksie_definisie = parse_funk(state);
	} else if (tok->type == TOK_EOF) {
		res.type = AST_EOF;
	} else {
		ERR_UNEXPECTED_TOKEN("SYM_AT or KW_FUNK or TOK_EOF");
	}

	PARSE_LEAVE("AST");
}

DESTROY_METH(program) {
	nob_da_foreach(struct ast, ast, this) {
		ast_destroy(ast);
	}
	nob_da_free(*this);
}
COPY_METH(program) {
	struct program res = {0};
	nob_da_reserve(&res, this->count);
	nob_da_foreach(struct ast, ast, this) {
		nob_da_append(&res, ast_copy(ast));
	}
	return res;
}
SB_APPEND_FUNC(program) {
	nob_da_foreach(struct ast, ast, this) {
		sb_append_ast(sb, ast);
		nob_sb_append_cstr(sb, "\n");
	}
}
PRINT_IMPL(program)
struct program parse_program(struct parse_state *state) {
	struct program res = {0};

	while (!st_atend(&state->src)) {
		nob_da_append(&res, parse_ast(state));
	}

	return res;
}
