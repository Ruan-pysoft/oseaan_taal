#include "types.h"
#include "utils.h"

SB_APPEND_FUNC_ENUM(tp_basiese_tipe) {
	assert(sb != NULL);

	nob_sb_append_cstr(sb, tp_basiese_tipe_str[this]);
}
PRINT_IMPL_ENUM(tp_basiese_tipe)

DESTROY_METH(tp_verwysing) {
	assert(this != NULL);

	tipe_destroy(this->na);
	free(this->na);
}
COPY_METH(tp_verwysing) {
	assert(this != NULL);

	struct tp_verwysing res = {0};
	res.na = malloc(sizeof(*res.na));
	*res.na = tipe_copy(this->na);
	return res;
}
SB_APPEND_FUNC(tp_verwysing) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "^");
	sb_append_tipe(sb, this->na);
}
PRINT_IMPL(tp_verwysing)

DESTROY_METH(tp_vaste_lys) {
	assert(this != NULL);

	tipe_destroy(this->van);
	free(this->van);
}
COPY_METH(tp_vaste_lys) {
	assert(this != NULL);

	struct tp_vaste_lys res = {0};
	res.van = malloc(sizeof(*res.van));
	*res.van = tipe_copy(this->van);
	return res;
}
SB_APPEND_FUNC(tp_vaste_lys) {
	assert(sb != NULL && this != NULL);

	nob_sb_appendf(sb, "[%lu]", this->lengte);
	sb_append_tipe(sb, this->van);
}
PRINT_IMPL(tp_vaste_lys)

DESTROY_METH(tp_dinamiese_lys) {
	assert(this != NULL);

	tipe_destroy(this->van);
	free(this->van);
}
COPY_METH(tp_dinamiese_lys) {
	assert(this != NULL);

	struct tp_dinamiese_lys res = {0};
	res.van = malloc(sizeof(*res.van));
	*res.van = tipe_copy(this->van);
	return res;
}
SB_APPEND_FUNC(tp_dinamiese_lys) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "[*]");
	sb_append_tipe(sb, this->van);
}
PRINT_IMPL(tp_dinamiese_lys)

DESTROY_METH(tp_argumente) {
	assert(this != NULL);

	nob_da_foreach(struct tipe, it, this) {
		tipe_destroy(it);
	}
	nob_da_free(*this);
}
COPY_METH(tp_argumente) {
	assert(this != NULL);

	struct tp_argumente res = {0};
	res.items = calloc(this->count, sizeof(*res.items));
	res.count = this->count;
	res.count = this->count;
	for (size_t i = 0; i < this->count; ++i) {
		res.items[i] = this->items[i];
	}
	return res;
}
SB_APPEND_FUNC(tp_argumente) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "(");
	bool first = true;
	nob_da_foreach(struct tipe, it, this) {
		if (!first) {
			nob_sb_append_cstr(sb, ", ");
			first = false;
		}
		nob_sb_append_cstr(sb, ": ");
		sb_append_tipe(sb, it);
	}
	nob_sb_append_cstr(sb, ")");
}
PRINT_IMPL(tp_argumente)

DESTROY_METH(tp_funksie) {
	assert(this != NULL);

	tipe_destroy(this->terugkeer);
	free(this->terugkeer);
	tp_argumente_destroy(&this->argumente);
}
COPY_METH(tp_funksie) {
	assert(this != NULL);

	struct tp_funksie res = {0};
	res.terugkeer = malloc(sizeof(*res.terugkeer));
	*res.terugkeer = tipe_copy(this->terugkeer);
	res.argumente = tp_argumente_copy(&this->argumente);
	return res;
}
SB_APPEND_FUNC(tp_funksie) {
	assert(sb != NULL && this != NULL);

	nob_sb_append_cstr(sb, "funk");
	sb_append_tp_argumente(sb, &this->argumente);
	nob_sb_append_cstr(sb, ": ");
	sb_append_tipe(sb, this->terugkeer);
}
PRINT_IMPL(tp_funksie)

DESTROY_METH(tipe) {
	assert(this != NULL);

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
	assert(this != NULL);

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
	assert(sb != NULL && this != NULL);

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
