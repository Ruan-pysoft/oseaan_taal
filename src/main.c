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

struct function_signature {
	enum oseaan_type *items; // arguments
	size_t count;
	size_t capacity;

	char *naam;
	enum oseaan_type return_type;
};

enum target_type {
	TARGET_X86_64,
	TARGET_INTERPRET,
};

struct target_x86_64 {
	enum oseaan_type *items; // typestack
	size_t count;
	size_t capacity;

	Nob_String_Builder prelude;
	Nob_String_Builder data;
	Nob_String_Builder code;
};
void target_x86_64_destroy(struct target_x86_64 target) {
	nob_da_free(target);
	nob_sb_free(target.prelude);
	nob_sb_free(target.data);
	nob_sb_free(target.code);
}
#define X86_64_INT_ARG0 "rdi"
#define X86_64_INT_ARG1 "rsi"
#define X86_64_INT_ARG2 "rdx"
#define X86_64_INT_ARG3 "rcx"
#define X86_64_INT_ARG4 "r8"
#define X86_64_INT_ARG5 "r9"
const char *x86_64_int_arg_regs[] = {
	X86_64_INT_ARG0,
	X86_64_INT_ARG1,
	X86_64_INT_ARG2,
	X86_64_INT_ARG3,
	X86_64_INT_ARG4,
	X86_64_INT_ARG5,
};

struct target_interpret {
	struct ifunksie {
		char *naam;
		bool is_builtin;
		union {
			struct expr lyf;
			struct oseaan_value (*builtin)(struct oseaan_value*);
		};
	} *items;
	size_t count;
	size_t capacity;

	struct oseaan_value last_val;
};
void target_interpret_destroy(struct target_interpret target) {
	for (size_t i = 0; i < target.count; ++i) {
		free(target.items[i].naam);
		if (!target.items[i].is_builtin) {
			expr_destroy(&target.items[i].lyf);
		}
	}
	nob_da_free(target);
}

struct program_state {
	struct function_signature *items;
	size_t count;
	size_t capacity;

	enum target_type type;
	union {
		struct target_x86_64 x86_64;
		struct target_interpret interpret;
	};

	bool verbose;
};
struct program_state program_state_init(enum target_type type) {
	struct program_state res = {
		.items = NULL,
		.count = 0,
		.capacity = 0,
		.type = type,
		.verbose = false,
	};

	switch (type) {
		case TARGET_X86_64: {
			res.x86_64 = (struct target_x86_64) {
				.prelude = {0},
				.data = {0},
				.code = {0},
			};
			nob_sb_append_cstr(&res.x86_64.prelude,
				"global _start\n"
				"SYS_WRITE equ 1\n"
				"SYS_EXIT  equ 60\n"
				"STDOUT    equ 1\n"
			);
			nob_sb_append_cstr(&res.x86_64.data,
				"SECTION .data\n"
			);
			nob_sb_append_cstr(&res.x86_64.code,
				"SECTION .text\n"
			);
		} break;
		case TARGET_INTERPRET: {
		} break;
	}

	return res;
}
void program_state_destroy(struct program_state *state) {
	for (size_t i = 0; i < state->count; ++i) {
		nob_da_free(state->items[i]);
		free(state->items[i].naam);
	}
	nob_da_free(*state);

	switch (state->type) {
		case TARGET_X86_64: {
			target_x86_64_destroy(state->x86_64);
		} break;
		case TARGET_INTERPRET: {
			target_interpret_destroy(state->interpret);
		} break;
	}
}
#define VERB(str) if (state->verbose) nob_log(NOB_INFO, str)
#define VERBF(fmt, ...) if (state->verbose) nob_log(NOB_INFO, fmt, __VA_ARGS__)

