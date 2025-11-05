#include "typechecking.h"

#include "utils.h"

struct scope {
	struct scope *outer;
};
DECL_STD_METHS(scope);
DESTROY_METH(scope) {
	if (this->outer) scope_destroy(this->outer);
}
COPY_METH(scope) {
	if (this->outer);
}
SB_APPEND_FUNC(scope) {
}
PRINT_IMPL(scope)

static struct {
	size_t n_errors;
	struct scope curr_scope;
} state;

bool typecheck_program(struct program *prog) {
	memset(&state, 0, sizeof(state));

	scope_destroy(&state.curr_scope);
	return state.n_errors == 0;
}
