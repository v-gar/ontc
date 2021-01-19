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

#define AST_NODE_SIBL(node) (node->base.sibling)
#define AST_NODE_CHLD(node) (node->base.child)
#define AST_NODE_TYPE(node) \
	(node->base.type)

#define AST_NODE_CHLD1(node) AST_NODE_CHLD(node)
#define AST_NODE_CHLD2(node) AST_NODE_CHLD1(node)->base.sibling
#define AST_NODE_CHLD3(node) AST_NODE_CHLD2(node)->base.sibling

#define AST_NODE_NEXT_SIBL(cur) (cur = AST_NODE_SIBL(cur))
#define AST_NODE_NEXT_CHLD(cur) (cur = AST_NODE_CHLD(cur))

#define AST_NODE_CAST(dest, src, subtype) \
	struct ast_node_ ## subtype *dest = \
	 (struct ast_node_ ## subtype *) src

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
	 * Sequence node.
	 * Contains one or more children to be executed in their
	 * order.
	 *
	 * Sequence nodes have the same structure as ::ANT_CMPD
	 * nodes and are only created via a conversion from
	 * an ::ANT_CMPD node via ::ast_convert_cmpd_seq().
	 *
	 * Sequence nodes occur as a child of ::ANT_FUNC,
	 * ::ANT_WHILE, ::ANT_FOR, etc. when they act as
	 * the body of that element (e.g. in a for loop).
	 *
	 * Sequence nodes DO NOT occur as sub-blocks in
	 * statements - this is the sole purpose of ::ANT_CMPD.
	 */
	ANT_SEQ,

	/**
	 * Compound statement node.
	 * Contains one or more children to be executed in their
	 * order.
	 *
	 * Compound nodes occur in imperative blocks and have
	 * their own scope (for variables, not to be confused
	 * ::ANT_SCOPE which is part of an ::ANT_ADDR)
	 */
	ANT_CMPD,

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
	ANT_ADDR,

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
	 * Less-than node.
	 */
	ANT_LT,

	/**
	 * Greater-than node.
	 */
	ANT_GT,

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
	 * Post increment node.
	 */
	ANT_POSTINC,

	/**
	 * Post decrement node.
	 */
	ANT_POSTDEC,

	/**
	 * Prefix increment node.
	 */
	ANT_PREINC,

	/**
	 * Prefix decrement node.
	 */
	ANT_PREDEC,

	/**
	 * Negative sign operator node.
	 */
	ANT_NEGSIGN,

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
	 * Variable signature
	 *
	 * Contains:
	 *
	 * 1. identifier
	 * 2. scope
	 */
	ANT_SIGVAR,

	/**
	 * Facts in the style of first-order logic.
	 *
	 * 1. relation (::ANT_SCOPE)
	 * 2. head of resources linked list
	 */
	ANT_FACT,

	/**
	 * Triple fact for the ontology.
	 *
	 * Triple facts have three children:
	 *
	 * 1. the relation / predicate (::ANT_SCOPE)
	 * 2. the subject (::ANT_ADDR)
	 * 3. the object (::ANT_SCOPE)
	 */
	ANT_TFACT,

	/**
	 * Conditional.
	 *
	 * Contains three parts:
	 *
	 * 1. condition
	 * 2. then-branch (if condition is true)
	 * 3. else-branch (if condition is false)
	 */
	ANT_COND,

	/**
	 * Conditional using a ternary operator.
	 *
	 * Contains three children:
	 *
	 * 1. condition
	 * 2. then-expr
	 * 3. else-expr
	 */
	ANT_CTERN,

	/**
	 * Return statement.
	 *
	 * Optional child: statement
	 */
	ANT_RET,

	/**
	 * Continue statement
	 */
	ANT_CONT,

	/**
	 * Break statement
	 */
	ANT_BREAK,

	/**
	 * While loop.
	 *
	 * Contains:
	 *
	 * 1. condition
	 * 2. imperative block
	 */
	ANT_WHILE,

	/**
	 * For loop
	 *
	 * Contains:
	 *
	 * 1. identifier
	 * 2. iterable
	 * 3. imperative block
	 */
	ANT_FOR,

	/**
	 * Variable declaration node.
	 *
	 * 1. variable siganture (::ANT_SIGVAR)
	 * 2. value (optiona)
	 */
	ANT_VARDECL,

	/**
	 * Class node.
	 *
	 * 1. identifier (::ANT_STR)
	 * 2. specification (::ANT_CSPEC)
	 */
	ANT_CLASS,

	/**
	 * Class specification node.
	 *
	 * This node contains all facts and functions
	 * as its children.
	 */
	ANT_CSPEC
};

