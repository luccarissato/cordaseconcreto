#ifndef LIST_H
#define LIST_H

typedef struct ListNode {
    void* data;
    struct ListNode* next;
    struct ListNode* prev;
} ListNode;

typedef struct {
    ListNode* head;
    ListNode* tail;
    int size;
} List;

void initList(List* list);

void pushBack(
    List* list,
    void* data
);

void removeNode(
    List* list,
    ListNode* node
);

void clearList(
    List* list
);

#endif