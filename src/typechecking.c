#include "typechecking.h"

#include <string.h>

#include "parser.h"
#include "types.h"
#include "utils.h"

#undef UTIL_METH
#define UTIL_METH static

struct assoc {
	char *naam;
	struct konkrete_tipe tipe;
};
DECL_STD_METHS(assoc);
DESTROY_METH(assoc) {
	bt_assert(this != NULL);

	free(this->naam);
	konkrete_tipe_destroy(&this->tipe);
}
COPY_METH(assoc) {
	bt_assert(this != NULL);

	struct assoc res = {0};
	res.naam = strdup(this->naam);
	res.tipe = konkrete_tipe_copy(&this->tipe);

	return res;
}
SB_APPEND_FUNC(assoc) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, this->naam);
	nob_sb_append_cstr(sb, ": ");
	sb_append_konkrete_tipe(sb, &this->tipe);
}
PRINT_IMPL(assoc)

struct scope {
	struct assoc *items;
	size_t count;
	size_t capacity;

	struct scope *outer;
};
DECL_STD_METHS(scope);
DESTROY_METH(scope) {
	bt_assert(this != NULL);

	nob_da_foreach(struct assoc, it, this) {
		assoc_destroy(it);
	}
	nob_da_free(*this);

	if (this->outer) scope_destroy(this->outer);
}
COPY_METH(scope) {
	bt_assert(this != NULL);

	struct scope res = {0};
	nob_da_foreach(struct assoc, it, this) {
		nob_da_append(&res, assoc_copy(it));
	}
	if (this->outer) res.outer = this->outer;

	return res;
}
SB_APPEND_FUNC(scope) {
	bt_assert(sb != NULL && this != NULL);

	bool first = true;
	nob_sb_append_cstr(sb, "{ ");
	nob_da_foreach(struct assoc, it, this) {
		if (!first) nob_sb_append_cstr(sb, ", ");
		first = false;
		sb_append_assoc(sb, it);
	}
	nob_sb_append_cstr(sb, " }\n");
	if (this->outer) {
		nob_sb_append_cstr(sb, "Outer Scope: ");
		sb_append_scope(sb, this->outer);
		nob_sb_append_cstr(sb, "\n");
	} else {
		nob_sb_append_cstr(sb, "Outer Scope: (nil)\n");
	}
}
PRINT_IMPL(scope)
struct assoc *scope_get_local(const struct scope *this, const char *str, size_t str_len) {
	bt_assert(this != NULL);

	fprintf(stderr, "Looking for symbol \"%.*s\" in scope %p...\n", (int)str_len, str, (void*)this);

	nob_da_foreach(struct assoc, it, this) {
		fprintf(stderr, " -> Checking against symbol \"%s\"...\n", it->naam);
		if (strnlen(it->naam, str_len+1) == str_len && strncmp(it->naam, str, str_len) == 0) {
			fputs("    Matched!\n", stderr);
			return it;
		}
	}

	return NULL;
}
struct assoc *scope_get(const struct scope *this, const char *str, size_t str_len) {
	bt_assert(this != NULL);

	struct assoc *it = NULL;
	const struct scope *curr_scope = this;;
	while (!it) {
		if (this == NULL) return NULL;
		it = scope_get_local(curr_scope, str, str_len);
		curr_scope = curr_scope->outer;
	}
	return it;
}

static struct {
	size_t n_errors;
	struct scope curr_scope;
} state;
static void push_scope() {
	struct scope *old = malloc(sizeof(*old));
	*old = state.curr_scope;
	state.curr_scope = (struct scope){0};
	state.curr_scope.outer = old;
}
static void pop_scope() {
	struct scope *outer = state.curr_scope.outer;
	state.curr_scope.outer = NULL;
	scope_destroy(&state.curr_scope);
	state.curr_scope = *outer;
}

