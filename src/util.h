/*
 * src/util.h
 *
 * Utilities like singly-linked list nodes, etc.
 *
 * Copyright (c) 2020 Viktor Garske
 */

#ifndef H_UTIL
#define H_UTIL

/**
 * Singly linked list node
 */
struct sl_list_node {
	void *data;
	struct sl_list_node *next;
};

#endif /* ifndef H_UTIL */
