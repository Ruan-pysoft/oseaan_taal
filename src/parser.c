#include "parser.h"

#include <assert.h>
#include <unistd.h>

#include "nob.h"

#include "lexer.h"
#include "types.h"
#include "utils.h"

DESTROY_METH(tipeerde_naam) {
	assert(this != NULL);

	token_destroy(&this->naam);
	tipe_destroy(&this->tipe);
}
COPY_METH(tipeerde_naam) {
	assert(this != NULL);

	struct tipeerde_naam res = {0};
	res.naam = token_copy(&this->naam);
	res.tipe = tipe_copy(&this->tipe);

	return res;
}
SB_APPEND_FUNC(tipeerde_naam) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "NAAM(%.*s: ", (int)this->naam.len, &this->naam.pos.source[this->naam.pos.idx]);
	sb_append_tipe(sb, &this->tipe);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(tipeerde_naam)

DESTROY_METH(st_insluiting) {
	assert(this != NULL);

	token_destroy(&this->module);
}
COPY_METH(st_insluiting) {
	assert(this != NULL);

	struct st_insluiting res = {0};
	res.module = token_copy(&this->module);

	return res;
}
SB_APPEND_FUNC(st_insluiting) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "SLUIT_IN(%.*s)", (int)this->module.len, &this->module.pos.source[this->module.pos.idx]);
}
PRINT_IMPL(st_insluiting)

DESTROY_METH(st_deklarasie) {
	assert(this != NULL);

	tipeerde_naam_destroy(&this->veranderlike);
}
COPY_METH(st_deklarasie) {
	assert(this != NULL);

	struct st_deklarasie res = {0};
	res.veranderlike = tipeerde_naam_copy(&this->veranderlike);

	return res;
}
SB_APPEND_FUNC(st_deklarasie) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "DEKL(");
	sb_append_tipeerde_naam(sb, &this->veranderlike);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(st_deklarasie)

DESTROY_METH(st_definisie) {
	assert(this != NULL);

	tipeerde_naam_destroy(&this->veranderlike);
	expr_destroy(this->wat);
	free(this->wat);
}
COPY_METH(st_definisie) {
	assert(this != NULL);

	struct st_definisie res = {0};
	res.veranderlike = tipeerde_naam_copy(&this->veranderlike);
	res.wat = malloc(sizeof(*res.wat));
	*res.wat = expr_copy(this->wat);

	return res;
}
SB_APPEND_FUNC(st_definisie) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "DEF(");
	sb_append_tipeerde_naam(sb, &this->veranderlike);
	nob_sb_append_cstr(sb, " = ");
	sb_append_expr(sb, this->wat);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(st_definisie)

DESTROY_METH(st_funksie) {
	assert(this != NULL);

	token_destroy(&this->naam);
	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		tipeerde_naam_destroy(it);
	}
	nob_da_free(this->argumente);
	if (this->benoemde_terugkeerwaarde) {
		tipeerde_naam_destroy(&this->benoem);
	} else {
		tipe_destroy(&this->onbenoem);
	}
	expr_destroy(this->lyf);
	free(this->lyf);
}
COPY_METH(st_funksie) {
	assert(this != NULL);

	struct st_funksie res = {0};
	res.naam = token_copy(&this->naam);
	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		nob_da_append(&res.argumente, tipeerde_naam_copy(it));
	}
	res.benoemde_terugkeerwaarde = this->benoemde_terugkeerwaarde;
	if (this->benoemde_terugkeerwaarde) {
		res.benoem = tipeerde_naam_copy(&this->benoem);
	} else {
		res.onbenoem = tipe_copy(&this->onbenoem);
	}
	res.lyf = malloc(sizeof(*this->lyf));
	*res.lyf = expr_copy(this->lyf);

	return res;
}
SB_APPEND_FUNC(st_funksie) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "FUNK(%.*s, (", (int)this->naam.len, &this->naam.pos.source[this->naam.pos.idx]);
	bool first = true;
	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		if (!first) {
			nob_sb_append_cstr(sb, ", ");
		}
		first = false;
		sb_append_tipeerde_naam(sb, it);
	}
	nob_sb_append_cstr(sb, ") -> ");
	if (this->benoemde_terugkeerwaarde) {
		sb_append_tipeerde_naam(sb, &this->benoem);
	} else {
		nob_sb_append_cstr(sb, ":");
		sb_append_tipe(sb, &this->onbenoem);
	}
	nob_sb_append_cstr(sb, " = ");
	sb_append_expr(sb, this->lyf);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(st_funksie)

