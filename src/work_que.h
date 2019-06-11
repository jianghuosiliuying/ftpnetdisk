#ifndef __WORK_QUE_H__
#define __WORK_QUE_H__
#include "head.h"
typedef struct node{
    int newFd;
    int userID;
    struct node* pNext;
}Node_t,*pNode_t;//元素结构体，存储实际client fd
typedef struct{
    pNode_t que_head,que_tail;
    int que_capacity;
    int que_size;
    pthread_mutex_t mutex;
}que_t,*pque_t;//描述队列的结构体
void queInit(pque_t,int);
void queInsert(pque_t,pNode_t);
int queGet(pque_t,pNode_t*);
#endif
