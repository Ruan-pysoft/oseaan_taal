#if 0
#include "lexer.h"

#define NOB_IMPLEMENTATION
#include "nob.h"

void tokenise_file(const char *filename) {
	printf("\n=== %s ===\n", filename);

	Nob_String_Builder source = {0};
	if (!nob_read_entire_file(filename, &source)) exit(1);
	nob_sb_append_null(&source);

	struct source_tracking lex_state = st_init(filename, source.items);

	Nob_String_Builder sb = {0};

	struct token tok;
	do {
		tok = lex_token(&lex_state);

		print_token(&tok);
		putchar('\n');

		token_destroy(&tok);
	} while (tok.type != TOK_EOF);

	nob_sb_free(sb);
	nob_sb_free(source);
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	tokenise_file("examples/hallo_wereld.os");
	tokenise_file("examples/basiese_funksies.os");
	tokenise_file("examples/fib.os");
	tokenise_file("examples/wisselvorme.os");

}
#else
#include "lexer.h"
#include "parser.h"

#define NOB_IMPLEMENTATION
#include "nob.h"

void print_program_ast(const char *filename) {
	printf("\n=== %s ===\n", filename);

	Nob_String_Builder source = {0};
	if (!nob_read_entire_file(filename, &source)) exit(1);
	nob_sb_append_null(&source);

	struct program prog = parse_file(st_init(filename, source.items));

	print_program(&prog);

	program_destroy(&prog);
	nob_sb_free(source);
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	print_program_ast("examples/hallo_wereld.os");
	print_program_ast("examples/basiese_funksies.os");
	//print_program_ast("examples/fib.os");
	//print_program_ast("examples/wisselvorme.os");

	struct konkrete_tipe druk_nommer_tipe = {
		.tipe_vlh = v_konstant,
		.tipe = {
			.klas = kvt_funksie,
			.funksie = {
				.terugkeer = calloc(1, sizeof(struct konkrete_tipe)),
				.argumente = {
					.items = NULL,
					.count = 0,
					.capacity = 0,
				},
			},
		},
	};
	*druk_nommer_tipe.tipe.funksie.terugkeer = (struct konkrete_tipe) {
		.tipe_vlh = v_veranderlik,
		.tipe = {
			.klas = kvt_basies,
			.basies = bt_niks,
		},
	};
	nob_da_append(&druk_nommer_tipe.tipe.funksie.argumente, ((struct konkrete_tipe) {
		.tipe_vlh = v_konstant,
		.tipe = {
			.klas = kvt_basies,
			.basies = bt_heel64,
		}
	}));

	struct konkrete_tipe tel_by_tipe = {
		.tipe_vlh = v_konstant,
		.tipe = {
			.klas = kvt_funksie,
			.funksie = {
				.terugkeer = calloc(1, sizeof(struct konkrete_tipe)),
				.argumente = {0},
			},
		},
	};
	*tel_by_tipe.tipe.funksie.terugkeer = (struct konkrete_tipe) {
		.tipe_vlh = v_veranderlik,
		.tipe = {
			.klas = kvt_basies,
			.basies = bt_heel64,
		},
	};
	nob_da_append(&tel_by_tipe.tipe.funksie.argumente, ((struct konkrete_tipe) {
		.tipe_vlh = v_konstant,
		.tipe = {
			.klas = kvt_basies,
			.basies = bt_heel64,
		}
	}));
	nob_da_append(&tel_by_tipe.tipe.funksie.argumente, ((struct konkrete_tipe) {
		.tipe_vlh = v_konstant,
		.tipe = {
			.klas = kvt_basies,
			.basies = bt_heel64,
		}
	}));

	struct konkrete_tipe hoof_tipe = {
		.tipe_vlh = v_konstant,
		.tipe = {
			.klas = kvt_funksie,
			.funksie = {
				.terugkeer = calloc(1, sizeof(struct konkrete_tipe)),
				.argumente = {0},
			},
		},
	};
	*hoof_tipe.tipe.funksie.terugkeer = (struct konkrete_tipe) {
		.tipe_vlh = v_veranderlik,
		.tipe = {
			.klas = kvt_basies,
			.basies = bt_heel64,
		},
	};

	printf("Die tipe van die druk_nommer funksie is: ");
	print_konkrete_tipe(&druk_nommer_tipe);
	putchar('\n');
	printf("Die tipe van die tel_by funksie is: ");
	print_konkrete_tipe(&tel_by_tipe);
	putchar('\n');
	printf("Die tipe van die hoof funksie is: ");
	print_konkrete_tipe(&hoof_tipe);
	putchar('\n');

	konkrete_tipe_destroy(&hoof_tipe);
	konkrete_tipe_destroy(&tel_by_tipe);
	konkrete_tipe_destroy(&druk_nommer_tipe);
}
#endif