struct konkrete_tipe typeresolve_expr(struct expr *this) {
	switch (this->type) {
		case ET_VERANDERLIKE: {
			struct assoc *it = scope_get(
				&state.curr_scope,
				&this->veranderlike.pos.source[this->veranderlike.pos.idx],
				this->veranderlike.len
			);
			if (it == NULL) {
				fputs("FOUT: veranderlike nie voorheen verklaar nie!\n", stderr);
				++state.n_errors;
				struct konkrete_tipe res = {0};
				res.tipe_vlh = v_konstant;
				res.tipe.klas = kvt_basies;
				res.tipe.basies = bt_niks;
				return res;
			}
			return konkrete_tipe_copy(&it->tipe);
		} break;
		case ET_BLOK: {
			if (this->blok.res == NULL) {
				struct konkrete_tipe res = {0};
				res.tipe_vlh = v_konstant;
				res.tipe.klas = kvt_basies;
				res.tipe.basies = bt_niks;
				return res;
			}
			return typeresolve_expr(this->blok.res);
		} break;
		case ET_FUNK: {
			fputs("TODO: typecheck function body\n", stderr);
			struct konkrete_tipe res = {0};
			res.tipe_vlh = v_konstant;
			res.tipe.klas = kvt_funksie;
			struct tp_funksie *funk = &res.tipe.funksie;
			funk->terugkeer = calloc(1, sizeof(*funk->terugkeer));
			*funk->terugkeer = konkrete_tipe_copy(this->funk.benoemde_terugkeerwaarde
				? &this->funk.benoem.tipe
				: &this->funk.onbenoem
			);
			nob_da_foreach(struct tipeerde_naam, it, &this->funk.argumente) {
				nob_da_append(&funk->argumente, konkrete_tipe_copy(&it->tipe));
			}
			return res;
		} break;
		case ET_ROEP: {
			fputs("TODO: typecheck function args\n", stderr);
			struct assoc *it = scope_get(
				&state.curr_scope,
				&this->roep.funksie.pos.source[this->roep.funksie.pos.idx],
				this->roep.funksie.len
			);
			if (it == NULL) {
				fputs("FOUT: veranderlike nie voorheen verklaar nie!\n", stderr);
				++state.n_errors;
				struct konkrete_tipe res = {0};
				res.tipe_vlh = v_konstant;
				res.tipe.klas = kvt_basies;
				res.tipe.basies = bt_niks;
				return res;
			}
			return konkrete_tipe_copy(&it->tipe);
		} break;
		case ET_STEL_VERANDERLIKE: {
			bt_assert(false && "TODO");
			return (struct konkrete_tipe) {0};
		} break;
		case ET_TWEEVOUD_OPERASIE: {
			bt_assert(false && "TODO");
			return (struct konkrete_tipe) {0};
		} break;
		case ET_EENVOUD_OPERASIE: {
			bt_assert(false && "TODO");
			return (struct konkrete_tipe) {0};
		} break;
		case ET_KONSTANTE: {
			bt_assert(false && "TODO");
			return (struct konkrete_tipe) {0};
		} break;
	}
}

