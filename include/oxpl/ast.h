/*
 * include/oxpl/ast.h
 *
 * Abstract Syntax Tree
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef H_AST
#define H_AST

/**
 * \file ast.h
 * \brief Interface for the abstract syntax tree
 */

#include <stddef.h>

/**
 * \brief Types of abstract syntax tree nodes
 */
enum ast_node_type {
	/**
	 * Undefined node type.
	 * Can be used in order to initialize a node
	 * set the type later.
	 */
	ANT_UNDEFINED,

	/**
	 * Translation unit.
	 * This is the root of the AST and should
	 * appear at least one time.
	 */
	ANT_TRANSUNIT,

	/* == SPECIAL NODES == */

	/**
	 * Integer node.
	 * Nodes with this type are ast_node_int
	 * and can be casted to this struct as they
	 * hold a special value: the integer.
	 */
	ANT_INT,

	/**
	 * Float node.
	 * Nodes with this type are ast_node_float
	 * and can be casted to this struct as they
	 * hold a special value:
	 * the floating point number.
	 */
	ANT_FLOAT,

	/**
	 * String node.
	 * Nodes with this type are ast_node_str
	 * and can be casted to this struct as they
	 * hold a special value: the pointer to the string.
	 */
	ANT_STR,

	/**
	 * Scope node.
	 * Contains everything with a scope.
	 */
	ANT_SCOPE,

	/**
	 * Address node.
	 *
	 * An address node is an extended ::ANT_SCOPE
	 * as you can address mores resources, even
	 * function parameters.
	 *
	 * The left child points an ::ANT_SCOPE,
	 * the right child to an ::ANT_STR.
	 */
	ANT_ADDRESS,

	/**
	 * Call node.
	 * Contains function calls.
	 *
	 * It has one one more children:
	 *
	 * 1. a pointer to the function/class name, i.e. an ANT_SCOPE
	 * 2. the args
	 */
	ANT_CALL,

	/**
	 * Incremental node.
	 * Increment or decrement the value.
	 */
	ANT_INC,

	/* == Operations == */

	/**
	 * Arithmetic binary addition node.
	 */
	ANT_BADD,

	/**
	 * Arithmetic binary subtraction node.
	 */
	ANT_BSUB,

	/**
	 * Arithmetic multiplication node.
	 */
	ANT_MUL,

	/**
	 * Arithmetic division node.
	 */
	ANT_DIV,

	/**
	 * Arithmetic modulo node.
	 */
	ANT_MOD,

	/**
	 * Assignment node.
	 */
	ANT_ASSIGN,

	/**
	 * Equality operator node.
	 */
	ANT_EQ,

	/**
	 * Negative equality node.
	 */
	ANT_NEQ,

	/**
	 * Logical-and node.
	 */
	ANT_LAND,

	/**
	 * Logical-or node.
	 */
	ANT_LOR,

	/**
	 * Bitwise-and node.
	 */
	ANT_BAND,

	/**
	 * (Bitwise) inclusive-or node.
	 */
	ANT_BOR,

	/**
	 * Exclusive-or node.
	 */
	ANT_XOR,

	/**
	 * Less-equal node.
	 */
	ANT_LEQ,

	/**
	 * Greater-equal node.
	 */
	ANT_GEQ,

	/**
	 * Shift-left node.
	 */
	ANT_SHIFTL,

	/**
	 * Shift-right node.
	 */
	ANT_SHIFTR,

	/**
	 * Function node.
	 *
	 * A function node has two children:
	 *
	 * 1. the first node points to a ANT_SIG node
	 * 2. the second node points to the first imperative node
	 */
	ANT_FUNC,

	/**
	 * Function signature node.
	 *
	 * The function siganture node contains one child:
	 * the name of the function which is an ANT_STR.
	 */
	ANT_SIG,

	/**
	 * Triple fact for the ontology.
	 *
	 * Triple facts have three children:
	 *
	 * 1. the relation / predicate (::ANT_SCOPE)
	 * 2. the subject (::ANT_ADDR)
	 * 3. the object (::ANT_SCOPE)
	 */
	ANT_TFACT
};

