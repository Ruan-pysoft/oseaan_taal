#include "parser.h"

#define NOB_IMPLEMENTATION
#include "nob.h"

enum oseaan_type {
	TP_NIKS = 0,
	TP_STR,
};
union oseaan_value_union {
	char *s_value;
};
struct oseaan_value {
	enum oseaan_type type;
	union oseaan_value_union value;
};
void value_destroy(struct oseaan_value value) {
	switch (value.type) {
		case TP_NIKS: break;
		case TP_STR: {
			free(value.value.s_value);
		} break;
	}
}
void print_value(struct oseaan_value value) {
	switch (value.type) {
		case TP_NIKS: {
			printf("niks");
		} break;
		case TP_STR: {
			printf("teks(%s)", value.value.s_value);
		} break;
	}
}
void sb_append_value(Nob_String_Builder *sb, struct oseaan_value value) {
	switch (value.type) {
		case TP_NIKS: {
			nob_sb_appendf(sb, "niks");
		} break;
		case TP_STR: {
			nob_sb_appendf(sb, "teks(%s)", value.value.s_value);
		} break;
	}
}

struct program_state {
};

struct oseaan_value eval_expr(struct program_state *state, struct expr expr) {
	struct oseaan_value res = {0};

	switch (expr.type) {
		case EXPR_FUNCALL: {
			if (strcmp(expr.value.funcall.naam, "druk_lyn") == 0) {
				for (size_t i = 0; i < expr.value.funcall.count; ++i) {
					struct oseaan_value val = eval_expr(state, expr.value.funcall.items[i]);
					print_value(val);
					value_destroy(val);
				}
				putchar('\n');
			} else {
				printf("TODO: parse funksie call\n");
				printf("Naam: %s\n", expr.value.funcall.naam);
			}
		} break;
		case EXPR_COMPOUND: {
			for (size_t i = 0; i < expr.value.compound.count; ++i) {
				struct oseaan_value val = eval_expr(state, expr.value.compound.items[i]);
				printf("In compound expression, got expression value: ");
				print_value(val);
				putchar('\n');
				if (!expr.value.compound.last_empty) {
					value_destroy(res);
					res = val;
				} else {
					value_destroy(val);
				}
			}
		} break;
		case EXPR_STR_LIT: {
			res.type = TP_STR;
			res.value.s_value = strdup(expr.value.str_lit);
		} break;
	}

	return res;
}
void eval_insluiting(struct program_state *state, struct insluiting insluiting) {
	(void) state;
	(void) insluiting;
	printf("TODO: parse insluiting\n");
}
void eval_funksie_definisie(struct program_state *state, struct funksie funksie) {
	if (strcmp(funksie.naam, "hoof") == 0) {
		struct oseaan_value val = eval_expr(state, funksie.lyf);
		printf("Program result (should be niks): ");
		print_value(val);
		putchar('\n');
		value_destroy(val);
	} else {
		printf("TODO: parse funksie definisie\n");
		printf("Naam: %s\n", funksie.naam);
	}
}
void eval(struct program_state *state, struct ast *ast) {
	switch (ast->type) {
		case AST_EOF: break;
		case EXPR: {
			struct oseaan_value val = eval_expr(state, ast->value.expr);
			printf("Got expression value: ");
			print_value(val);
			putchar('\n');
			value_destroy(val);
		} break;
		case INSLUITING: {
			eval_insluiting(state, ast->value.insluiting);
		} break;
		case FUNKSIE_DEFINISIE: {
			eval_funksie_definisie(state, ast->value.funksie_definisie);
		} break;
	}
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	Nob_String_Builder sb = {0};

	if (!nob_read_entire_file("examples/hallo_wereld.os", &sb)) exit(1);

	struct parse_state state = {
		.source = sb.items,
		.source_len = sb.count,
		.idx = 0,

		.verbose = true,

		.tok = {0},
		.has_tok = false,
	};
	struct program_state pstate = { };

	while (state.idx < state.source_len) {
		struct ast *ast = parse_ast(&state);
		printf("AST: ");
		print_ast(ast);
		putchar('\n');
		eval(&pstate, ast);
		ast_destroy(ast);
		free(ast);
	}
	/*
	while (idx < sb.count) {
		struct token tok = lex_token(sb.items, sb.count, &idx);
		printf("TOKEN: %d", tok.type);
		if (tok.type == IDENTIFIER || tok.type == STRING_LIT) {
			printf(" (%s)", tok.value.s_val);
		}
		putchar('\n');
	}
	*/
}