void eval_expr(struct program_state *state, struct expr expr);
void visit_eof(struct program_state *state) {
	VERB("Visiting EOF");
	switch (state->type) {
		case TARGET_X86_64: {
			Nob_String_Builder *sb = &state->x86_64.code;

			nob_sb_append_cstr(sb,
				"_start:\n"
				"  call hoof\n"
				"  ; syscall(SYS_EXIT, 0)\n"
				"  mov rax, SYS_EXIT\n"
				"  xor rdi, rdi\n"
				"  syscall\n"
			);

			nob_sb_append_null(&state->x86_64.prelude);
			nob_sb_append_null(&state->x86_64.code);
			nob_sb_append_null(&state->x86_64.data);
		} break;
		case TARGET_INTERPRET: {
			for (size_t i = 0; i < state->interpret.count; ++i) {
				VERBF("Checking function %s for main...", state->interpret.items[i].naam);
				if (strcmp(state->interpret.items[i].naam, "hoof") == 0) {
					eval_expr(state, state->interpret.items[i].lyf);
					printf("Program result (should be niks): ");
					print_value(state->interpret.last_val);
					putchar('\n');
					value_destroy(state->interpret.last_val);
					goto found_main;
				}
			}
			nob_log(NOB_ERROR, "No main function provided!");
		found_main:;
		} break;
	}
}
void visit_funcall(struct program_state *state, struct funcall funcall) {
	VERB("Visiting funcall");
	switch (state->type) {
		case TARGET_X86_64: {
			// only support up to six integer arguments for now
			Nob_String_Builder *sb = &state->x86_64.code;
			int args_pushed = 0;
			for (size_t i = 0; i < funcall.count; ++i) {
				eval_expr(state, funcall.items[i]);
				switch (state->x86_64.items[--state->x86_64.count]) {
					case TP_NIKS: {
						nob_sb_appendf(sb,
							"  pop %s\n"
						, x86_64_int_arg_regs[args_pushed++]);
					} break;
					case TP_STR: {
						const char *arg0_reg = x86_64_int_arg_regs[args_pushed++];
						const char *arg1_reg = x86_64_int_arg_regs[args_pushed++];
						nob_sb_appendf(sb,
							"  pop %s\n"
							"  pop %s\n"
						, arg1_reg, arg0_reg);
					} break;
				}
			}

			nob_sb_appendf(sb,
				"  call %s\n"
				"  push rax\n"
			, funcall.naam);
			nob_da_append(&state->x86_64, TP_NIKS);
		} break;
		case TARGET_INTERPRET: {
			const struct ifunksie *funk = NULL;
			for (size_t i = 0; i < state->interpret.count; ++i) {
				funk = &state->interpret.items[i];
				if (strcmp(funcall.naam, funk->naam) == 0) {
					goto called_funk;
				}
			}
			nob_log(NOB_ERROR, "Undefined function %s", funcall.naam);
			exit(1);
		called_funk:;
			struct {
				struct oseaan_value *items;
				size_t count;
				size_t capacity;
			} args = {0};
			for (size_t i = 0; i < funcall.count; ++i) {
				eval_expr(state, funcall.items[i]);
				nob_da_append(&args, state->interpret.last_val);
			}
			if (funk->is_builtin) {
				state->interpret.last_val = funk->builtin(args.items);
				for (size_t i = 0; i < args.count; ++i) {
					value_destroy(args.items[i]);
				}
				nob_da_free(args);
			} else {
				printf("TODO: parse funksie call\n");
				printf("Naam: %s\n", funcall.naam);
				state->interpret.last_val = (struct oseaan_value) {0};
			}
		} break;
	}
}
void visit_compound(struct program_state *state, struct compound compound) {
	VERB("Visiting compound");
	switch (state->type) {
		case TARGET_X86_64: {
			Nob_String_Builder *sb = &state->x86_64.code;
			for (size_t i = 0; i < compound.count; ++i) {
				if (i) {
					nob_sb_append_cstr(sb, "  pop rax\n");
					--state->x86_64.count;
				}
				eval_expr(state, compound.items[i]);
			}
			if (compound.last_empty) {
				nob_sb_append_cstr(sb, "  pop rax\n  push 0\n");
				--state->x86_64.count;
				nob_da_append(&state->x86_64, TP_NIKS);
			}
		} break;
		case TARGET_INTERPRET: {
			struct oseaan_value res = {0};
			for (size_t i = 0; i < compound.count; ++i) {
				eval_expr(state, compound.items[i]);
				if (state->verbose) {
					printf("In compound expression, got expression value: ");
					print_value(state->interpret.last_val);
					putchar('\n');
				}
				if (!compound.last_empty) {
					value_destroy(res);
					res = state->interpret.last_val;
				} else {
					value_destroy(state->interpret.last_val);
				}
			}
			state->interpret.last_val = res;
		} break;
	}
}
void visit_str_lit(struct program_state *state, char *str_lit) {
	VERB("Visiting string literal");
	switch (state->type) {
		case TARGET_X86_64: {
			static size_t strlit_counter = 0;
			Nob_String_Builder *sb = &state->x86_64.data;
			nob_sb_appendf(sb,
				"strlit_%zu db \"%s\"\n"
				"strlit_%zu_len equ $-strlit_%zu\n"
			, strlit_counter, str_lit, strlit_counter, strlit_counter);

			sb = &state->x86_64.code;
			nob_sb_appendf(sb,
				"  push strlit_%zu\n"
				"  push strlit_%zu_len\n"
			, strlit_counter, strlit_counter);
			nob_da_append(&state->x86_64, TP_STR);

			++strlit_counter;
		} break;
		case TARGET_INTERPRET: {
			state->interpret.last_val.type = TP_STR;
			state->interpret.last_val.value.s_value = strdup(str_lit);
		} break;
	}
}
void visit_funksie_definisie(struct program_state *state, struct funksie funksie) {
	VERB("Visiting funksie definisie");
	switch (state->type) {
		case TARGET_X86_64: {
			Nob_String_Builder *sb = &state->x86_64.code;
			nob_sb_appendf(sb,
				"%s:\n"
				"  push rbp\n"
				"  mov rbp, rsp\n"
			, funksie.naam);
			eval_expr(state, funksie.lyf);
			nob_sb_append_cstr(sb,
				"  pop rax\n"
				"  pop rbp\n"
				"  ret\n"
			);
		} break;
		case TARGET_INTERPRET: {
			struct ifunksie funk = {
				.naam = strdup(funksie.naam),
				.is_builtin = false,
				.lyf = expr_copy(&funksie.lyf),
			};
			nob_da_append(&state->interpret, funk);
		} break;
	}
}

