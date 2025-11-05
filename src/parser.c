#include "parser.h"

#include <unistd.h>

#include "nob.h"

#include "lexer.h"
#include "types.h"
#include "utils.h"

DESTROY_METH(tipeerde_naam) {
	bt_assert(this != NULL);

	token_destroy(&this->naam);
	konkrete_tipe_destroy(&this->tipe);
}
COPY_METH(tipeerde_naam) {
	bt_assert(this != NULL);

	struct tipeerde_naam res = {0};
	res.naam = token_copy(&this->naam);
	res.tipe = konkrete_tipe_copy(&this->tipe);

	return res;
}
SB_APPEND_FUNC(tipeerde_naam) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(
		sb, "NAAM("SRC_POS_FMT": ",
		SRC_POS_FARGS(this->naam.pos)
	);
	sb_append_konkrete_tipe(sb, &this->tipe);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(tipeerde_naam)

DESTROY_METH(st_insluiting) {
	bt_assert(this != NULL);

	token_destroy(&this->module);
}
COPY_METH(st_insluiting) {
	bt_assert(this != NULL);

	struct st_insluiting res = {0};
	res.module = token_copy(&this->module);

	return res;
}
SB_APPEND_FUNC(st_insluiting) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(
		sb, "SLUIT_IN("SRC_POS_FMT")",
		SRC_POS_FARGS(this->module.pos)
	);
}
PRINT_IMPL(st_insluiting)

DESTROY_METH(st_deklarasie) {
	bt_assert(this != NULL);

	tipeerde_naam_destroy(&this->veranderlike);
}
COPY_METH(st_deklarasie) {
	bt_assert(this != NULL);

	struct st_deklarasie res = {0};
	res.veranderlike = tipeerde_naam_copy(&this->veranderlike);

	return res;
}
SB_APPEND_FUNC(st_deklarasie) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "DEKL(");
	sb_append_tipeerde_naam(sb, &this->veranderlike);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(st_deklarasie)

DESTROY_METH(st_definisie) {
	bt_assert(this != NULL);

	tipeerde_naam_destroy(&this->veranderlike);
	expr_destroy(this->wat);
	free(this->wat);
}
COPY_METH(st_definisie) {
	bt_assert(this != NULL);

	struct st_definisie res = {0};
	res.veranderlike = tipeerde_naam_copy(&this->veranderlike);
	res.wat = malloc(sizeof(*res.wat));
	*res.wat = expr_copy(this->wat);

	return res;
}
SB_APPEND_FUNC(st_definisie) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "DEF(");
	sb_append_tipeerde_naam(sb, &this->veranderlike);
	nob_sb_append_cstr(sb, " = ");
	sb_append_expr(sb, this->wat);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(st_definisie)

DESTROY_METH(st_funksie) {
	bt_assert(this != NULL);

	token_destroy(&this->naam);
	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		tipeerde_naam_destroy(it);
	}
	nob_da_free(this->argumente);
	if (this->benoemde_terugkeerwaarde) {
		tipeerde_naam_destroy(&this->benoem);
	} else {
		konkrete_tipe_destroy(&this->onbenoem);
	}
	expr_destroy(this->lyf);
	free(this->lyf);
}
COPY_METH(st_funksie) {
	bt_assert(this != NULL);

	struct st_funksie res = {0};
	res.naam = token_copy(&this->naam);
	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		nob_da_append(&res.argumente, tipeerde_naam_copy(it));
	}
	res.benoemde_terugkeerwaarde = this->benoemde_terugkeerwaarde;
	if (this->benoemde_terugkeerwaarde) {
		res.benoem = tipeerde_naam_copy(&this->benoem);
	} else {
		res.onbenoem = konkrete_tipe_copy(&this->onbenoem);
	}
	res.lyf = malloc(sizeof(*this->lyf));
	*res.lyf = expr_copy(this->lyf);

	return res;
}
SB_APPEND_FUNC(st_funksie) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(
		sb, "FUNK("SRC_POS_FMT", (",
		SRC_POS_FARGS(this->naam.pos)
	);
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
		sb_append_konkrete_tipe(sb, &this->onbenoem);
	}
	nob_sb_append_cstr(sb, " = ");
	sb_append_expr(sb, this->lyf);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(st_funksie)

DESTROY_METH(statement) {
	bt_assert(this != NULL);

	switch (this->type) {
		case ST_INSLUITING: st_insluiting_destroy(&this->insluiting); break;
		case ST_DEKLARASIE: st_deklarasie_destroy(&this->deklarasie); break;
		case ST_DEFINISIE: st_definisie_destroy(&this->definisie); break;
		case ST_FUNKSIE: st_funksie_destroy(&this->funksie); break;
	}
}
COPY_METH(statement) {
	bt_assert(this != NULL);

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
	bt_assert(sb != NULL && this != NULL);

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
	bt_assert(this != NULL);

	nob_da_foreach(struct statement, it, this) {
		statement_destroy(it);
	}
	nob_da_free(*this);
}
COPY_METH(program) {
	bt_assert(this != NULL);

	struct program res = {0};
	nob_da_foreach(struct statement, it, this) {
		nob_da_append(&res, statement_copy(it));
	}

	return res;
}
SB_APPEND_FUNC(program) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "PROGRAM WITH %lu STATEMENTS:\n", this->count);
	nob_da_foreach(struct statement, it, this) {
		nob_sb_append_cstr(sb, " - ");
		sb_append_statement(sb, it);
		nob_sb_append_cstr(sb, "\n");
	}
}
PRINT_IMPL(program)

