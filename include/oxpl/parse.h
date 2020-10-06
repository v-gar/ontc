/*
 * src/parse.h
 *
 * Parser interface
 *
 * This is primarily used for connecting the parser and lexer.
 *
 * Copyright (c) 2020 Viktor Garske
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
