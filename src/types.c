#include "types.h"
#include "utils.h"

SB_APPEND_FUNC_ENUM(veranderlikheid) {
	bt_assert(sb != NULL);

	nob_sb_append_cstr(sb, veranderlikheid_str[this]);
}
PRINT_IMPL_ENUM(veranderlikheid)

SB_APPEND_FUNC_ENUM(tp_basiese_tipe) {
	bt_assert(sb != NULL);

	nob_sb_append_cstr(sb, tp_basiese_tipe_str[this]);
}
PRINT_IMPL_ENUM(tp_basiese_tipe)
bool tp_basiese_tipe_tequiv(enum tp_basiese_tipe lhs, enum tp_basiese_tipe rhs) {
	return lhs == rhs;
}

DESTROY_METH(tp_verwysing) {
	bt_assert(this != NULL);

	konkrete_tipe_destroy(this->na);
	free(this->na);
}
COPY_METH(tp_verwysing) {
	bt_assert(this != NULL);

	struct tp_verwysing res = {0};
	res.na = malloc(sizeof(*res.na));
	*res.na = konkrete_tipe_copy(this->na);
	return res;
}
SB_APPEND_FUNC(tp_verwysing) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "^");
	sb_append_konkrete_tipe(sb, this->na);
}
PRINT_IMPL(tp_verwysing)
bool tp_verwysing_tequiv(struct tp_verwysing *lhs, struct tp_verwysing *rhs) {
	bt_assert(lhs != NULL && rhs != NULL);

	return konkrete_tipe_tequiv(lhs->na, rhs->na);
}

DESTROY_METH(tp_vaste_lys) {
	bt_assert(this != NULL);

	konkrete_tipe_destroy(this->van);
	free(this->van);
}
COPY_METH(tp_vaste_lys) {
	bt_assert(this != NULL);

	struct tp_vaste_lys res = {0};
	res.van = malloc(sizeof(*res.van));
	*res.van = konkrete_tipe_copy(this->van);
	return res;
}
SB_APPEND_FUNC(tp_vaste_lys) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "[%lu]", this->lengte);
	sb_append_konkrete_tipe(sb, this->van);
}
PRINT_IMPL(tp_vaste_lys)
bool tp_vaste_lys_tequiv(struct tp_vaste_lys *lhs, struct tp_vaste_lys *rhs) {
	bt_assert(lhs != NULL && rhs != NULL);

	return lhs->lengte == rhs->lengte && konkrete_tipe_tequiv(lhs->van, rhs->van);
}

DESTROY_METH(tp_dinamiese_lys) {
	bt_assert(this != NULL);

	konkrete_tipe_destroy(this->van);
	free(this->van);
}
COPY_METH(tp_dinamiese_lys) {
	bt_assert(this != NULL);

	struct tp_dinamiese_lys res = {0};
	res.van = malloc(sizeof(*res.van));
	*res.van = konkrete_tipe_copy(this->van);
	return res;
}
SB_APPEND_FUNC(tp_dinamiese_lys) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "[*]");
	sb_append_konkrete_tipe(sb, this->van);
}
PRINT_IMPL(tp_dinamiese_lys)
bool tp_dinamiese_lys_tequiv(struct tp_dinamiese_lys *lhs, struct tp_dinamiese_lys *rhs) {
	bt_assert(lhs != NULL && rhs != NULL);

	return konkrete_tipe_tequiv(lhs->van, rhs->van);
}

DESTROY_METH(tp_argumente) {
	bt_assert(this != NULL);

	nob_da_foreach(struct konkrete_tipe, it, this) {
		konkrete_tipe_destroy(it);
	}
	nob_da_free(*this);
}
COPY_METH(tp_argumente) {
	bt_assert(this != NULL);

	struct tp_argumente res = {0};
	res.items = calloc(this->count, sizeof(*res.items));
	res.count = this->count;
	res.count = this->count;
	for (size_t i = 0; i < this->count; ++i) {
		res.items[i] = konkrete_tipe_copy(&this->items[i]);
	}
	return res;
}
SB_APPEND_FUNC(tp_argumente) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "(");
	bool first = true;
	nob_da_foreach(struct konkrete_tipe, it, this) {
		if (!first) {
			nob_sb_append_cstr(sb, ", ");
		}
		first = false;
		nob_sb_append_cstr(sb, ": ");
		sb_append_konkrete_tipe(sb, it);
	}
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(tp_argumente)
bool tp_argumente_tequiv(struct tp_argumente *lhs, struct tp_argumente *rhs) {
	bt_assert(lhs != NULL && rhs != NULL);

	if (lhs->count != rhs->count) return false;
	for (size_t i = 0; i < lhs->count; ++i) {
		if (!konkrete_tipe_tequiv(&lhs->items[i], &rhs->items[i])) return false;
	}
	return true;
}