void typecheck_deklarasie(struct st_deklarasie *this) {
	struct assoc *it = scope_get_local(
		&state.curr_scope,
		&this->veranderlike.naam.pos.source[this->veranderlike.naam.pos.idx],
		this->veranderlike.naam.len
	);

	if (it == NULL) {
		struct assoc new_assoc = {0};
		new_assoc.naam = strndup(
			&this->veranderlike.naam.pos.source[this->veranderlike.naam.pos.idx],
			this->veranderlike.naam.len
		);
		new_assoc.tipe = konkrete_tipe_copy(&this->veranderlike.tipe);
		nob_da_append(&state.curr_scope, new_assoc);
	} else if (!konkrete_tipe_tequiv(&it->tipe, &this->veranderlike.tipe)) {
		fputs("FOUT: Tipe van deklarasie verskil van vorige tipe!\n", stderr);
		++state.n_errors;
	}
}
void typecheck_definisie(struct st_definisie *this) {
	struct assoc *it = scope_get_local(
		&state.curr_scope,
		&this->veranderlike.naam.pos.source[this->veranderlike.naam.pos.idx],
		this->veranderlike.naam.len
	);

	if (it == NULL) {
		struct assoc new_assoc = {0};
		new_assoc.naam = strndup(
			&this->veranderlike.naam.pos.source[this->veranderlike.naam.pos.idx],
			this->veranderlike.naam.len
		);
		new_assoc.tipe = konkrete_tipe_copy(&this->veranderlike.tipe);
		nob_da_append(&state.curr_scope, new_assoc);
	} else if (!konkrete_tipe_tequiv(&it->tipe, &this->veranderlike.tipe)) {
		fputs("FOUT: Tipe van definisie verskil van vorige tipe!\n", stderr);
		++state.n_errors;
	}

	struct konkrete_tipe wat_tipe = typeresolve_expr(this->wat);
	if (!konkrete_tipe_tequiv(&this->veranderlike.tipe, &wat_tipe)) {
		fputs("FOUT: Tipe van definisie waarde verskil van gedeklareerde tipe!\n", stderr);
		++state.n_errors;
	}
	konkrete_tipe_destroy(&wat_tipe);
}
void typecheck_funksie_definisie(struct st_funksie *this) {
	struct assoc *it = scope_get_local(
		&state.curr_scope,
		&this->naam.pos.source[this->naam.pos.idx],
		this->naam.len
	);

	struct konkrete_tipe fn_tipe = {0};
	{
		fn_tipe.tipe_vlh = v_konstant;
		fn_tipe.tipe.klas = kvt_funksie;
		struct tp_funksie *funk = &fn_tipe.tipe.funksie;
		funk->terugkeer = calloc(1, sizeof(*funk->terugkeer));
		*funk->terugkeer = konkrete_tipe_copy(this->benoemde_terugkeerwaarde
			? &this->benoem.tipe
			: &this->onbenoem
		);
		nob_da_foreach(struct tipeerde_naam, it, &this->argumente) {
			nob_da_append(&funk->argumente, konkrete_tipe_copy(&it->tipe));
		}
	}

	if (it == NULL) {
		struct assoc new_assoc = {0};
		new_assoc.naam = strndup(
			&this->naam.pos.source[this->naam.pos.idx],
			this->naam.len
		);
		new_assoc.tipe = fn_tipe;
		nob_da_append(&state.curr_scope, new_assoc);
		return;
	} else if (!konkrete_tipe_tequiv(&it->tipe, &fn_tipe)) {
		fputs("FOUT: Tipe van funksie definisie verskil van vorige tipe!\n", stderr);
		++state.n_errors;
	}
	konkrete_tipe_destroy(&fn_tipe);
}
void typecheck_statement(struct statement *this) {
	bt_assert(this != NULL);

	switch (this->type) {
		case ST_INSLUITING: break;
		case ST_DEKLARASIE: {
			typecheck_deklarasie(&this->deklarasie);
		} break;
		case ST_DEFINISIE: {
			typecheck_definisie(&this->definisie);
		} break;
		case ST_FUNKSIE: {
			typecheck_funksie_definisie(&this->funksie);
		} break;
	}
	fputs("Current Scope: ", stderr);
	fprint_scope(&state.curr_scope, stderr);
}

bool typecheck_program(struct program *prog) {
	assert(prog != NULL);

	memset(&state, 0, sizeof(state));

	nob_da_foreach(struct statement, it, prog) {
		typecheck_statement(it);
		assert(
			state.curr_scope.outer == NULL
			&& "Expected that all new scopes created in typechecking a statement should've been popped afterwards"
		);
	}

	scope_destroy(&state.curr_scope);
	return state.n_errors == 0;
}
