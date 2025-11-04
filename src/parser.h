#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "types.h"
#include "utils.h"

struct expr;

struct tipeerde_naam {
	struct token naam;
	struct konkrete_tipe tipe;
};
DECL_STD_METHS(tipeerde_naam);

enum statement_type {
	ST_INSLUITING,
	ST_DEKLARASIE,
	ST_DEFINISIE,
	ST_FUNKSIE,
};
struct st_insluiting {
	struct token module;
};
DECL_STD_METHS(st_insluiting);
struct st_deklarasie {
	struct tipeerde_naam veranderlike;
};
DECL_STD_METHS(st_deklarasie);
struct st_definisie {
	struct tipeerde_naam veranderlike;
	struct expr *wat;
};
DECL_STD_METHS(st_definisie);
struct st_funksie {
	struct token naam;
	struct {
		struct tipeerde_naam *items;
		size_t count;
		size_t capacity;
	} argumente;
	bool benoemde_terugkeerwaarde;
	union {
		struct tipeerde_naam benoem;
		struct konkrete_tipe onbenoem;
	};
	struct expr *lyf;
};
DECL_STD_METHS(st_funksie);
struct statement {
	enum statement_type type;
	union {
		struct st_insluiting insluiting;
		struct st_deklarasie deklarasie;
		struct st_definisie definisie;
		struct st_funksie funksie;
	};
};

struct program {
	struct statement *items;
	size_t count;
	size_t capacity;
};
DECL_STD_METHS(program);

struct program parse_file(struct source_tracking source);
bool parser_had_error(void);

enum expr_type {
	ET_VERANDERLIKE,
	ET_BLOK,
	ET_FUNK,
	ET_ROEP,
	ET_STEL_VERANDERLIKE,
	ET_TWEEVOUD_OPERASIE,
	ET_EENVOUD_OPERASIE,
	ET_KONSTANTE,
};
struct et_blok {
	struct expr *items;
	size_t count;
	size_t capacity;
};
DECL_STD_METHS(et_blok);
struct et_funk {
	struct {
		struct tipeerde_naam *items;
		size_t count;
		size_t capacity;
	} argumente;
	bool benoemde_terugkeerwaarde;
	union {
		struct tipeerde_naam benoem;
		struct konkrete_tipe onbenoem;
	};
	struct expr *lyf;
};
DECL_STD_METHS(et_funk);
struct et_roep {
	struct token funksie;
	struct {
		struct expr *items;
		size_t count;
		size_t capacity;
	} argumente;
};
DECL_STD_METHS(et_roep);
struct et_stel_veranderlike {
	struct token veranderlike;
	struct expr *na;
};
DECL_STD_METHS(et_stel_veranderlike);
struct et_tweevoud_operasie {
	struct token operasie;
	struct expr *links;
	struct expr *regs;
};
DECL_STD_METHS(et_tweevoud_operasie);
struct et_eenvoud_operasie {
	struct token operasie;
	struct expr *invoer;
};
DECL_STD_METHS(et_eenvoud_operasie);
struct et_konstante {
	struct token konstante;
};
DECL_STD_METHS(et_konstante);
struct expr {
	enum expr_type type;
	union {
		struct token veranderlike;
		struct et_blok blok;
		struct et_funk funk;
		struct et_roep roep;
		struct et_stel_veranderlike stel_veranderlike;
		struct et_tweevoud_operasie tweevoud_operasie;
		struct et_eenvoud_operasie eenvoud_operasie;
		struct et_konstante konstante;
	};
};
DECL_STD_METHS(expr);

#endif /* PARSER_H */
