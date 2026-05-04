#include <stdio.h>
#include <stdlib.h>

#define MAX_NODES 100

struct node {
    char value;
    struct node* next;
} __attribute__ ((aligned(8)));

typedef struct AllocatedNodes {
    int count;
    struct node *nodes[MAX_NODES];
} AllocatedNodes;
static AllocatedNodes allocatedNodes;

/* --- Registry: tracks all heap-allocated nodes as a safety net --- */

/* Returns 0 on success, -1 if the registry is full. */
int registerNode(struct node* node)
{
    if (allocatedNodes.count >= MAX_NODES) {
        fprintf(stderr, "registerNode: registry full, cannot track node\n");
        return -1;
    }
    allocatedNodes.nodes[allocatedNodes.count] = node;
    allocatedNodes.count++;
    return 0;
}

/* Search the registry for this specific pointer, free it, and compact the array. */
void unregisterNode(struct node* node)
{
    int i;
    for (i = 0; i < allocatedNodes.count; i++) {
        if (allocatedNodes.nodes[i] == node) {
            free(node);
            allocatedNodes.count--;
            /* Fill the gap with the last entry to keep the array packed. */
            allocatedNodes.nodes[i] = allocatedNodes.nodes[allocatedNodes.count];
            allocatedNodes.nodes[allocatedNodes.count] = NULL;
            return;
        }
    }
    fprintf(stderr, "unregisterNode: node not found in registry\n");
}

/* Free every tracked node regardless of list structure — last-resort cleanup. */
void forceUnregisterAllNodes(void)
{
    int i;
    for (i = 0; i < allocatedNodes.count; i++) {
        free(allocatedNodes.nodes[i]);
        allocatedNodes.nodes[i] = NULL;
    }
    allocatedNodes.count = 0;
}

/* --- List operations --- */

void insertNode(struct node* listStart, struct node* node)
{
    struct node* seekNode = listStart;
    while (seekNode->next != NULL)
        seekNode = seekNode->next;
    seekNode->next = node;
}

struct node* nextNode(struct node* node)
{
    return node->next;
}

struct node* getLastNode(struct node* listStart)
{
    struct node* seekNode = listStart;
    while (seekNode->next != NULL)
        seekNode = seekNode->next;
    return seekNode;
}

struct node* createNode(char value)
{
    struct node* newNode = malloc(sizeof(struct node));
    if (newNode == NULL) {
        fprintf(stderr, "createNode: malloc failed\n");
        return NULL;
    }
    newNode->value = value;
    newNode->next = NULL;
    if (registerNode(newNode) != 0) {
        free(newNode);
        return NULL;
    }
    return newNode;
}

/*
 * Walk the list to find the actual last node (and its predecessor),
 * then unlink and free it via the registry.
 * Takes a double pointer so it can null out the caller's head when
 * the only remaining node is freed.
 */
void destructLastNode(struct node** listStart)
{
    if (listStart == NULL || *listStart == NULL)
        return;

    struct node* seekNode = *listStart;
    struct node* prev = NULL;

    while (seekNode->next != NULL) {
        prev = seekNode;
        seekNode = seekNode->next;
    }

    if (prev != NULL)
        prev->next = NULL;
    else
        *listStart = NULL;

    unregisterNode(seekNode);
}

/* O(n) forward traversal — no repeated scans to find the tail. */
void destructList(struct node* listStart)
{
    struct node* current = listStart;
    struct node* next;

    while (current != NULL) {
        next = current->next;
        unregisterNode(current);
        current = next;
    }
}

int main(void)
{
    struct node node_1 = {'a', NULL};
    struct node node_2 = {'b', NULL};
    struct node node_3 = {'c', NULL};
    struct node node_4 = {'d', NULL};
    struct node node_5 = {'e', NULL};
    struct node* temporaryNode = NULL;
    struct node* iteratorNode = NULL;
    int count = 1;
    struct node* dynamicNode = NULL;

    printf("Static Linked List functions\n");
    insertNode(&node_1, &node_2);
    insertNode(&node_1, &node_3);
    insertNode(&node_1, &node_4);
    insertNode(&node_1, &node_5);

    iteratorNode = &node_1;
    while (iteratorNode != NULL) {
        printf("node[%d].value = %c\n", count, iteratorNode->value);
        count++;
        iteratorNode = nextNode(iteratorNode);
    }

    printf("Dynamic Linked List functions\n");

    dynamicNode = createNode('1');
    if (dynamicNode == NULL) return 1;
    printf("dynamicList at %p\n", (void*)dynamicNode);
    printf("dynamicNode.value: %c\n", dynamicNode->value);

    temporaryNode = createNode('2');
    if (temporaryNode == NULL) return 1;
    dynamicNode->next = temporaryNode;
    temporaryNode = getLastNode(dynamicNode);
    printf("temporaryNode at %p\n", (void*)temporaryNode);
    printf("lastNode.value: %c\n", temporaryNode->value);

    temporaryNode->next = createNode('3');
    if (temporaryNode->next == NULL) return 1;
    temporaryNode = getLastNode(dynamicNode);
    printf("temporaryNode at %p\n", (void*)temporaryNode);
    printf("lastNode.value: %c\n", temporaryNode->value);

    temporaryNode->next = createNode('4');
    if (temporaryNode->next == NULL) return 1;
    temporaryNode = getLastNode(dynamicNode);
    printf("temporaryNode at %p\n", (void*)temporaryNode);
    printf("lastNode.value: %c\n", temporaryNode->value);

    destructLastNode(&dynamicNode);
    temporaryNode = getLastNode(dynamicNode);
    printf("temporaryNode at %p\n", (void*)temporaryNode);
    printf("lastNode.value: %c\n", temporaryNode->value);

    printf("total of nodes %d\n", allocatedNodes.count);
    destructList(dynamicNode);
    printf("total of nodes %d\n", allocatedNodes.count);
    forceUnregisterAllNodes(); /* no-op here, but safe to call as a final guard */

    return 0;
}
