#include "lexer.h"

struct source_tracking st_init(const char *filename, const char *source) {
	assert(source != NULL);

	return (struct source_tracking) {
		.filename = filename,
		.source = source,
		.size = strlen(source),
		.idx = 0,
		.line_start = 0,
		.line = 1,
	};
}
#define src_atend() st_atend(src)
bool st_atend(const struct source_tracking *this) {
	assert(this != NULL);

	return this->idx >= this->size;
}
#define src_at() st_at(src)
char st_at(const struct source_tracking *this) {
	assert(this != NULL && !st_atend(this));

	return this->source[this->idx];
}
#define src_isat(c) st_isat(src, c)
bool st_isat(const struct source_tracking *this, char c) {
	assert(this != NULL);

	return !st_atend(this) && st_at(this) == c;
}
#define src_match(str) st_match(src, str)
bool st_match(const struct source_tracking *this, const char *str) {
	assert(this != NULL);

	return this->size - this->idx >= strlen(str) && strncmp(&this->source[this->idx], str, strlen(str)) == 0;
}
#define src_advat(c) st_advat(src, c)
bool st_advat(struct source_tracking *this, char c) {
	assert(this != NULL);

	if (!st_atend(this) && st_isat(this, c)) {
		st_adv(this);
		return true;
	} else return false;
}
#define src_checkcur(check) st_checkcur(src, check)
bool st_checkcur(const struct source_tracking *this, bool (*check)(char)) {
	assert(this != NULL);

	return !st_atend(this) && check(st_at(this));
}
#define src_skipwhile(cond) st_skipwhile(src, cond)
#define src_skipwhile_expr(cond_expr) do { \
		while (cond_expr) src_adv(); \
	} while (0)
void st_skipwhile(struct source_tracking *this, bool (*cond)(char)) {
	assert(this != NULL);

	while (st_checkcur(this, cond)) st_adv(this);
}
#define src_canpeek() st_canpeek(src)
bool st_canpeek(const struct source_tracking *this) {
	assert(this != NULL);

	return this->idx+1 < this->size;
}
#define src_peek() st_peek(src)
char st_peek(const struct source_tracking *this) {
	assert(st_canpeek(this) && this != NULL);

	return this->source[this->idx+1];
}
#define src_adv() st_adv(src)
char st_adv(struct source_tracking *this) {
	assert(!st_atend(this) && this != NULL);

	const char res = this->source[this->idx];
	++this->idx;
	if (res == '\n') {
		this->line_start = this->idx;
		++this->line;
	}
	return res;
}
#define src_advby(by) st_advby(src, by)
void st_advby(struct source_tracking *this, size_t by) {
	assert(this != NULL && this->size - this->idx >= by);

	for (size_t i = 0; i < by; ++i) st_adv(this);
}

DESTROY_METH(token) {
	assert(this != NULL);
}
COPY_METH(token) {
	assert(this != NULL);

	struct token res = {
		.type = this->type,
		.pos = {0},
		.len = 0,
	};

	res.pos = this->pos;
	res.len = this->len;

	return res;
}
SB_APPEND_FUNC(token) {
	assert(sb != NULL && this != NULL);

	switch (this->type) {
		case TOK_EOF: {
			nob_sb_appendf(sb, "TOK_EOF");
		} break;
		case TOK_UNKNOWN: {
			nob_sb_appendf(sb, "TOK_UNKNOWN");
		} break;

#define X(tt) case tt: { nob_sb_append_cstr(sb, #tt); } break;
		S_VAL_TOKENS
#undef X

#define X(tt, _) case tt: { nob_sb_append_cstr(sb, #tt); } break;
		SYMBOL_TOKENS

		KW_TOKENS
#undef X
	}

	if (this->len != 0) {
		nob_sb_appendf(sb, " (%s:%lu \"%.*s\")", this->pos.filename, this->pos.idx, (int)this->len, &this->pos.source[this->pos.idx]);
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
	assert(src != NULL && src->source != NULL);

	struct token res = {0};

	src_skipwhile(is_space);

	if (src_atend()) return res;

#define X(tt, ch) if (src_match(ch)) { \
		res.type = tt; \
		res.pos = *src; \
		res.len = strlen(ch); \
		src_advby(strlen(ch)); \
	} else
	SYMBOL_TOKENS
#undef X
	if (src_checkcur(is_alpha) || src_isat('_')) {
		const struct source_tracking begin = *src;

		src_skipwhile(is_alnum);

		const size_t len = src->idx - begin.idx;

#define X(tt, st) if (strncmp(&begin.source[begin.idx], st, strlen(st)) == 0) { \
		res.type = tt; \
		res.pos = begin; \
		res.len = len; \
	} else
		KW_TOKENS
#undef X
		{
			res.type = IDENTIFIER;
			res.pos = begin;
			res.len = len;
		}
	} else if (src_isat('"')) {
		const struct source_tracking begin = *src;
		src_adv();

		// TODO: proper string parsing including escape sequences
		src_skipwhile_expr(!src_atend() && src_at() != '"');

		if (src_isat('"')) src_adv();
		else {
			nob_log(NOB_ERROR, "Unterminated string literal");
			exit(1);
		}

		res.type = STRING_LIT;
		res.pos = begin;
		res.len = src->idx - begin.idx;
	} else if (src_checkcur(is_digit)) {
		const struct source_tracking begin = *src;

		src_skipwhile(is_digit);

		res.type = NUMBER_LIT;
		res.pos = begin;
		res.len = src->idx - begin.idx;
	} else {
		res.type = TOK_UNKNOWN;
		res.pos = *src;
		res.len = 1;
		src_adv();
	}

	return res;
}