DESTROY_METH(statement) {
	assert(this != NULL);

	switch (this->type) {
		case ST_INSLUITING: st_insluiting_destroy(&this->insluiting); break;
		case ST_DEKLARASIE: st_deklarasie_destroy(&this->deklarasie); break;
		case ST_DEFINISIE: st_definisie_destroy(&this->definisie); break;
		case ST_FUNKSIE: st_funksie_destroy(&this->funksie); break;
	}
}
COPY_METH(statement) {
	assert(this != NULL);

	struct statement res = {0};
	res.type = this->type;
	switch (this->type) {
		case ST_INSLUITING: res.insluiting = st_insluiting_copy(&this->insluiting); break;
		case ST_DEKLARASIE: res.deklarasie = st_deklarasie_copy(&this->deklarasie); break;
		case ST_DEFINISIE: res.definisie = st_definisie_copy(&this->definisie); break;
		case ST_FUNKSIE: res.funksie = st_funksie_copy(&this->funksie); break;
	}
	return res;
}
SB_APPEND_FUNC(statement) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "STMT: ");
	switch (this->type) {
		case ST_INSLUITING: sb_append_st_insluiting(sb, &this->insluiting); break;
		case ST_DEKLARASIE: sb_append_st_deklarasie(sb, &this->deklarasie); break;
		case ST_DEFINISIE: sb_append_st_definisie(sb, &this->definisie); break;
		case ST_FUNKSIE: sb_append_st_funksie(sb, &this->funksie); break;
	}
}
PRINT_IMPL(statement)

DESTROY_METH(program) {
	assert(this != NULL);

	nob_da_foreach(struct statement, it, this) {
		statement_destroy(it);
	}
	nob_da_free(*this);
}
COPY_METH(program) {
	assert(this != NULL);

	struct program res = {0};
	nob_da_foreach(struct statement, it, this) {
		nob_da_append(&res, statement_copy(it));
	}

	return res;
}
SB_APPEND_FUNC(program) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "PROGRAM WITH %lu STATEMENTS:\n", this->count);
	nob_da_foreach(struct statement, it, this) {
		nob_sb_append_cstr(sb, " - ");
		sb_append_statement(sb, it);
		nob_sb_append_cstr(sb, "\n");
	}
}
PRINT_IMPL(program)

DESTROY_METH(et_blok) {
	assert(this != NULL);

	nob_da_foreach(struct expr, it, this) {
		expr_destroy(it);
	}
	nob_da_free(*this);
}
COPY_METH(et_blok) {
	assert(this != NULL);

	struct et_blok res = {0};
	nob_da_foreach(struct expr, it, this) {
		nob_da_append(&res, expr_copy(it));
	}

	return res;
}
SB_APPEND_FUNC(et_blok) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "BLOK { ");
	nob_da_foreach(struct expr, it, this) {
		sb_append_expr(sb, it);
		nob_sb_append_cstr(sb, "; ");
	}
	nob_sb_append_cstr(sb, "}");
}
PRINT_IMPL(et_blok)

