#include <stdio.h>
#include <stdlib.h>

#define MAX_NODES 100

struct node  {
    char value;
    struct node* next;
}__attribute__ ((aligned(8)));

typedef struct _AllocatedNodes {
    int count;
    struct node *nodes[MAX_NODES];
}AllocatedNodes;
static AllocatedNodes allocatedNodes; 

void registerNode(void* node)
{
    allocatedNodes.nodes[allocatedNodes.count] = node;
    allocatedNodes.count++;
}

void forceUnregisterAllNodes(void)
{
    int i;
    for (i=0;i<allocatedNodes.count;i++) {
        free((struct node*)allocatedNodes.nodes[i]);
        allocatedNodes.nodes[i] = NULL;
    }
}
struct node* unregisterLastNode(void)
{
    allocatedNodes.count--;
    free((struct node*)allocatedNodes.nodes[allocatedNodes.count]);
    allocatedNodes.nodes[allocatedNodes.count] = NULL;
    return (allocatedNodes.count>=0)?(struct node*)allocatedNodes.nodes[allocatedNodes.count-1]:NULL;
}

void insertNode(struct node* listStart, struct node* node)
{
    struct node* temporaryNode;
    struct node* seekNode;

    seekNode = listStart;

    while(NULL != seekNode->next) {
        temporaryNode = seekNode;
        seekNode = temporaryNode->next;
    }
    seekNode->next = node;
}

struct node* nextNode(struct node* node)
{
    return (NULL == node->next)?node:node->next;
}

struct node* getLastNode(struct node* listStart)
{
    struct node* seekNode = listStart; 
    struct node* temporaryNode = NULL; 

    do {
        temporaryNode = seekNode;
        seekNode = nextNode(seekNode);
    } while( seekNode != temporaryNode); 
    return seekNode;
}    

struct node* createNode(char value)
{
    struct node* temporaryNode = malloc(sizeof(struct node));
    temporaryNode->value = value;
    temporaryNode->next = NULL;
    registerNode((void*)temporaryNode);
    return temporaryNode; 
}

void destructLastNode(struct node* listStart)
{
    struct node* lastNode; 
    lastNode = unregisterLastNode();
    if(NULL != lastNode) lastNode->next = NULL;
}

void destructList(struct node* listStart)
{
    struct node* seekNode = NULL;

    do { 
        seekNode = getLastNode(listStart);
        destructLastNode(listStart);
    } while( seekNode != listStart);
}

int main(int argc, char *argv[])
{
    struct node node_1 = {'a', NULL};
    struct node node_2 = {'b', NULL};
    struct node node_3 = {'c', NULL};
    struct node node_4 = {'d', NULL};
    struct node node_5 = {'e', NULL};
    struct node* temporaryNode = NULL;
    struct node* iteratorNode = NULL;
    int count = 1;
    struct node*  dynamicNode = NULL;

    printf("Static Linked List funcitons\n");
    insertNode(&node_1, &node_2);
    insertNode(&node_1, &node_3);
    insertNode(&node_1, &node_4);
    insertNode(&node_1, &node_5);

    iteratorNode = &node_1; 
    do {
        temporaryNode = iteratorNode;
        iteratorNode = nextNode(temporaryNode);
        printf("node[%d].value = %c\n", count, temporaryNode->value);
        count++;
    } while(temporaryNode != iteratorNode); 

    printf("Dynamic Linked List functions\n");

    dynamicNode = createNode('1');
    printf("dynamicList at %p\n", dynamicNode);
    printf("dynamicNode.value: %c\n", dynamicNode->value);

    dynamicNode->next = createNode('2');
    temporaryNode = getLastNode(dynamicNode);
    printf("temporaryNode at %p\n", temporaryNode);
    printf("lastNode.value: %c\n", temporaryNode->value);

    temporaryNode->next = createNode('3');
    temporaryNode = getLastNode(dynamicNode);
    printf("temporaryNode at %p\n", temporaryNode);
    printf("lastNode.value: %c\n", temporaryNode->value);

    temporaryNode->next = createNode('4');
    temporaryNode = getLastNode(dynamicNode);
    printf("temporaryNode at %p\n", temporaryNode);
    printf("lastNode.value: %c\n", temporaryNode->value);

    destructLastNode(dynamicNode);
    temporaryNode = getLastNode(dynamicNode);
    printf("temporaryNode at %p\n", temporaryNode);
    printf("lastNode.value: %c\n", temporaryNode->value);

    printf("total of nodes %d\n", allocatedNodes.count);
    destructList(dynamicNode); //frees the linked list
    printf("total of nodes %d\n", allocatedNodes.count);
    forceUnregisterAllNodes();//frees all the nodes allocated
}
