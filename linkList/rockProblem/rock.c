#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct ele_t{
    char element;
    int count;
    struct ele_t *next;
}ele;
int printstr(char **str, int n);
void printNode(ele *head);

void inCnt(ele *head, char elem) {
    ele *node = head;
    printf("element to increase count: %c\n", elem);
    while(node != NULL) {
        printf("element in node (inCnt()): %c\n", node->element);
        if(node->element == elem) {
            printf("match \n");
            printf("element count : %d\n", node->count);
            printf("increase the count.");
            node->count++;
            printf("New element count : %d\n", node->count);
            return;
        }
        printf("not match. Look for next node\n");
        node = node->next;
    }

    printf("Element not present in the node. Add the node\n");
}

int getEle(ele *head, char elem) {
    ele *node = head;

    printf("element to get: %c\n", elem);
    
    while(node != NULL) {
        printf("element in node(getEle()) : %c\n", node->element);
        if(node->element == elem) {
            printf("match \n");
            printf("element count : %d\n", node->count);
            return node->count;
        }
        printf("not match. Look for next node\n");
        
        node = node->next;
    }

    printf("Element not present in the node. Add the node\n");

    return 0;
}

void addEle(ele *head, char elem) {    
    if(head == NULL)
        return;
    ele *nodeP = head;
    
    printf("element to add : %c\n", elem);
    
    while(nodeP->next != NULL) {
        printf("1\n");
        if(nodeP->element)
            printf("element in node (addEle()) : %c\n", nodeP->element);
        nodeP = nodeP->next;
    }
    printf("element in node (addEle()) : %c\n", nodeP->element);

    printf("2\n");

    ele *node = (struct ele_t*)malloc(sizeof(ele));
    node->element = elem;
    node->count = 1;
    node->next = NULL;
    printf("3\n");
    nodeP->next = node;
    printf("4\n");
    printNode(head);
    printf("5\n");
}

void printNode(ele *head) {
    ele *node = head;
    node = node->next;
    while(node != NULL) {
        printf("element %c counts %d\n", node->element, node->count);
        if(node->next != NULL)
            node = node->next;
        else 
            break;
    }
}

int cntNode(ele *head, int cntCh) {
    int cnt = 0;
    ele *node = head;
    node = node->next;
    while(node != NULL) {
        printf("element %c counts %d\n", node->element, node->count);
        if (node->count == cntCh)
            cnt++;
        if(node->next != NULL)
            node = node->next;
        else 
            break;
    }
    return cnt;
}

int printstr(char **str, int n) {
    
    for(int j = 0; j < n; j++) {
        printf("data in %d : %s\n", j, *(str+j));    
    }
    ele *head = (struct ele_t*)malloc(sizeof(ele));
    head->next = NULL;
    for(int j = 0; j < n; j++) {
        printf("data in %d : %s\n", j, *(str+j));    
        printf("size : %ld\n", strlen(*(str+j)));
        for(int i = 0; i< strlen(*(str+j)) ; i++)
        {
            printf("************** %c *****************\n", *(str[j]+i) );    
            
            int cnt = getEle(head, *(str[j]+i));
            printf("***** cnt : %d\n", cnt);
            if(0 == cnt){
                printf("will add the element\n");
                addEle(head, *(str[j]+i));
                continue;
                // add element in the list
            } else if (cnt < j+1) {
                printf("will increase the count\n");
                inCnt(head, *(str[j]+i));
                // increment count
            } else {
                printf("Data already available\n");
            }

        }
        printf("\n\n");
    }
    //printNode(head);
    return cntNode(head, n);
}

void main() {
    char **str = (char **) malloc(sizeof(char*)*10);
    printf("1\n");
    for(int i = 0; i < 3; i++)  {
        str[i] = (char*) malloc(sizeof(char)*10);
        if (i == 0)
            strcpy(str[i], "abcdefghijkl");
        if (i == 1)
            strcpy(str[i], "aabcdefghijklbcde");
        if (i == 2)
            strcpy(str[i], "abcdabcdefghijkle");
        
    }

    printf("2\n");
    for(int j = 0; j < 3; j++) {
        printf("data in %d : %s\n", j, str[j]);    
    }
    printf("3\n");
    printf("%d\n", printstr(&(*str), 3));
}

