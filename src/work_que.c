#include "work_que.h"

void queInit(pque_t pq,int capacity)
{
    memset(pq,0,sizeof(que_t));
    pq->que_capacity=capacity;
    pthread_mutex_init(&pq->mutex,NULL);
}

void queInsert(pque_t pq,pNode_t pnew)
{
    if(pq->que_tail==NULL)
    {
        pq->que_head=pnew;
        pq->que_tail=pnew;
    }else{
        pq->que_tail->pNext=pnew;
        pq->que_tail=pnew;
    }
    pq->que_size++;
}

int queGet(pque_t pq,pNode_t* pdelete)//get到任务，将其从任务队列头部删除
{
    pNode_t pcur;
    pcur=pq->que_head;
    pq->que_head=pcur->pNext;
    if(pq->que_head==NULL)
    {
        pq->que_tail=NULL;
    }
    pq->que_size--;
    *pdelete=pcur;
    return 0;
}