DESTROY_METH(et_funk) {
	assert(this != NULL);

	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		tipeerde_naam_destroy(it);
	}
	nob_da_free(this->argumente);
	if (this->benoemde_terugkeerwaarde) {
		tipeerde_naam_destroy(&this->benoem);
	} else {
		tipe_destroy(&this->onbenoem);
	}
	expr_destroy(this->lyf);
	free(this->lyf);
}
COPY_METH(et_funk) {
	assert(this != NULL);

	struct et_funk res = {0};
	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		nob_da_append(&res.argumente, tipeerde_naam_copy(it));
	}
	res.benoemde_terugkeerwaarde = this->benoemde_terugkeerwaarde;
	if (this->benoemde_terugkeerwaarde) {
		res.benoem = tipeerde_naam_copy(&this->benoem);
	} else {
		res.onbenoem = tipe_copy(&this->onbenoem);
	}
	res.lyf = malloc(sizeof(*this->lyf));
	*res.lyf = expr_copy(this->lyf);

	return res;
}
SB_APPEND_FUNC(et_funk) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "FUNK((");
	bool first = true;
	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		if (!first) {
			nob_sb_append_cstr(sb, ", ");
		}
		first = false;
		sb_append_tipeerde_naam(sb, it);
	}
	nob_sb_append_cstr(sb, ") -> ");
	if (this->benoemde_terugkeerwaarde) {
		sb_append_tipeerde_naam(sb, &this->benoem);
	} else {
		nob_sb_append_cstr(sb, ":");
		sb_append_tipe(sb, &this->onbenoem);
	}
	nob_sb_append_cstr(sb, " = ");
	sb_append_expr(sb, this->lyf);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_funk)

DESTROY_METH(et_roep) {
	assert(this != NULL);

	token_destroy(&this->funksie);
	nob_da_foreach(struct expr, it, &this->argumente) {
		expr_destroy(it);
	}
	nob_da_free(this->argumente);
}
COPY_METH(et_roep) {
	assert(this != NULL);

	struct et_roep res = {0};
	res.funksie = token_copy(&this->funksie);
	nob_da_foreach(struct expr, it, &this->argumente) {
		nob_da_append(&res.argumente, expr_copy(it));
	}

	return res;
}
SB_APPEND_FUNC(et_roep) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "ROEP(%.*s", (int)this->funksie.len, &this->funksie.pos.source[this->funksie.pos.idx]);
	nob_da_foreach(struct expr, it, &this->argumente) {
		nob_sb_append_cstr(sb, ", ");
		sb_append_expr(sb, it);
	}
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_roep)

DESTROY_METH(et_stel_veranderlike) {
	assert(this != NULL);

	token_destroy(&this->veranderlike);
	expr_destroy(this->na);
	free(this->na);
}
COPY_METH(et_stel_veranderlike) {
	assert(this != NULL);

	struct et_stel_veranderlike res = {0};
	res.veranderlike = token_copy(&this->veranderlike);
	res.na = malloc(sizeof(*res.na));
	*res.na = expr_copy(this->na);

	return res;
}
SB_APPEND_FUNC(et_stel_veranderlike) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "STEL(%.*s = ", (int)this->veranderlike.len, &this->veranderlike.pos.source[this->veranderlike.pos.idx]);
	sb_append_expr(sb, this->na);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_stel_veranderlike)

DESTROY_METH(et_tweevoud_operasie) {
	assert(this != NULL);

	token_destroy(&this->operasie);
	expr_destroy(this->links);
	free(this->links);
	expr_destroy(this->regs);
	free(this->regs);
}
COPY_METH(et_tweevoud_operasie) {
	assert(this != NULL);

	struct et_tweevoud_operasie res = {0};
	res.operasie = token_copy(&this->operasie);
	res.links = malloc(sizeof(*res.links));
	*res.links = expr_copy(this->links);
	res.regs = malloc(sizeof(*res.regs));
	*res.regs = expr_copy(this->regs);

	return res;
}
SB_APPEND_FUNC(et_tweevoud_operasie) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "OP(%.*s, ", (int)this->operasie.len, &this->operasie.pos.source[this->operasie.pos.idx]);
	sb_append_expr(sb, this->links);
	nob_sb_append_cstr(sb, ", ");
	sb_append_expr(sb, this->regs);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_tweevoud_operasie)