/**
 * \brief Basic node of the abstract syntax tree (AST).
 *
 * The AST is implemented as a left-child, right-sibling tree, thus it
 * contains a pointer to a child node and a sibling node.
 */
struct ast_node {
	/**
	 * Type of the AST node
	 */
	enum ast_node_type type;

	/**
	 * Pointer to the next child node.
	 * One level down descending the tree.
	 */
	struct ast_node *child;

	/**
	 * Pointer to the next sibling node.
	 * Same level.
	 */
	struct ast_node *sibling;
};

/**
 * \brief Special node of the AST carrying an integer value.
 */
struct ast_node_int {
	enum ast_node_type type;
	struct ast_node *child;
	struct ast_node *sibling;

	int value;
};

/**
 * \brief Special node of the AST carrying a float value.
 */
struct ast_node_float {
	enum ast_node_type type;
	struct ast_node *child;
	struct ast_node *sibling;

	float value;
};

/**
 * \brief Special node of the AST carrying a string.
 */
struct ast_node_str {
	enum ast_node_type type;
	struct ast_node *child;
	struct ast_node *sibling;

	char *value;
};

/**
 * Create a new ::ANT_INT node.
 */
struct ast_node *ast_new_int(int value);

/**
 * Create a new ::ANT_FLOAT node.
 */
struct ast_node *ast_new_float(float value);

/**
 * Create a new ::ANT_STR node.
 */
struct ast_node *ast_new_str(char *value);

/**
 * Create a new ::ANT_SCOPE node with one namespace element.
 * More namespaces element can be added using ::ast_scope_add().
 */
struct ast_node *ast_new_scope(struct ast_node *value);


/**
 * Add new ::ANT_CALL node.
 */
struct ast_node *ast_new_call(struct ast_node *callee,
		struct ast_node *arglist);

/**
 * Create new binary operation node.
 *
 * \param oper Operator like '+', '-', '*', '/', '%'
 * \param operand1 LHS operand
 * \param operand2 RHS operand
 */
struct ast_node *ast_new_binop(char oper, struct ast_node *operand1,
		struct ast_node *operand2);

/**
 * Create new binary operation node but for operators with
 * more than one character.
 *
 * This will remap the operator and call ::ast_new_binop().
 *
 * \param oper Operator like "<<", ">>", "&&", "==", ...
 * \param operand1 LHS operand
 * \param operand2 RHS operand
 */
struct ast_node *ast_new_binop_s(char *oper_s, struct ast_node *operand1,
		struct ast_node *operand2);

/**
 * Create new signature node.
 */
struct ast_node *ast_new_sig(struct ast_node *name);

/**
 * Create new function node.
 */
struct ast_node *ast_new_func(struct ast_node *sig,
		struct ast_node *block);

/**
 * Create new address node.
 */
struct ast_node *ast_new_address(struct ast_node *scope,
		struct ast_node *param);

/**
 * Create new triple fact.
 */
struct ast_node *ast_new_tfact(struct ast_node *subj,
		struct ast_node *rel, struct ast_node *obj);

/**
 * Create new translation unit node (::ANT_TRANSUNIT).
 * The node should occur at least once in an AST.
 */
struct ast_node *ast_new_transunit(struct ast_node *first);

/**
 * Add new sibling to an existing node.
 */
struct ast_node *ast_add_seq(struct ast_node *node,
		struct ast_node *successor);

/**
 * Add a new scope to an existing ::ANT_SCOPE node.
 */
struct ast_node *ast_scope_add(struct ast_node *node,
		struct ast_node *successor);

/**
 * Validate an AST.
 *
 * \param root ::ANT_TRANSUNIT node
 * \return 0 if valid or 1 if not
 */
int ast_validate(struct ast_node *root);

/* debugging tools */
/**
 * Print AST for debugging purposes to stdout.
 *
 * AST should be validated using ::ast_validate()
 * beforehand.
 *
 * \param root ::ANT_TRANSUNIT node
 */
void ast_print(struct ast_node *root);

/**
 * Print DOT graphs of the AST to stdout.
 *
 * \param root ::ANT_TRANSUNIT node
 */
void ast_print_dot(struct ast_node *root);

#endif /* ifndef H_AST */
