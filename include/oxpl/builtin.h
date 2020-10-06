/*
 * include/oxpl/builtin.h
 *
 * Interface for built-in language elements
 *
 * Copyright (c) 2020 Viktor Garske
 */

#ifndef H_LANG_BUILTIN
#define H_LANG_BUILTIN

/**
 * \file builtin.h
 * \brief Interface for built-in language elements
 */

#include "ast.h"

void lang_builtin_fn_print(struct ast_node *args);
void lang_builtin_fn_println(struct ast_node *args);

#endif /* ifndef H_LANG_BUILTIN */