DESTROY_METH(et_eenvoud_operasie) {
	assert(this != NULL);

	token_destroy(&this->operasie);
	expr_destroy(this->invoer);
	free(this->invoer);
}
COPY_METH(et_eenvoud_operasie) {
	assert(this != NULL);

	struct et_eenvoud_operasie res = {0};
	res.operasie = token_copy(&this->operasie);
	res.invoer = malloc(sizeof(*res.invoer));
	*res.invoer = expr_copy(this->invoer);

	return res;
}
SB_APPEND_FUNC(et_eenvoud_operasie) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "OP(%.*s, ", (int)this->operasie.len, &this->operasie.pos.source[this->operasie.pos.idx]);
	sb_append_expr(sb, this->invoer);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_eenvoud_operasie)

DESTROY_METH(et_konstante) {
	assert(this != NULL);

	token_destroy(&this->konstante);
}
COPY_METH(et_konstante) {
	assert(this != NULL);

	struct et_konstante res = {0};
	res.konstante = token_copy(&this->konstante);

	return res;
}
SB_APPEND_FUNC(et_konstante) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "KONSTANTE(%.*s)", (int)this->konstante.len, &this->konstante.pos.source[this->konstante.pos.idx]);
}
PRINT_IMPL(et_konstante)

DESTROY_METH(expr) {
	assert(this != NULL);

	switch (this->type) {
		case ET_BLOK: et_blok_destroy(&this->blok); break;
		case ET_FUNK: et_funk_destroy(&this->funk); break;
		case ET_ROEP: et_roep_destroy(&this->roep); break;
		case ET_STEL_VERANDERLIKE: et_stel_veranderlike_destroy(&this->stel_veranderlike); break;
		case ET_TWEEVOUD_OPERASIE: et_tweevoud_operasie_destroy(&this->tweevoud_operasie); break;
		case ET_EENVOUD_OPERASIE: et_eenvoud_operasie_destroy(&this->eenvoud_operasie); break;
		case ET_KONSTANTE: et_konstante_destroy(&this->konstante); break;
	}
}
COPY_METH(expr) {
	assert(this != NULL);

	struct expr res = {0};
	res.type = this->type;
	switch (this->type) {
		case ET_BLOK: res.blok = et_blok_copy(&this->blok); break;
		case ET_FUNK: res.funk = et_funk_copy(&this->funk); break;
		case ET_ROEP: res.roep = et_roep_copy(&this->roep); break;
		case ET_STEL_VERANDERLIKE: res.stel_veranderlike = et_stel_veranderlike_copy(&this->stel_veranderlike); break;
		case ET_TWEEVOUD_OPERASIE: res.tweevoud_operasie = et_tweevoud_operasie_copy(&this->tweevoud_operasie); break;
		case ET_EENVOUD_OPERASIE: res.eenvoud_operasie = et_eenvoud_operasie_copy(&this->eenvoud_operasie); break;
		case ET_KONSTANTE: res.konstante = et_konstante_copy(&this->konstante); break;
	}
	return res;
}
SB_APPEND_FUNC(expr) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "EXPR: ");
	switch (this->type) {
		case ET_BLOK: sb_append_et_blok(sb, &this->blok); break;
		case ET_FUNK: sb_append_et_funk(sb, &this->funk); break;
		case ET_ROEP: sb_append_et_roep(sb, &this->roep); break;
		case ET_STEL_VERANDERLIKE: sb_append_et_stel_veranderlike(sb, &this->stel_veranderlike); break;
		case ET_TWEEVOUD_OPERASIE: sb_append_et_tweevoud_operasie(sb, &this->tweevoud_operasie); break;
		case ET_EENVOUD_OPERASIE: sb_append_et_eenvoud_operasie(sb, &this->eenvoud_operasie); break;
		case ET_KONSTANTE: sb_append_et_konstante(sb, &this->konstante); break;
	}
}
PRINT_IMPL(expr)

static struct {
	size_t n_errors;
	struct source_tracking state;
	struct token curr;
	bool has_next;
	struct token next;
} state;