/**
 * \brief Base node of the abstract syntax tree (AST).
 *
 * The AST is implemented as a left-child, right-sibling tree, thus it
 * contains a pointer to a child node and a sibling node.
 */
struct ast_node_base {
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
 * \brief AST node without values.
 */
struct ast_node {
	struct ast_node_base base;
};

/**
 * \brief Value node of the AST carrying an integer value.
 */
struct ast_node_int {
	struct ast_node_base base;

	int value;
};

/**
 * \brief Value node of the AST carrying a float value.
 */
struct ast_node_float {
	struct ast_node_base base;

	float value;
};

/**
 * \brief Value node of the AST carrying a string.
 */
struct ast_node_str {
	struct ast_node_base base;

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
 * Create a new ::ANT_CMPD node and add the head element.
 * More elements can be added using ::ast_add_seq().
 *
 * To create an ::ANT_SEQ node, create a ::ANT_CMPD node
 * and then convert it using ::ast_convert_cmpd_seq().
 */
struct ast_node *ast_new_cmpd(struct ast_node *head);

/**
 * Converts an ::ANT_CMPD node to an ::ANT_SEQ node.
 *
 * For more information see ::ANT_SEQ.
 *
 * This is possible because they share their structure.
 *
 * \param cmpd_node AST node with type ::ANT_CMPD or NULL
 *                  (if compound statement is empty)
 * \result AST node with type ::ANT_SEQ or NULL
 */
struct ast_node *ast_convert_cmpd_seq(struct ast_node *const cmpd_node);

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
 * Create new unary operation.
 *
 * \param affix 0 = prefix, 1 = postfix
 * \param oper Operator like "+", "-", "++"
 * \param operand Operand
 */
struct ast_node *ast_new_unop(char affix, char *oper,
		struct ast_node *operand);

/**
 * Create new signature node.
 *
 * \param name ::ANT_STR node with the name identifier
 */
struct ast_node *ast_new_sig(struct ast_node *name);

/**
 * Create new variable signature
 *
 * \param identifier IDENTIFIER
 * \param type scope or NULL
 */
struct ast_node *ast_new_sigvar(struct ast_node *identifier,
		struct ast_node *type);


/**
 * Create new function node.
 *
 * \param sig ::ANT_SIG node
 * \param block node sequence (same level)
 */
struct ast_node *ast_new_func(struct ast_node *sig,
		struct ast_node *block);

/**
 * Create new address node.
 *
 * \param scope ::ANT_SCOPE node
 * \param param ::ANT_STR node
 */
struct ast_node *ast_new_address(struct ast_node *scope,
		struct ast_node *param);

/**
 * Create new FOL fact
 */
struct ast_node *ast_new_fact(struct ast_node *rel,
		struct ast_node *args);

/**
 * Create new triple fact.
 */
struct ast_node *ast_new_tfact(struct ast_node *subj,
		struct ast_node *rel, struct ast_node *obj);

/**
 * Create new conditional (if statement).
 *
 * \param cond any type of expression node starting at ::ANT_CONT
 * \param then then block
 * \param else_ else block
 */
struct ast_node *ast_new_cond(struct ast_node *cond,
		struct ast_node *then, struct ast_node *else_);

/**
 * Create new conditional (ternary statement).
 */
struct ast_node *ast_new_ctern(struct ast_node *cond,
		struct ast_node *then, struct ast_node *else_);

/**
 * Create new return statement.
 *
 * \param expr Optional expression
 */
struct ast_node *ast_new_ret(struct ast_node *expr);

/**
 * Create new jump statement other than return.
 *
 * \param type 'b' for break, 'c' for continue
 */
struct ast_node *ast_new_jump(char type);

/**
 * Create new while loop.
 */
struct ast_node *ast_new_while(struct ast_node *condition,
		struct ast_node *block);

/**
 * Create new for loop.
 */
struct ast_node *ast_new_for(struct ast_node *identifier,
		struct ast_node *iterable,
		struct ast_node *block);

/**
 * Create new variable declaration.
 *
 * \param sigvar variable signature (::ANT_SIGVAR)
 * \param val expression or NULL
 */
struct ast_node *ast_new_vardecl(struct ast_node *sigvar,
		struct ast_node *val);

/**
 * Create new class node (::ANT_CLASS)
 */
struct ast_node *ast_new_class(struct ast_node *identifier,
		struct ast_node *spec);

/**
 * Create new class specification node (::ANT_CSPEC).
 * An ::ANT_CSPEC is a sequential list so it will be
 * populated using ::ast_add_seq.
 */
struct ast_node *ast_new_cspec(struct ast_node *head);

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

/**
 * Free the AST.
 *
 * \param root ::ANT_TRANSUNIT node
 */
void ast_free(struct ast_node *root);

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