DESTROY_METH(et_blok) {
	bt_assert(this != NULL);

	nob_da_foreach(struct expr, it, this) {
		expr_destroy(it);
	}
	nob_da_free(*this);

	if (this->res) {
		expr_destroy(this->res);
		free(this->res);
	}
}
COPY_METH(et_blok) {
	bt_assert(this != NULL);

	struct et_blok res = {0};
	nob_da_foreach(struct expr, it, this) {
		nob_da_append(&res, expr_copy(it));
	}

	if (this->res) {
		res.res = malloc(sizeof(*res.res));
		*res.res = expr_copy(this->res);
	}

	return res;
}
SB_APPEND_FUNC(et_blok) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "BLOK { ");
	nob_da_foreach(struct expr, it, this) {
		sb_append_expr(sb, it);
		nob_sb_append_cstr(sb, "; ");
	}
	if (this->res) {
		sb_append_expr(sb, this->res);
		nob_sb_append_cstr(sb, " ");
	}
	nob_sb_append_cstr(sb, "}");
}
PRINT_IMPL(et_blok)

DESTROY_METH(et_funk) {
	bt_assert(this != NULL);

	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		tipeerde_naam_destroy(it);
	}
	nob_da_free(this->argumente);
	if (this->benoemde_terugkeerwaarde) {
		tipeerde_naam_destroy(&this->benoem);
	} else {
		konkrete_tipe_destroy(&this->onbenoem);
	}
	expr_destroy(this->lyf);
	free(this->lyf);
}
COPY_METH(et_funk) {
	bt_assert(this != NULL);

	struct et_funk res = {0};
	nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
		nob_da_append(&res.argumente, tipeerde_naam_copy(it));
	}
	res.benoemde_terugkeerwaarde = this->benoemde_terugkeerwaarde;
	if (this->benoemde_terugkeerwaarde) {
		res.benoem = tipeerde_naam_copy(&this->benoem);
	} else {
		res.onbenoem = konkrete_tipe_copy(&this->onbenoem);
	}
	res.lyf = malloc(sizeof(*this->lyf));
	*res.lyf = expr_copy(this->lyf);

	return res;
}
SB_APPEND_FUNC(et_funk) {
	bt_assert(sb != NULL && this != NULL);

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
		sb_append_konkrete_tipe(sb, &this->onbenoem);
	}
	nob_sb_append_cstr(sb, " = ");
	sb_append_expr(sb, this->lyf);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_funk)

DESTROY_METH(et_roep) {
	bt_assert(this != NULL);

	token_destroy(&this->funksie);
	nob_da_foreach(struct expr, it, &this->argumente) {
		expr_destroy(it);
	}
	nob_da_free(this->argumente);
}
COPY_METH(et_roep) {
	bt_assert(this != NULL);

	struct et_roep res = {0};
	res.funksie = token_copy(&this->funksie);
	nob_da_foreach(struct expr, it, &this->argumente) {
		nob_da_append(&res.argumente, expr_copy(it));
	}

	return res;
}
SB_APPEND_FUNC(et_roep) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(
		sb, "ROEP("SRC_POS_FMT,
		SRC_POS_FARGS(this->funksie.pos)
	);
	nob_da_foreach(struct expr, it, &this->argumente) {
		nob_sb_append_cstr(sb, ", ");
		sb_append_expr(sb, it);
	}
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_roep)

DESTROY_METH(et_stel_veranderlike) {
	bt_assert(this != NULL);

	token_destroy(&this->veranderlike);
	expr_destroy(this->na);
	free(this->na);
}
COPY_METH(et_stel_veranderlike) {
	bt_assert(this != NULL);

	struct et_stel_veranderlike res = {0};
	res.veranderlike = token_copy(&this->veranderlike);
	res.na = malloc(sizeof(*res.na));
	*res.na = expr_copy(this->na);

	return res;
}
SB_APPEND_FUNC(et_stel_veranderlike) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(
		sb, "STEL("SRC_POS_FMT" = ",
		SRC_POS_FARGS(this->veranderlike.pos)
	);
	sb_append_expr(sb, this->na);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_stel_veranderlike)

DESTROY_METH(et_tweevoud_operasie) {
	bt_assert(this != NULL);

	token_destroy(&this->operasie);
	expr_destroy(this->links);
	free(this->links);
	expr_destroy(this->regs);
	free(this->regs);
}
COPY_METH(et_tweevoud_operasie) {
	bt_assert(this != NULL);

	struct et_tweevoud_operasie res = {0};
	res.operasie = token_copy(&this->operasie);
	res.links = malloc(sizeof(*res.links));
	*res.links = expr_copy(this->links);
	res.regs = malloc(sizeof(*res.regs));
	*res.regs = expr_copy(this->regs);

	return res;
}
SB_APPEND_FUNC(et_tweevoud_operasie) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(
		sb, "OP("SRC_POS_FMT", ",
		SRC_POS_FARGS(this->operasie.pos)
	);
	sb_append_expr(sb, this->links);
	nob_sb_append_cstr(sb, ", ");
	sb_append_expr(sb, this->regs);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_tweevoud_operasie)

