#include "parser.h"
#include "types.h"

#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	/*
	Nob_String_Builder sb = {0};

	if (!nob_read_entire_file("examples/hallo_wereld.os", &sb)) exit(1);

	nob_sb_append_null(&sb);
	struct parse_state state = parse_state_init(sb.items);

	nob_sb_free(sb);
	*/

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
