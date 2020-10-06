/*
 * include/ontg/util.h
 *
 * Utilities like singly-linked list nodes, etc.
 *
 * Copyright (c) 2020 Viktor Garske
 */

#ifndef H_UTIL
#define H_UTIL

/**
 * \file util.h
 * \brief Utilities used by multiple components
 */

/**
 * Singly linked list node
 */
struct sl_list_node {
	void *data;
	struct sl_list_node *next;
};

#endif /* ifndef H_UTIL */
