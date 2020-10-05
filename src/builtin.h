/*
 * src/builtin.h
 *
 * Interface for built-in language elements
 *
 * Copyright (c) 2020 Viktor Garske
 */

#ifndef H_LANG_BUILTIN
#define H_LANG_BUILTIN

#include "ast.h"

void lang_builtin_fn_print(struct ast_node *args);

#endif /* ifndef H_LANG_BUILTIN */