static void advance(void) {
	if (state.has_next) {
		token_destroy(&state.curr);
		state.curr = state.next;
		state.has_next = false;
	} else if (state.curr.type == TOK_EOF) {
	} else {
		token_destroy(&state.curr);
		state.curr = lex_token(&state.state);
	}
}
static struct token *peek(void) {
	if (state.has_next) return &state.next;
	if (state.curr.type == TOK_EOF) return &state.curr;
	state.has_next = true;
	state.next = lex_token(&state.state);
	return &state.next;
}

void fprint_current(const char *prefix, FILE *file) {
	fputs(prefix, file);
	fprintf(file, "%s:%lu,%lu:\n", state.curr.pos.filename, state.curr.pos.line, state.curr.pos.idx - state.curr.pos.line_start + 1);
	const char *line = &state.curr.pos.source[state.curr.pos.line_start];
	size_t line_len;
	for (line_len = 0; state.curr.pos.idx + line_len < state.curr.pos.size; ++line_len) {
		if (line[line_len] == '\n') break;
	}
	fputs(prefix, file);
	fputc(' ', file);
	fprintf(file, "%.*s\n", (int)line_len, line);
	fputs(prefix, file);
	fputc(' ', file);
	for (size_t i = 0; i < state.curr.pos.idx - state.curr.pos.line_start; ++i) {
		fputc(' ', file);
	}
	for (size_t i = 0; i < state.curr.len; ++i) {
		fputc('^', file);
	}
	fputc('\n', file);
}
#define parse_error(short, msg, ...) do { \
		++state.n_errors; \
		fprintf(stderr, "FOUT: %s\n", short); \
		fprint_current("      ", stderr); \
		fputs("      ", stderr); \
		fprintf(stderr, msg, __VA_ARGS__); \
		fputc('\n', stderr); \
	} while (0)
void recover_error(void) {
	while (state.curr.type != TOK_EOF && state.curr.type != SYM_SEMICOLON) {
		advance();
	}
	if (state.curr.type == SYM_SEMICOLON) {
		advance();
	}
}

bool parse_expr(struct expr *res);
bool parse_insluiting(struct st_insluiting *res) {
	assert(state.curr.type == SYM_AT);
	advance();

	if (state.curr.type != IDENTIFIER || state.curr.len != strlen("sluit_in") || strncmp(&state.curr.pos.source[state.curr.pos.idx], "sluit_in", state.curr.len)) {
		parse_error(
			"het \"sluit_in\" verwag",
			"verwag die woord \"sluit_in\" ná 'n @ simbool", NULL
		);
		recover_error();
		return false;
	} else advance();

	if (state.curr.type != IDENTIFIER) {
		parse_error(
			"het modulenaam verwag",
			"verwag 'n naam van 'n module om in te sluit", NULL
		);
		recover_error();
		return false;
	}
	res->module = token_copy(&state.curr);
	advance();

	return true;
}
bool parse_funksie_definisie(struct st_funksie *res) {
	assert(state.curr.type == KW_FUNK);
	advance();

	if (state.curr.type != IDENTIFIER) {
		parse_error(
			"het funksie naam verwag",
			"verwag in 'n funksie definisie dat die naam van die funksie na die \"funk\" sleutelwoord kom", NULL
		);
		recover_error();
		return false;
	}
	res->naam = token_copy(&state.curr);
	advance();

	if (state.curr.type != SYM_LPAREN) {
		token_destroy(&res->naam);

		parse_error(
			"het linker hakie verwag",
			"verwag na 'n funksie se naam die funksie se argumente, wat met 'n linker hakie begin", NULL
		);
		recover_error();
		return false;
	} else advance();

	if (state.curr.type != SYM_RPAREN) {
		token_destroy(&res->naam);

		parse_error(
			"het regter hakie verwag",
			"verwag dat die argumente van 'n funksie met 'n regter hakie moet eindig", NULL
		);
		recover_error();
		return false;
	} else advance();

	res->lyf = calloc(1, sizeof(*res->lyf));
	if (!parse_expr(res->lyf)) {
		free(res->lyf);
		token_destroy(&res->naam);
		return false;
	}

	return true;
}