DESTROY_METH(et_eenvoud_operasie) {
	bt_assert(this != NULL);

	token_destroy(&this->operasie);
	expr_destroy(this->invoer);
	free(this->invoer);
}
COPY_METH(et_eenvoud_operasie) {
	bt_assert(this != NULL);

	struct et_eenvoud_operasie res = {0};
	res.operasie = token_copy(&this->operasie);
	res.invoer = malloc(sizeof(*res.invoer));
	*res.invoer = expr_copy(this->invoer);

	return res;
}
SB_APPEND_FUNC(et_eenvoud_operasie) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(
		sb, "OP("SRC_POS_FMT", ",
		SRC_POS_FARGS(this->operasie.pos)
	);
	sb_append_expr(sb, this->invoer);
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(et_eenvoud_operasie)

DESTROY_METH(et_konstante) {
	bt_assert(this != NULL);

	token_destroy(&this->konstante);
}
COPY_METH(et_konstante) {
	bt_assert(this != NULL);

	struct et_konstante res = {0};
	res.konstante = token_copy(&this->konstante);

	return res;
}
SB_APPEND_FUNC(et_konstante) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(
		sb, "KONSTANTE("SRC_POS_FMT")",
		SRC_POS_FARGS(this->konstante.pos)
	);
}
PRINT_IMPL(et_konstante)

