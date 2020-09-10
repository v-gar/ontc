/*
 * Ontology Toolchain
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * src/main.c
 *
 * This is the main file of the ontc command.
 */

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "shell.h"

void print_help(void)
{
	printf("ontc - ontology toolchain\n");
}

int main(int argc, char *argv[])
{
	if (argc == 2) {
		if (strcmp("shell", argv[1]) == 0)
			start_repl_shell();
	} else if (argc < 2)
		print_help();
	return 0;
}
