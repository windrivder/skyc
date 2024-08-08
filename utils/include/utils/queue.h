/*
 * Copyright (c) 2020-2024 Siddharth Chandrasekaran <sidcha.dev@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _UTILS_QUEUE_H_
#define _UTILS_QUEUE_H_

#include <utils/list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define queue_node_t node_t

typedef struct {
	list_t list;
} queue_t;

void queue_init(queue_t *queue);
void queue_enqueue(queue_t *queue, queue_node_t *node);
int queue_dequeue(queue_t *queue, queue_node_t **node);
int queue_peek_last(queue_t *queue, queue_node_t **node);
int queue_peek_first(queue_t *queue, queue_node_t **node);

#ifdef __cplusplus
}
#endif

#endif /* _UTILS_QUEUE_H_ */
