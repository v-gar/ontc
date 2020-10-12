/*
 * include/oxpl/parse.h
 *
 * Parser interface
 *
 * This is primarily used for connecting the parser and lexer.
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef H_PARSE
#define H_PARSE

/**
 * \file parse.h
 * \brief Bison-agnostic interface for the parser
 */

enum keywords {
	K_CLS,
	K_INST,
};

#endif /* ifndef H_PARSE */