DESTROY_METH(tp_funksie) {
	bt_assert(this != NULL);

	konkrete_tipe_destroy(this->terugkeer);
	free(this->terugkeer);
	tp_argumente_destroy(&this->argumente);
}
COPY_METH(tp_funksie) {
	bt_assert(this != NULL);

	struct tp_funksie res = {0};
	res.terugkeer = malloc(sizeof(*res.terugkeer));
	*res.terugkeer = konkrete_tipe_copy(this->terugkeer);
	res.argumente = tp_argumente_copy(&this->argumente);
	return res;
}
SB_APPEND_FUNC(tp_funksie) {
	bt_assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "funk");
	sb_append_tp_argumente(sb, &this->argumente);
	nob_sb_append_cstr(sb, " -> :");
	sb_append_konkrete_tipe(sb, this->terugkeer);
}
PRINT_IMPL(tp_funksie)
bool tp_funksie_tequiv(struct tp_funksie *lhs, struct tp_funksie *rhs) {
	bt_assert(lhs != NULL && rhs != NULL);

	return konkrete_tipe_tequiv(lhs->terugkeer, rhs->terugkeer) && tp_argumente_tequiv(&lhs->argumente, &rhs->argumente);
}

DESTROY_METH(tipe) {
	bt_assert(this != NULL);

	switch (this->klas) {
		case kvt_basies: break;
		case kvt_verwysing: {
			tp_verwysing_destroy(&this->verwysing);
		} break;
		case kvt_vaste_lys: {
			tp_vaste_lys_destroy(&this->vaste_lys);
		} break;
		case kvt_dinamiese_lys: {
			tp_dinamiese_lys_destroy(&this->dinamiese_lys);
		} break;
		case kvt_funksie: {
			tp_funksie_destroy(&this->funksie);
		} break;
	}
}
COPY_METH(tipe) {
	bt_assert(this != NULL);

	struct tipe res = {0};
	res.klas = this->klas;
	switch (this->klas) {
		case kvt_basies: {
			res.basies = this->basies;
		} break;
		case kvt_verwysing: {
			res.verwysing = tp_verwysing_copy(&this->verwysing);
		} break;
		case kvt_vaste_lys: {
			res.vaste_lys = tp_vaste_lys_copy(&this->vaste_lys);
		} break;
		case kvt_dinamiese_lys: {
			res.dinamiese_lys = tp_dinamiese_lys_copy(&this->dinamiese_lys);
		} break;
		case kvt_funksie: {
			res.funksie = tp_funksie_copy(&this->funksie);
		} break;
	}
	return res;
}
SB_APPEND_FUNC(tipe) {
	bt_assert(sb != NULL && this != NULL);

	switch (this->klas) {
		case kvt_basies: {
			sb_append_tp_basiese_tipe(sb, this->basies);
		} break;
		case kvt_verwysing: {
			sb_append_tp_verwysing(sb, &this->verwysing);
		} break;
		case kvt_vaste_lys: {
			sb_append_tp_vaste_lys(sb, &this->vaste_lys);
		} break;
		case kvt_dinamiese_lys: {
			sb_append_tp_dinamiese_lys(sb, &this->dinamiese_lys);
		} break;
		case kvt_funksie: {
			sb_append_tp_funksie(sb, &this->funksie);
		} break;
	}
}
PRINT_IMPL(tipe)
bool tipe_tequiv(struct tipe *lhs, struct tipe *rhs) {
	bt_assert(lhs != NULL && rhs != NULL);

	if (lhs->klas != rhs->klas) return false;
	switch (lhs->klas) {
		case kvt_basies: return tp_basiese_tipe_tequiv(lhs->basies, rhs->basies);
		case kvt_verwysing: return tp_verwysing_tequiv(&lhs->verwysing, &rhs->verwysing);
		case kvt_vaste_lys: return tp_vaste_lys_tequiv(&lhs->vaste_lys, &rhs->vaste_lys);
		case kvt_dinamiese_lys: return tp_dinamiese_lys_tequiv(&lhs->dinamiese_lys, &rhs->dinamiese_lys);
		case kvt_funksie: return tp_funksie_tequiv(&lhs->funksie, &rhs->funksie);
	}
}

DESTROY_METH(konkrete_tipe) {
	bt_assert(this != NULL);
	tipe_destroy(&this->tipe);
}
COPY_METH(konkrete_tipe) {
	bt_assert(this != NULL);

	struct konkrete_tipe res = {0};
	res.tipe = tipe_copy(&this->tipe);
	res.tipe_vlh = this->tipe_vlh;

	return res;
}
SB_APPEND_FUNC(konkrete_tipe) {
	bt_assert(sb != NULL && this != NULL);

	sb_append_veranderlikheid(sb, this->tipe_vlh);
	nob_sb_append_cstr(sb, " ");
	sb_append_tipe(sb, &this->tipe);
}
PRINT_IMPL(konkrete_tipe)
bool konkrete_tipe_tequiv(struct konkrete_tipe *lhs, struct konkrete_tipe *rhs) {
	bt_assert(lhs != NULL && rhs != NULL);

	return tipe_tequiv(&lhs->tipe, &rhs->tipe);
}
