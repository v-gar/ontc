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

#include "main.h"

void print_help(void)
{
	printf("ontc - ontology toolchain\n");
}

int main(int argc, char *argv[])
{
	print_help();
	return 0;
}
