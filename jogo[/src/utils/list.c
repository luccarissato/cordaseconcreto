#include "list.h"
#include <stdlib.h>

void initList(List* list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void pushBack(List* list, void* data) {
    ListNode* newNode = malloc(sizeof(ListNode));

    newNode->data = data;

    if (list->head == NULL) {
        newNode->next = newNode;
        newNode->prev = newNode;
        list->head = newNode;
        list->tail = newNode;
    }

    else {
        newNode->next = list->head;
        newNode->prev = list->tail;
        list->tail->next = newNode;
        list->head->prev = newNode;
        list->tail = newNode;
    }

    list->size++;
}

void removeNode(List* list, ListNode* node) {

    if (list->head == NULL || node == NULL)
        return;

    if (list->size == 1) {
        list->head = NULL;
        list->tail = NULL;
    }

    else {
        node->prev->next = node->next;
        node->next->prev = node->prev;

        if (node == list->head) {
            list->head = node->next;
        }

        if (node == list->tail) {
            list->tail = node->prev;
        }
    }

    free(node);
    list->size--;
}

void clearList(List* list) {
    if (list->head == NULL)
        return;

    ListNode* current =
        list->head->next;

    while (current != list->head) {
        ListNode* next =
            current->next;

        free(current);
        current = next;
    }

    free(list->head);

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

