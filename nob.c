#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h/nob.h"

const char *executable = "os";

const char *files[] = {
	"main",
	"parser",
	"types",
};

const char *flags[] = {
	"-Wall",
	"-Wextra",
	"-pedantic",
	"-I./nob.h",
	"-g",
	"-fsanitize=address",
	"-p",
};
Cmd cmd_flags = {
	.items = flags,
	.count = sizeof(flags)/sizeof(*flags),
	.capacity = sizeof(flags)/sizeof(*flags),
};

int main(int argc, char **argv) {
	NOB_GO_REBUILD_URSELF(argc, argv);

	Cmd cmd = {0};
	Procs procs = {0};

	mkdir_if_not_exists("build");

	for (size_t i = 0; i < sizeof(files)/sizeof(*files); ++i) {
		size_t mark = temp_save();
		nob_cc(&cmd);
		cmd_append(&cmd, "-c");
		nob_cc_flags(&cmd);
		cmd_extend(&cmd, &cmd_flags);
		nob_cc_output(&cmd, temp_sprintf("build/%s.o", files[i]));
		nob_cc_inputs(&cmd, temp_sprintf("src/%s.c", files[i]));
		if (!cmd_run(&cmd, .async = &procs)) return 1;
		temp_rewind(mark);
	}
	if (!procs_flush(&procs)) return 1;

	size_t mark = temp_save();
	nob_cc(&cmd);
	nob_cc_flags(&cmd);
	cmd_extend(&cmd, &cmd_flags);
	nob_cc_output(&cmd, temp_sprintf("build/%s", executable));
	for (size_t i = 0; i < sizeof(files)/sizeof(*files); ++i) {
		nob_cc_inputs(&cmd, temp_sprintf("build/%s.o", files[i]));
	}
	if (!cmd_run(&cmd)) return 1;
	temp_rewind(mark);
}