bool parse_roep(struct et_roep *res) {
	assert(state.curr.type == IDENTIFIER && peek()->type == SYM_LPAREN);
	res->funksie = token_copy(&state.curr);
	advance();
	advance();

	// TODO: proper parsing here
	fputs("WARNING: skipping over function call argument because I can't be arsed to parse it right now\n", stderr);
	advance(); // skip argument

	if (state.curr.type != SYM_RPAREN) {
		token_destroy(&res->funksie);

		parse_error(
			"het regter hakie verwag",
			"verwag dat die argumente van 'n funksie met 'n regter hakie moet eindig", NULL
		);
		recover_error();
		return false;
	} else advance();

	return true;
}
bool parse_blok(struct et_blok *res) {
	assert(state.curr.type == SYM_LBRACE);
	advance();

	while (state.curr.type != SYM_RBRACE && state.curr.type != TOK_EOF) {
		struct expr expr;
		if (!parse_expr(&expr)) continue;
		nob_da_append(res, expr);
		if (state.curr.type != SYM_SEMICOLON) {
			parse_error(
				"het 'n kommapunt verwag",
				"ná elke uitdrukking in 'n blok moet daar 'n kommapunt wees", NULL
			);
			recover_error();
		} else advance();
	}

	if (state.curr.type == TOK_EOF) {
		parse_error(
			"vroeë einde van lêer",
			"einde van lêer bevind terwyl besig om blok te parse", NULL
		);
		return false;
	} else advance();

	return true;
}

bool parse_expr(struct expr *res) {
	switch (state.curr.type) {
		case IDENTIFIER: {
			struct token *next = peek();
			if (next->type == SYM_LPAREN) {
				res->type = ET_ROEP;
				return parse_roep(&res->roep);
			} else {
				assert(false && "TODO");
			}
		} break;
		case SYM_LBRACE: {
			res->type = ET_BLOK;
			return parse_blok(&res->blok);
		} break;
		default: {
			return false;
		} break;
	}
	assert(false && "This should be unreachable");
	return false;
}

struct program parse_file(struct source_tracking source) {
	memset(&state, 0, sizeof(state));
	state.state = source;
	state.curr = lex_token(&state.state);

	struct program res = {0};

	while (state.curr.type != TOK_EOF) {
		struct statement stmt = {0};
		switch (state.curr.type) {
			case SYM_AT: {
				stmt.type = ST_INSLUITING;
				if (parse_insluiting(&stmt.insluiting)) {
					nob_da_append(&res, stmt);
				} else {
					goto continue_while_loop;
				}
			} break;
			case KW_LAAT: {
				parse_error(
					"nog nie geïmplimenteer nie",
					"delkarasies en definisies is nog nie geïmplimenteer nie", NULL
				);
				recover_error();
				goto continue_while_loop;
			} break;
			case KW_FUNK: {
				stmt.type = ST_FUNKSIE;
				if (parse_funksie_definisie(&stmt.funksie)) {
					nob_da_append(&res, stmt);
				} else {
					goto continue_while_loop;
				}
			} break;
			default: {
				parse_error(
					"het stelling verwag",
					"verwag \"funk\", \"laat\", of \"@\", het \"%.*s\" gekry",
					(int)state.curr.len, &state.curr.pos.source[state.curr.pos.idx]
				);
				recover_error();
				goto continue_while_loop;
			} break;
		}
		if (state.curr.type != SYM_SEMICOLON) {
			parse_error(
				"het 'n kommapunt verwag",
				"ná elke insluiting, deklarasie, definisie, of funksiedefinisie moet daar 'n kommapunt wees", NULL
			);
		} else advance();
continue_while_loop:;
	}

	return res;
}
bool parser_had_error(void) {
	return state.n_errors != 0;
}
