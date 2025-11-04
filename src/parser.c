#include "parser.h"

#include <assert.h>

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

	nob_sb_append_cstr(sb, "BLOK {");
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

static size_t n_errors;
static struct source_tracking state;
static struct token curr;

struct program parse_file(struct source_tracking source) {
	n_errors = 0;
	state = source;
	curr = lex_token(&state);

	struct program res = {0};

	while (curr.type != TOK_EOF) {
		switch (curr.type) {
			default: goto break_from_parseloop;
		}
	}
break_from_parseloop:;

	return res;
}
bool parser_had_error(void) {
	return n_errors != 0;
}