DESTROY_METH(expr) {
	bt_assert(this != NULL);

	switch (this->type) {
		case ET_VERANDERLIKE: token_destroy(&this->veranderlike); break;
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
	bt_assert(this != NULL);

	struct expr res = {0};
	res.type = this->type;
	switch (this->type) {
		case ET_VERANDERLIKE: res.veranderlike = token_copy(&this->veranderlike); break;
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
	bt_assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "EXPR: ");
	switch (this->type) {
		case ET_VERANDERLIKE: {
			nob_sb_appendf(
				sb, "VERANDERLIKE("SRC_POS_FMT")",
				SRC_POS_FARGS(this->veranderlike.pos)
			);
		} break;
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

#define ERRORS_LIMIT 25

static struct {
	size_t n_errors;
	struct source_tracking state;
	struct token curr;
	bool has_next;
	struct token next;
} state;

static inline void advance(void) {
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
static inline struct token *peek(void) {
	if (state.has_next) return &state.next;
	if (state.curr.type == TOK_EOF) return &state.curr;
	state.has_next = true;
	state.next = lex_token(&state.state);
	return &state.next;
}

static inline void fprint_current(const char *prefix, FILE *file) {
	const struct src_pos *here = &state.curr.pos;
	const char *line = src_pos_getline(here);
	const size_t start_idx_in_line = here->idx - here->line_start;
	const size_t line_len = src_pos_comp_linelen(here);

	// print current location: file, line, column
	fputs(prefix, file);
	fprintf(file, SRC_POS_LOC_FMT":\n", SRC_POS_LOC_FARGS(*here));

	// print the line itself, rendering a tab as four spaces
	fputs(prefix, file);
	fputc(' ', file);
	for (size_t i = 0; i < line_len; ++i) {
		if (line[i] == '\t') {
			for (size_t j = 0; j < 4; ++j) fputc(' ', file);
		} else fputc(line[i], file);
	}
	fputc('\n', file);

	// print the pointer (via carats) to the current token
	fputs(prefix, file);
	fputc(' ', file);
	for (size_t i = 0; i < start_idx_in_line; ++i) {
		if (line[i] == '\t') { // render tabs as four spaces
			for (size_t j = 0; j < 4; ++j) fputc(' ', file);
		} else fputc(' ', file);
	}
	for (size_t i = 0; i < state.curr.pos.len; ++i) {
		fputc('^', file);
	}
	fputc('\n', file);
}
#define parse_error_fmt(short, msg, ...) do { \
		++state.n_errors; \
		fprintf(stderr, "FOUT: %s\n", short); \
		fprint_current("      ", stderr); \
		fputs("      ", stderr); \
		fprintf(stderr, msg, __VA_ARGS__); \
		fputc('\n', stderr); \
	} while (0)
#define parse_error(short, msg) do { \
		++state.n_errors; \
		fprintf(stderr, "FOUT: %s\n", short); \
		fprint_current("      ", stderr); \
		fputs("      ", stderr); \
		fputs(msg, stderr); \
		fputc('\n', stderr); \
	} while (0)
#define fail_if_over_limit(fail) do { \
		if (state.n_errors >= ERRORS_LIMIT) goto fail; \
	} while (0)
static inline void recover_error(void) {
	while (state.curr.type != TOK_EOF && state.curr.type != SYM_SEMICOLON) {
		advance();
	}
	if (state.curr.type == SYM_SEMICOLON) {
		advance();
	}
}

static bool parse_expr(struct expr *res);
static bool parse_tipe(struct konkrete_tipe *res, enum veranderlikheid outo_ver);
static bool parse_insluiting(struct st_insluiting *res) {
	bt_assert(state.curr.type == SYM_AT);
	advance();

	if (state.curr.type != IDENTIFIER || src_pos_strcmp(&state.curr.pos, "sluit_in") != 0) {
		parse_error(
			"het \"sluit_in\" verwag",
			"verwag die woord \"sluit_in\" ná 'n @ simbool"
		);
		recover_error();
		return false;
	} else advance();

	if (state.curr.type != IDENTIFIER) {
		parse_error(
			"het modulenaam verwag",
			"verwag 'n naam van 'n module om in te sluit"
		);
		recover_error();
		return false;
	}
	res->module = token_copy(&state.curr);
	advance();

	return true;
}
static bool parse_laat(struct statement *res) {
	bt_assert(state.curr.type == KW_LAAT);
	advance();

	if (state.curr.type == KW_EKSTERN) {
		parse_error(
			"nog nie geïmplimenteer nie",
			"externe veranderlikes is nog nie geïmplimenteer nie"
		);
		--state.n_errors;
		advance();
	}

	if (state.curr.type != IDENTIFIER) {
		parse_error(
			"het veranderlike naam verwag",
			"verwag in 'n laat stelling dat die naam van die veranderlike na die \"laat\" sleutelwoord kom"
		);
		recover_error();
		goto fail;
	}
	struct token naam = token_copy(&state.curr);
	advance();

	if (state.curr.type != SYM_COLON) {
		parse_error(
			"het dubbelpunt verwag",
			"verwag 'n dubbelpunt na 'n veranderlike se naam in 'n laat stelling"
		);
		recover_error();
		goto fail_cleanup_naam;
	} else advance();

	struct konkrete_tipe tipe = {0};
	if (!parse_tipe(&tipe, v_konstant)) {
		goto fail_cleanup_naam;
	}

	if (state.curr.type == SYM_SEMICOLON) {
		res->type = ST_DEKLARASIE;
		res->deklarasie.veranderlike.naam = naam;
		res->deklarasie.veranderlike.tipe = tipe;
		return true;
	}

	if (state.curr.type != SYM_EQUAL) {
		parse_error(
			"het 'n kommapunt of gelykaan verwag",
			"verwag na 'n deklarasie óf 'n kommapunt (vir net 'n deklarasie) óf 'n gelykaan simbool gevolg deur 'n uitdrukking (vir 'n definisie)"
		);
		recover_error();
		goto fail_cleanup_tipe;
	} else advance();

	struct expr *wat = calloc(1, sizeof(*wat));
	if (!parse_expr(wat)) {
		free(wat);
		goto fail_cleanup_tipe;
	}

	res->type = ST_DEFINISIE;
	res->definisie.veranderlike.naam = naam;
	res->definisie.veranderlike.tipe = tipe;
	res->definisie.wat = wat;
	return true;

fail_cleanup_tipe:
	konkrete_tipe_destroy(&tipe);
fail_cleanup_naam:
	token_destroy(&naam);
fail:
	return false;
}
static bool parse_funksie_definisie(struct st_funksie *res) {
	bt_assert(state.curr.type == KW_FUNK);
	advance();

	if (state.curr.type != IDENTIFIER) {
		parse_error(
			"het funksie naam verwag",
			"verwag in 'n funksie definisie dat die naam van die funksie na die \"funk\" sleutelwoord kom"
		);
		recover_error();
		goto fail;
	}
	res->naam = token_copy(&state.curr);
	advance();

	if (state.curr.type != SYM_LPAREN) {
		parse_error(
			"het linker hakie verwag",
			"verwag na 'n funksie se naam die funksie se argumente, wat met 'n linker hakie begin"
		);
		recover_error();
		goto fail_cleanup_naam;
	} else advance();

	bool first = true;
	while (state.curr.type != SYM_RPAREN && state.curr.type != TOK_EOF) {
		if (!first) {
			if (state.curr.type != SYM_COMMA) {
				parse_error(
					"het 'n komma verwag",
					"tussen twee funksie argumente moet daar 'n komma wees"
				);
				fail_if_over_limit(fail_cleanup_argumente);
			} else advance();
		}
		first = false;

		struct tipeerde_naam argument = {0};
		if (state.curr.type != IDENTIFIER) {
			parse_error(
				"het 'n naam verwag",
				"verwag 'n argumentnaam vir elke argument in 'n funksie definisie"
			);
			recover_error();
			goto fail_cleanup_argumente;
		}
		argument.naam = token_copy(&state.curr);
		advance();

		if (state.curr.type != SYM_COLON) {
			parse_error(
				"het 'n dubbelpunt verwag",
				"verwag 'n dubbelpunt tussen elke funksie argument en sy tipe"
			);
			recover_error();
			goto fail_cleanup_argumentnaam;
		} else advance();

		if (!parse_tipe(&argument.tipe, v_konstant)) {
			goto fail_cleanup_argumentnaam;
		}
		nob_da_append(&res->argumente, argument);

		if (false) { // WARN: gcc *might* try to optimise this out? That could cause some issues...
		fail_cleanup_argumentnaam:
			token_destroy(&argument.naam);
			goto fail_cleanup_argumente;
		}
	}

	if (state.curr.type != SYM_RPAREN) {
		parse_error(
			"het regter hakie verwag",
			"verwag dat die argumente van 'n funksie met 'n regter hakie moet eindig"
		);
		recover_error();
		goto fail_cleanup_argumente;
	} else advance();

	if (state.curr.type == SYM_ARROW) {
		advance();

		if (state.curr.type == IDENTIFIER) {
			res->benoemde_terugkeerwaarde = true;
			res->benoem.naam = token_copy(&state.curr);
			advance();
		}

		if (state.curr.type != SYM_COLON) {
			parse_error(
				"het 'n dubbelpunt verwag",
				res->benoemde_terugkeerwaarde
				? "verwag 'n dubbelpunt tussen die terugkeernaam en terugkeertipe van 'n funksie"
				: "verwag 'n dubbelpunt voor die terugkeertipe van 'n funksie"
			);
			recover_error();
			goto fail_cleanup_terugkeerwaarde_naam;
		} else advance();
		if (!parse_tipe(res->benoemde_terugkeerwaarde ? &res->benoem.tipe : &res->onbenoem, v_veranderlik)) {
			return false;
			goto fail_cleanup_terugkeerwaarde_naam;
		}

		if (false) { // WARN: gcc *might* try to optimise this out? That could cause some issues...
		fail_cleanup_terugkeerwaarde_naam:
			if (res->benoemde_terugkeerwaarde) token_destroy(&res->benoem.naam);
			goto fail_cleanup_argumente;
		}
	} else {
		res->benoemde_terugkeerwaarde = false;
		res->onbenoem.tipe_vlh = v_veranderlik;
		res->onbenoem.tipe.klas = kvt_basies;
		res->onbenoem.tipe.basies = bt_niks;
	}

	if ((res->benoemde_terugkeerwaarde ? res->benoem.tipe.tipe_vlh : res->onbenoem.tipe_vlh) != v_veranderlik) {
		fputs("WAARSKUWING: funksie terugkeerwaarde is nie veranderlik nie\n", stderr);
		fprint_current("    ", stderr);
		fputs("    As die terugkeerwaarde van 'n funksie nie veranderlik is nie kan dit nie gestel word nie, en die funksie kan nie 'n waarde teruggee nie. Hierdie is amper definitief 'n vout\n", stderr);
	}

	res->lyf = calloc(1, sizeof(*res->lyf));
	if (!parse_expr(res->lyf)) {
		free(res->lyf);
		goto fail_cleanup_terugkeerwaarde;
	}

	return true;

fail_cleanup_terugkeerwaarde:
	if (res->benoemde_terugkeerwaarde) {
		tipeerde_naam_destroy(&res->benoem);
	} else {
		konkrete_tipe_destroy(&res->onbenoem);
	}
fail_cleanup_argumente:
	nob_da_foreach(struct tipeerde_naam, it, &res->argumente) {
		tipeerde_naam_destroy(it);
	}
	nob_da_free(res->argumente);
fail_cleanup_naam:
	token_destroy(&res->naam);
fail:
	return false;
}

static bool parse_funksie_tipe(struct tp_funksie *res) {
	bt_assert(state.curr.type == KW_FUNK);
	advance();

	if (state.curr.type == IDENTIFIER && peek()->type == SYM_LPAREN) {
		parse_error(
			"het linker hakie verwag",
			"verwag na die \"funk\" sleutelwoord in 'n funksie tipe die funksie se argumente, wat met 'n linker hakie begin"
		);
		fputs("WENK: dit lyk of jy 'n funksie naam na \"funk\" probeer sit het; dit is net in funksie definisies toegelaat, 'n funksie tipe het nie 'n naam daarmee geassosiëer nie", stderr);
		advance();
		advance();
		fail_if_over_limit(fail);
	} else if (state.curr.type != SYM_LPAREN) {
		parse_error(
			"het linker hakie verwag",
			"verwag na die \"funk\" sleutelwoord in 'n funksie tipe die funksie se argumente, wat met 'n linker hakie begin"
		);
		recover_error();
		goto fail;
	} else advance();

	bool first = true;
	while (state.curr.type != SYM_RPAREN && state.curr.type != TOK_EOF) {
		if (!first) {
			if (state.curr.type != SYM_COMMA) {
				parse_error(
					"het 'n komma verwag",
					"tussen twee funksie tipe argumente moet daar 'n komma wees"
				);
				fail_if_over_limit(fail_cleanup_argumente);
			} else advance();
		}
		first = false;

		if (state.curr.type == IDENTIFIER && peek()->type == SYM_COLON) {
			parse_error(
				"het 'n dubbelpunt verwag",
				"verwag 'n dubbelpunt voor elke argumenttipe in 'n funksie tipe"
			);
			fputs("WENK: dit lyk of jy 'n argument naam probeer spesifiseer het; dit is net in funksie definisies en funksie waardes toegelaat, 'n funksie tipe het nie name met sy argumente geassosiëer nie", stderr);
			advance();
			advance();
			fail_if_over_limit(fail_cleanup_argumente);
		} else if (state.curr.type != SYM_COLON) {
			parse_error(
				"het 'n dubbelpunt verwag",
				"verwag 'n dubbelpunt voor elke argumenttipe in 'n funksie tipe"
			);
			recover_error();
			goto fail_cleanup_argumente;
		} else advance();

		struct konkrete_tipe tipe = {0};
		if (!parse_tipe(&tipe, v_konstant)) goto fail_cleanup_argumente;
		nob_da_append(&res->argumente, tipe);
	}

	if (state.curr.type != SYM_RPAREN) {
		parse_error(
			"het reger hakie verwag",
			"verwag dat die argumente van 'n funksie tipe met 'n regter hakie moet eindig"
		);
		recover_error();
		goto fail_cleanup_argumente;
	} else advance();

	if (state.curr.type != SYM_ARROW) {
		res->terugkeer = malloc(sizeof(*res->terugkeer));
		res->terugkeer->tipe_vlh = v_veranderlik;
		res->terugkeer->tipe.klas = kvt_basies;
		res->terugkeer->tipe.basies = bt_niks;
		return true;
	} else advance();

	if (state.curr.type == IDENTIFIER && peek()->type == SYM_COLON) {
		parse_error(
			"het 'n dubbelpunt verwag",
			"verwag 'n dubbelpunt voor die terugkeertipe van 'n funksie tipe"
		);
		fputs("WENK: dit lyk of jy 'n terugkeernaam probeer spesifiseer het; dit is net in funksie definisies en funksie waardes toegelaat, 'n funksie tipe het nie 'n naam met sy terugkeerwaarde geassosiëer nie", stderr);
		advance();
		advance();
		fail_if_over_limit(fail_cleanup_argumente);
	} else if (state.curr.type != SYM_COLON) {
		parse_error(
			"het 'n dubbelpunt verwag",
			"verwag 'n dubbelpunt voor die terugkeertipe van 'n funksie tipe"
		);
		recover_error();
		goto fail_cleanup_argumente;
	} else advance();

	res->terugkeer = calloc(1, sizeof(*res->terugkeer));
	if (!parse_tipe(res->terugkeer, v_veranderlik)) {
		free(res->terugkeer);
		goto fail_cleanup_argumente;
	}

	if (res->terugkeer->tipe_vlh != v_veranderlik) {
		fputs("WAARSKUWING: funksie terugkeerwaarde is nie veranderlik nie\n", stderr);
		fprint_current("    ", stderr);
		fputs("    As die terugkeerwaarde van 'n funksie nie veranderlik is nie kan dit nie gestel word nie, en die funksie kan nie 'n waarde teruggee nie. Hierdie is amper definitief 'n vout\n", stderr);
	}

	return true;

fail_cleanup_argumente:
	nob_da_foreach(struct konkrete_tipe, it, &res->argumente) {
		konkrete_tipe_destroy(it);
	}
	nob_da_free(res->argumente);
fail:
	return false;
}
static bool parse_tipe(struct konkrete_tipe *res, enum veranderlikheid outo_ver) {
	res->tipe_vlh = outo_ver;
	if (state.curr.type == KW_KON) {
		res->tipe_vlh = v_konstant;
		advance();
	} else if (state.curr.type == KW_VER) {
		res->tipe_vlh = v_veranderlik;
		advance();
	} else if (state.curr.type == KW_ERF) {
		res->tipe_vlh = v_erfbaar;
		advance();
	}
	switch (state.curr.type) {
		case KW_HEEL: {
			res->tipe.klas = kvt_basies;
			res->tipe.basies = bt_heel64;
			advance();
			return true;
		} break;
		case KW_FUNK: {
			res->tipe.klas = kvt_funksie;
			return parse_funksie_tipe(&res->tipe.funksie);
		}
		default: {
			parse_error(
				"het 'n tipe verwag",
				"tans is daar net heel of funksies"
			);
			recover_error();
			return false;
		} break;
	}

	bt_assert(false && "Should be unreachable");
	return false;
}

static bool parse_roep(struct et_roep *res) {
	bt_assert(state.curr.type == IDENTIFIER && peek()->type == SYM_LPAREN);
	res->funksie = token_copy(&state.curr);
	advance();
	advance();

	bool first = true;
	while (state.curr.type != SYM_RPAREN && state.curr.type != TOK_EOF) {
		if (!first) {
			if (state.curr.type != SYM_COMMA) {
				parse_error(
					"het 'n komma verwag",
					"tussen twee funksie argumente moet daar 'n komma wees"
				);
				fail_if_over_limit(fail);
			} else advance();
		}
		first = false;

		struct expr expr = {0};
		if (!parse_expr(&expr)) {
			// recover_error(); // NOTE: hierdie behoort nie nodig te wees nie?
			goto fail;
		}
		nob_da_append(&res->argumente, expr);
	}

	if (state.curr.type != SYM_RPAREN) {
		parse_error(
			"het regter hakie verwag",
			"verwag dat die argumente van 'n funksie met 'n regter hakie moet eindig"
		);
		recover_error();
		goto fail;
	} else advance();

	return true;

fail:
	nob_da_foreach(struct expr, it, &res->argumente) {
		expr_destroy(it);
	}
	nob_da_free(res->argumente);
	token_destroy(&res->funksie);
	return false;
}
static bool parse_blok(struct et_blok *res) {
	bt_assert(state.curr.type == SYM_LBRACE);
	advance();

	while (state.curr.type != SYM_RBRACE && state.curr.type != TOK_EOF) {
		struct expr expr = {0};
		if (!parse_expr(&expr)) continue;
		if (state.curr.type == SYM_RBRACE) {
			// expr at end of block without following semicolon
			res->res = malloc(sizeof(*res->res));
			*res->res = expr;
			break;
		}
		nob_da_append(res, expr);

		if (state.curr.type != SYM_SEMICOLON) {
			nob_da_foreach(struct expr, it, res) {
				expr_destroy(it);
			}
			nob_da_free(*res);

			parse_error(
				"het 'n kommapunt verwag",
				"ná elke uitdrukking in 'n blok moet daar 'n kommapunt wees"
			);
			recover_error();
			return false;
		} else advance();
	}

	if (state.curr.type == TOK_EOF) {
		et_blok_destroy(res);

		parse_error(
			"vroeë einde van lêer",
			"einde van lêer bevind terwyl besig om blok te parse"
		);
		return false;
	} else advance();

	return true;
}
static bool parse_funksie_waarde(struct et_funk *res) {
	bt_assert(state.curr.type == KW_FUNK);
	advance();

	if (state.curr.type == IDENTIFIER && peek()->type == SYM_LPAREN) {
		parse_error(
			"het linker hakie verwag",
			"verwag na die \"funk\" sleutelwoord in 'n funksie waarde die funksie se argumente, wat met 'n linker hakie begin"
		);
		fputs("WENK: dit lyk of jy 'n funksie naam na \"funk\" probeer sit het; dit is net in funksie definisies toegelaat, 'n funksie waarde het nie 'n naam daarmee geassosiëer nie", stderr);
		advance();
		advance();
		fail_if_over_limit(fail);
	} else if (state.curr.type != SYM_LPAREN) {
		parse_error(
			"het linker hakie verwag",
			"verwag na die \"funk\" sleutelwoord in 'n funksie waarde die funksie se argumente, wat met 'n linker hakie begin"
		);
		recover_error();
		goto fail;
	} else advance();

	bool first = true;
	while (state.curr.type != SYM_RPAREN && state.curr.type != TOK_EOF) {
		if (!first) {
			if (state.curr.type != SYM_COMMA) {
				parse_error(
					"het 'n komma verwag",
					"tussen twee funksie argumente moet daar 'n komma wees"
				);
				fail_if_over_limit(fail_cleanup_argumente);
			} else advance();
		}
		first = false;

		struct tipeerde_naam argument = {0};
		if (state.curr.type != IDENTIFIER) {
			parse_error(
				"het 'n naam verwag",
				"verwag 'n argumentnaam vir elke argument in 'n funksie waarde"
			);
			recover_error();
			goto fail_cleanup_argumente;
		}
		argument.naam = token_copy(&state.curr);
		advance();

		if (state.curr.type != SYM_COLON) {
			parse_error(
				"het 'n dubbelpunt verwag",
				"verwag 'n dubbelpunt tussen elke funksie argument en sy tipe"
			);
			recover_error();
			goto fail_cleanup_argumentnaam;
		} else advance();

		if (!parse_tipe(&argument.tipe, v_konstant)) goto fail_cleanup_argumentnaam;
		nob_da_append(&res->argumente, argument);
		if (false) { // WARN: gcc *might* try to optimise this out? That could cause some issues...
		fail_cleanup_argumentnaam:
			token_destroy(&argument.naam);
			goto fail_cleanup_argumente;
		}
	}

	if (state.curr.type != SYM_RPAREN) {
		parse_error(
			"het regter hakie verwag",
			"verwag dat die argumente van 'n funksie met 'n regter hakie moet eindig"
		);
		recover_error();
		goto fail_cleanup_argumente;
	} else advance();

	if (state.curr.type == SYM_ARROW) {
		advance();

		if (state.curr.type == IDENTIFIER) {
			res->benoemde_terugkeerwaarde = true;
			res->benoem.naam = token_copy(&state.curr);
			advance();
		}

		if (state.curr.type != SYM_COLON) {
			parse_error(
				"het 'n dubbelpunt verwag",
				res->benoemde_terugkeerwaarde
				? "verwag 'n dubbelpunt tussen die terugkeernaam en terugkeertipe van 'n funksie"
				: "verwag 'n dubbelpunt voor die terugkeertipe van 'n funksie"
			);
			recover_error();
			goto fail_cleanup_terugkeerwaarde_naam;
		} else advance();
		if (!parse_tipe(res->benoemde_terugkeerwaarde ? &res->benoem.tipe : &res->onbenoem, v_veranderlik)) {
			goto fail_cleanup_terugkeerwaarde_naam;
		}

		if (false) { // WARN: gcc *might* try to optimise this out? That could cause some issues...
		fail_cleanup_terugkeerwaarde_naam:
			if (res->benoemde_terugkeerwaarde) token_destroy(&res->benoem.naam);
			goto fail_cleanup_argumente;
		}
	} else {
		res->benoemde_terugkeerwaarde = false;
		res->onbenoem.tipe_vlh = v_veranderlik;
		res->onbenoem.tipe.klas = kvt_basies;
		res->onbenoem.tipe.basies = bt_niks;
	}

	if ((res->benoemde_terugkeerwaarde ? res->benoem.tipe.tipe_vlh : res->onbenoem.tipe_vlh) != v_veranderlik) {
		fputs("WAARSKUWING: funksie terugkeerwaarde is nie veranderlik nie\n", stderr);
		fprint_current("    ", stderr);
		fputs("    As die terugkeerwaarde van 'n funksie nie veranderlik is nie kan dit nie gestel word nie, en die funksie kan nie 'n waarde teruggee nie. Hierdie is amper definitief 'n vout\n", stderr);
	}

	res->lyf = calloc(1, sizeof(*res->lyf));
	if (!parse_expr(res->lyf)) {
		free(res->lyf);
		goto fail_cleanup_terugkeerwaarde;
	}

	return true;

fail_cleanup_terugkeerwaarde:
	if (res->benoemde_terugkeerwaarde) {
		tipeerde_naam_destroy(&res->benoem);
	} else {
		konkrete_tipe_destroy(&res->onbenoem);
	}
fail_cleanup_argumente:
	nob_da_foreach(struct tipeerde_naam, it, &res->argumente) {
		tipeerde_naam_destroy(it);
	}
	nob_da_free(res->argumente);
fail:
	return false;
}

static bool parse_expr_terminal(struct expr *res) {
	switch (state.curr.type) {
		case IDENTIFIER: {
			struct token *next = peek();
			if (next->type == SYM_LPAREN) {
				res->type = ET_ROEP;
				return parse_roep(&res->roep);
			} else if (next->type == SYM_EQUAL) {
				res->type = ET_STEL_VERANDERLIKE;
				res->stel_veranderlike.veranderlike = token_copy(&state.curr);
				advance();
				advance();
				res->stel_veranderlike.na = calloc(1, sizeof(*res->stel_veranderlike.na));
				if (!parse_expr(res->stel_veranderlike.na)) {
					free(res->stel_veranderlike.na);
					token_destroy(&res->stel_veranderlike.veranderlike);
					return false;
				}
				return true;
			} else {
				res->type = ET_VERANDERLIKE;
				res->veranderlike = token_copy(&state.curr);
				advance();
				return true;
			}
		} break;
		case STRING_LIT:
		case NUMBER_LIT:
		{
			res->type = ET_KONSTANTE;
			res->konstante.konstante = token_copy(&state.curr);
			advance();
			return true;
		} break;
		case SYM_LBRACE: {
			res->type = ET_BLOK;
			return parse_blok(&res->blok);
		} break;
		case KW_FUNK: {
			res->type = ET_FUNK;
			return parse_funksie_waarde(&res->funk);
		} break;
		case SYM_LPAREN: {
			advance();
			if (!parse_expr(res)) return false;
			if (state.curr.type != SYM_RPAREN) {
				parse_error(
					"het regter hakie verwag",
					"verwag dat 'n uitdrukking wat met 'n linker hakie begin met 'n regter hakie eindig"
				);
				recover_error();
				return false;
			}
			return true;
		} break;
		default: {
			parse_error(
				"het 'n uitdrukking verwag",
				"het 'n uitdrukking verwag, uitdrukkings begin met 'n veranderlike se naam, 'n tekskonstante, 'n nommerkonstante, 'n linker brace, of die \"funk\" sleutelwoord"
			);
			recover_error();
			return false;
		} break;
	}
	bt_assert(false && "This should be unreachable");
	return false;
}
static bool parse_expr0(struct expr *res) {
	if (!parse_expr_terminal(res)) return false;

	while (state.curr.type == SYM_STAR) {
		struct expr *links = calloc(1, sizeof(*links));
		struct expr *regs = calloc(1, sizeof(*regs));
		struct expr expr = {
			.type = ET_TWEEVOUD_OPERASIE,
			.tweevoud_operasie = {
				.operasie = token_copy(&state.curr),
				.links = links,
				.regs = regs,
			},
		};
		*links = *res;
		advance();

		if (!parse_expr_terminal(regs)) {
			expr_destroy(expr.tweevoud_operasie.links);
			token_destroy(&expr.tweevoud_operasie.operasie);
			*res = (struct expr) {0};
			return false;
		}
		*res = expr;
	}

	return true;
}
static bool parse_expr1(struct expr *res) {
	if (!parse_expr0(res)) return false;

	while (state.curr.type == SYM_PLUS) {
		struct expr *links = calloc(1, sizeof(*links));
		struct expr *regs = calloc(1, sizeof(*regs));
		struct expr expr = {
			.type = ET_TWEEVOUD_OPERASIE,
			.tweevoud_operasie = {
				.operasie = token_copy(&state.curr),
				.links = links,
				.regs = regs,
			},
		};
		*links = *res;
		advance();

		if (!parse_expr0(regs)) {
			expr_destroy(expr.tweevoud_operasie.links);
			token_destroy(&expr.tweevoud_operasie.operasie);
			*res = (struct expr) {0};
			return false;
		}
		*res = expr;
	}

	return true;
}
static bool parse_expr(struct expr *res) {
	return parse_expr1(res);
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
					fail_if_over_limit(break_while_loop);
					goto continue_while_loop;
				}
			} break;
			case KW_LAAT: {
				if (parse_laat(&stmt)) {
					nob_da_append(&res, stmt);
				} else {
					fail_if_over_limit(break_while_loop);
					goto continue_while_loop;
				}
			} break;
			case KW_FUNK: {
				stmt.type = ST_FUNKSIE;
				if (parse_funksie_definisie(&stmt.funksie)) {
					nob_da_append(&res, stmt);
				} else {
					fail_if_over_limit(break_while_loop);
					goto continue_while_loop;
				}
			} break;
			default: {
				parse_error_fmt(
					"het stelling verwag",
					"verwag \"funk\", \"laat\", of \"@\", het \""SRC_POS_FMT"\" gekry",
					SRC_POS_FARGS(state.curr.pos)
				);
				recover_error();
				fail_if_over_limit(break_while_loop);
				goto continue_while_loop;
			} break;
		}
		if (state.curr.type != SYM_SEMICOLON) {
			parse_error(
				"het 'n kommapunt verwag",
				"ná elke insluiting, deklarasie, definisie, of funksiedefinisie moet daar 'n kommapunt wees"
			);
			fail_if_over_limit(break_while_loop);
		} else advance();
continue_while_loop:;
	}
break_while_loop:;

	return res;
}
bool parser_had_error(void) {
	return state.n_errors != 0;
}
