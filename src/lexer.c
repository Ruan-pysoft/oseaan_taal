#include "lexer.h"
#include "utils.h"

struct source_tracking st_init(const char *filename, const char *source) {
	bt_assert(source != NULL);

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
	bt_assert(this != NULL);

	return this->idx >= this->size;
}
#define src_at() st_at(src)
char st_at(const struct source_tracking *this) {
	bt_assert(this != NULL && !st_atend(this));

	return this->source[this->idx];
}
#define src_isat(c) st_isat(src, c)
bool st_isat(const struct source_tracking *this, char c) {
	bt_assert(this != NULL);

	return !st_atend(this) && st_at(this) == c;
}
#define src_match(str) st_match(src, str)
bool st_match(const struct source_tracking *this, const char *str) {
	bt_assert(this != NULL);

	return this->size - this->idx >= strlen(str) && strncmp(&this->source[this->idx], str, strlen(str)) == 0;
}
#define src_advat(c) st_advat(src, c)
bool st_advat(struct source_tracking *this, char c) {
	bt_assert(this != NULL);

	if (!st_atend(this) && st_isat(this, c)) {
		st_adv(this);
		return true;
	} else return false;
}
#define src_checkcur(check) st_checkcur(src, check)
bool st_checkcur(const struct source_tracking *this, bool (*check)(char)) {
	bt_assert(this != NULL);

	return !st_atend(this) && check(st_at(this));
}
#define src_skipwhile(cond) st_skipwhile(src, cond)
#define src_skipwhile_expr(cond_expr) do { \
		while (cond_expr) src_adv(); \
	} while (0)
void st_skipwhile(struct source_tracking *this, bool (*cond)(char)) {
	bt_assert(this != NULL);

	while (st_checkcur(this, cond)) st_adv(this);
}
#define src_canpeek() st_canpeek(src)
bool st_canpeek(const struct source_tracking *this) {
	bt_assert(this != NULL);

	return this->idx+1 < this->size;
}
#define src_peek() st_peek(src)
char st_peek(const struct source_tracking *this) {
	bt_assert(st_canpeek(this) && this != NULL);

	return this->source[this->idx+1];
}
#define src_adv() st_adv(src)
char st_adv(struct source_tracking *this) {
	bt_assert(!st_atend(this) && this != NULL);

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
	bt_assert(this != NULL && this->size - this->idx >= by);

	for (size_t i = 0; i < by; ++i) st_adv(this);
}

#define src_to_sp(len) st_to_sp(src, len)
struct src_pos st_to_sp(const struct source_tracking *this, size_t len) {
	bt_assert(this != NULL);

	return (struct src_pos) {
		.filename = this->filename,
		.source = this->source,
		.size = this->size,
		.idx = this->idx,
		.line_start = this->line_start,
		.line = this->line,
		.len = len,
	};
}

DESTROY_METH(token) {
	bt_assert(this != NULL);
}
COPY_METH(token) {
	bt_assert(this != NULL);

	struct token res = {
		.type = this->type,
		.pos = {0},
	};

	res.pos = this->pos;

	return res;
}
SB_APPEND_FUNC(token) {
	bt_assert(sb != NULL && this != NULL);

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

	if (this->pos.len != 0) {
		nob_sb_appendf(
			sb, " ("SRC_POS_LOC_FMT" \""SRC_POS_FMT"\")",
			SRC_POS_LOC_FARGS(this->pos), SRC_POS_FARGS(this->pos)
		);
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
	bt_assert(src != NULL && src->source != NULL);

	struct token res = {0};

	do {
		if (src_match("//")) {
			src_skipwhile_expr(!src_atend() && src_at() != '\n');
		}

		src_skipwhile(is_space);

		if (src_atend()) return res;
	} while (src_match("//"));

#define X(tt, ch) if (src_match(ch)) { \
		res.type = tt; \
		res.pos = src_to_sp(strlen(ch)); \
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
		res.pos = st_to_sp(&begin, len); \
	} else
		KW_TOKENS
#undef X
		{
			res.type = IDENTIFIER;
			res.pos = st_to_sp(&begin, len);
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
		res.pos = st_to_sp(&begin, src->idx - begin.idx);
	} else if (src_checkcur(is_digit)) {
		const struct source_tracking begin = *src;

		src_skipwhile(is_digit);

		res.type = NUMBER_LIT;
		res.pos = st_to_sp(&begin, src->idx - begin.idx);
	} else {
		res.type = TOK_UNKNOWN;
		res.pos = src_to_sp(1);
		src_adv();
	}

	return res;
}