void eval_expr(struct program_state *state, struct expr expr) {
	switch (expr.type) {
		case EXPR_FUNCALL: {
			visit_funcall(state, expr.value.funcall);
		} break;
		case EXPR_COMPOUND: {
			visit_compound(state, expr.value.compound);
		} break;
		case EXPR_STR_LIT: {
			visit_str_lit(state, expr.value.str_lit);
		} break;
	}
}
struct oseaan_value builtin_druk_lyn(struct oseaan_value *args) {
	if (args[0].type != TP_STR) {
		nob_log(NOB_ERROR, "Expected string for argument!");
	}
	printf("%s\n", args[0].value.s_value);
	return (struct oseaan_value) {0};
}
void eval_insluiting(struct program_state *state, struct insluiting insluiting) {
	if (strcmp(insluiting.module, "stdiu") == 0) {
		struct function_signature builtin_druk_lyn_sig = {
			.items = NULL,
			.count = 0,
			.capacity = 0,
			.naam = strdup("druk lyn"),
			.return_type = TP_NIKS,
		};
		nob_da_append(&builtin_druk_lyn_sig, TP_STR);
		nob_da_append(state, builtin_druk_lyn_sig);
	}
	switch (state->type) {
		case TARGET_X86_64: {
			if (strcmp(insluiting.module, "stdiu") == 0) {
				nob_sb_append_cstr(&state->x86_64.code,
					"druk_lyn: ; van module stdiu\n"
					"  push rbp\n"
					"  mov rbp, rsp\n"

					"  ; syscall(SYS_WRITE, STDOUT, arg0, arg1)\n"
					"  mov rdx, "X86_64_INT_ARG1" ; arg1 --> arg3\n"
					"  mov rsi, "X86_64_INT_ARG0" ; arg0 --> arg2\n"
					"  mov rax, SYS_WRITE\n"
					"  mov rdi, STDOUT\n"
					"  syscall\n"
					"  ; if write failed (rax < 0), return"
					"  text rax, rax\n"
					"  js .exit\n"

					"  mov rbx, rax ; save rax into rbx\n"
					"  ; syscall(SYS_WRITE, STDOUT, \"\\n\", 1)\n"
					"  mov rsi, .nl\n"
					"  mov rdx, 1\n"
					"  mov rax, SYS_WRITE\n"
					"  syscall\n"
					"  ; if write failed (rax < 0), return"
					"  text rax, rax\n"
					"  js .exit\n"
					"  add rax, rbx ; update total bytes written\n"

					".exit:\n"
					"  pop rbp\n"
					"  ret\n"
					".nl: db 10\n"
				);
			} else {
				printf("TODO: parse insluiting\n");
			}
		} break;
		case TARGET_INTERPRET: {
			if (strcmp(insluiting.module, "stdiu") == 0) {
				struct ifunksie funk = {
					.naam = strdup("druk_lyn"),
					.is_builtin = true,
					.builtin = builtin_druk_lyn,
				};
				nob_da_append(&state->interpret, funk);
			} else {
				printf("TODO: parse insluiting\n");
			}
		} break;
	}
}
void eval(struct program_state *state, struct ast *ast) {
	switch (ast->type) {
		case AST_EOF: {
			visit_eof(state);
		} break;
		case EXPR: {
			nob_log(NOB_ERROR, "Didn't exptect an expression here");
			exit(1);
			/*struct oseaan_value val = eval_expr(state, ast->value.expr);
			printf("Got expression value: ");
			print_value(val);
			putchar('\n');
			value_destroy(val);*/
		} break;
		case INSLUITING: {
			eval_insluiting(state, ast->value.insluiting);
		} break;
		case FUNKSIE_DEFINISIE: {
			visit_funksie_definisie(state, ast->value.funksie_definisie);
		} break;
	}
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	Nob_String_Builder sb = {0};

	if (!nob_read_entire_file("examples/hallo_wereld.os", &sb)) exit(1);

	nob_sb_append_null(&sb);
	struct parse_state state = parse_state_init(sb.items);
	struct program_state pstate = program_state_init(TARGET_INTERPRET);

	while (!st_atend(&state.src)) {
		struct ast ast = parse_ast(&state);
		printf("AST: ");
		print_ast(&ast);
		putchar('\n');
		eval(&pstate, &ast);
		ast_destroy(&ast);
	}

	program_state_destroy(&pstate);

	state = parse_state_init(sb.items);
	pstate = program_state_init(TARGET_X86_64);

	struct program prog = parse_program(&state);
	print_program(&prog);
	nob_da_foreach(struct ast, ast, &prog) {
		eval(&pstate, ast);
	}
	program_destroy(&prog);

	printf("%s", pstate.x86_64.prelude.items);
	printf("%s", pstate.x86_64.data.items);
	printf("%s", pstate.x86_64.code.items);

	program_state_destroy(&pstate);

	nob_sb_free(sb);

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
