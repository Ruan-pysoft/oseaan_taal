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
	struct funksie *items;
	size_t count;
	size_t capacity;

	struct oseaan_value last_val;
};

union target {
	struct target_x86_64 x86_64;
	struct target_interpret interpret;
};

struct program_state {
	struct function_signature *items;
	size_t count;
	size_t capacity;

	enum target_type type;
	union target target;

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
			res.target.x86_64 = (struct target_x86_64) {
				.prelude = {0},
				.data = {0},
				.code = {0},
			};
			nob_sb_append_cstr(&res.target.x86_64.prelude,
				"global _start\n"
				"SYS_WRITE equ 1\n"
				"SYS_EXIT  equ 60\n"
				"STDOUT    equ 1\n"
			);
			nob_sb_append_cstr(&res.target.x86_64.data,
				"SECTION .data\n"
			);
			nob_sb_append_cstr(&res.target.x86_64.code,
				"SECTION .text\n"
				"druk_lyn:\n"
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
		} break;
		case TARGET_INTERPRET: {
		} break;
	}

	return res;
}
#define VERB(str) if (state->verbose) nob_log(NOB_INFO, str)
#define VERBF(fmt, ...) if (state->verbose) nob_log(NOB_INFO, fmt, __VA_ARGS__)

void eval_expr(struct program_state *state, struct expr expr);
void visit_eof(struct program_state *state) {
	VERB("Visiting EOF");
	switch (state->type) {
		case TARGET_X86_64: {
			Nob_String_Builder *sb = &state->target.x86_64.code;

			nob_sb_append_cstr(sb,
				"_start:\n"
				"  call hoof\n"
				"  ; syscall(SYS_EXIT, 0)\n"
				"  mov rax, SYS_EXIT\n"
				"  xor rdi, rdi\n"
				"  syscall\n"
			);
		} break;
		case TARGET_INTERPRET: {
			for (size_t i = 0; i < state->target.interpret.count; ++i) {
				VERBF("Checking function %s for main...", state->target.interpret.items[i].naam);
				if (strcmp(state->target.interpret.items[i].naam, "hoof") == 0) {
					eval_expr(state, state->target.interpret.items[i].lyf);
					printf("Program result (should be niks): ");
					print_value(state->target.interpret.last_val);
					putchar('\n');
					value_destroy(state->target.interpret.last_val);
					break;
				}
				nob_log(NOB_ERROR, "No main function provided!");
			}
		} break;
	}
}
void visit_funcall(struct program_state *state, struct funcall funcall) {
	VERB("Visiting funcall");
	switch (state->type) {
		case TARGET_X86_64: {
			// only support up to six integer arguments for now
			Nob_String_Builder *sb = &state->target.x86_64.code;
			int args_pushed = 0;
			for (size_t i = 0; i < funcall.count; ++i) {
				eval_expr(state, funcall.items[i]);
				switch (state->target.x86_64.items[--state->target.x86_64.count]) {
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
			nob_da_append(&state->target.x86_64, TP_NIKS);
		} break;
		case TARGET_INTERPRET: {
			if (strcmp(funcall.naam, "druk_lyn") == 0) {
				for (size_t i = 0; i < funcall.count; ++i) {
					eval_expr(state, funcall.items[i]);
					print_value(state->target.interpret.last_val);
					value_destroy(state->target.interpret.last_val);
				}
				putchar('\n');
			} else {
				printf("TODO: parse funksie call\n");
				printf("Naam: %s\n", funcall.naam);
			}
			state->target.interpret.last_val = (struct oseaan_value) {0};
		} break;
	}
}
void visit_compound(struct program_state *state, struct compound compound) {
	VERB("Visiting compound");
	switch (state->type) {
		case TARGET_X86_64: {
			Nob_String_Builder *sb = &state->target.x86_64.code;
			for (size_t i = 0; i < compound.count; ++i) {
				if (i) {
					nob_sb_append_cstr(sb, "  pop rax\n");
					--state->target.x86_64.count;
				}
				eval_expr(state, compound.items[i]);
			}
			if (compound.last_empty) {
				nob_sb_append_cstr(sb, "  pop rax\n  push 0\n");
				--state->target.x86_64.count;
				nob_da_append(&state->target.x86_64, TP_NIKS);
			}
		} break;
		case TARGET_INTERPRET: {
			struct oseaan_value res = {0};
			for (size_t i = 0; i < compound.count; ++i) {
				eval_expr(state, compound.items[i]);
				if (state->verbose) {
					printf("In compound expression, got expression value: ");
					print_value(state->target.interpret.last_val);
					putchar('\n');
				}
				if (!compound.last_empty) {
					value_destroy(res);
					res = state->target.interpret.last_val;
				} else {
					value_destroy(state->target.interpret.last_val);
				}
			}
			state->target.interpret.last_val = res;
		} break;
	}
}
void visit_str_lit(struct program_state *state, char *str_lit) {
	VERB("Visiting string literal");
	switch (state->type) {
		case TARGET_X86_64: {
			static size_t strlit_counter = 0;
			Nob_String_Builder *sb = &state->target.x86_64.data;
			nob_sb_appendf(sb,
				"strlit_%zu db \"%s\"\n"
				"strlit_%zu_len equ $-strlit_%zu\n"
			, strlit_counter, str_lit, strlit_counter, strlit_counter);

			sb = &state->target.x86_64.code;
			nob_sb_appendf(sb,
				"  push strlit_%zu\n"
				"  push strlit_%zu_len\n"
			, strlit_counter, strlit_counter);
			nob_da_append(&state->target.x86_64, TP_STR);

			++strlit_counter;
		} break;
		case TARGET_INTERPRET: {
			state->target.interpret.last_val.type = TP_STR;
			state->target.interpret.last_val.value.s_value = strdup(str_lit);
		} break;
	}
}
void visit_funksie_definisie(struct program_state *state, struct funksie funksie) {
	VERB("Visiting funksie definisie");
	switch (state->type) {
		case TARGET_X86_64: {
			Nob_String_Builder *sb = &state->target.x86_64.code;
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
			nob_da_append(&state->target.interpret, funksie_copy(funksie));
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
void eval_insluiting(struct program_state *state, struct insluiting insluiting) {
	(void) state;
	(void) insluiting;
	printf("TODO: parse insluiting\n");
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

	struct parse_state state = {
		.source = sb.items,
		.source_len = sb.count,
		.idx = 0,

		.verbose = true,

		.tok = {0},
		.has_tok = false,
	};
	struct program_state pstate = program_state_init(TARGET_X86_64);
	pstate.verbose = true;

	while (state.idx < state.source_len) {
		struct ast *ast = parse_ast(&state);
		printf("AST: ");
		print_ast(ast);
		putchar('\n');
		eval(&pstate, ast);
		ast_destroy(ast);
		free(ast);
	}

	printf("%s", pstate.target.x86_64.prelude.items);
	printf("%s", pstate.target.x86_64.data.items);
	printf("%s", pstate.target.x86_64.code.items);
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
