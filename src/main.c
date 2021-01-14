/*
 * Ontology Toolchain
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * src/main.c
 *
 * This is the main file of the ontc command.
 */

/**
 * \file main.c
 * \brief main file for the ontc command.
 */

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "exec.h"
#include "shell.h"

void print_help(void)
{
	char text[] = "ontc - ontology toolchain\n\n"
		"Available commands:\n"
		"shell\tOpen an interactive KB shell\n"
		"run\tRun an OXPL program\n"
		"dbgon\tDebug ontology of an OXPL program"
		" using interactive KB shell\n";
	printf("%s", text);
}

void start_interpreter(char *filename)
{
	FILE *file = fopen(filename, "r");
	exec_program(file);
	fclose(file);
}

void start_dbgon(char *filename)
{
	FILE *file = fopen(filename, "r");
	debug_ontology(file);
	fclose(file);
}

int main(int argc, char *argv[])
{
	if (argc == 3) {
		if (strcmp("run", argv[1]) == 0)
			start_interpreter(argv[2]);
		else if (strcmp("dbgon", argv[1]) == 0)
			start_dbgon(argv[2]);
	} else if (argc == 2) {
		if (strcmp("shell", argv[1]) == 0)
			start_repl_shell(NULL);
	} else if (argc < 2)
		print_help();
	return 0;
}
