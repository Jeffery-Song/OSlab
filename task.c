#include "myOS.h"
void inqueue(myTCB *p){
    tskrear->next=p;
    tskrear=p;
    tskrear->next=NULL;
}
void dequeue(){
    tskhead->next=tskhead->next->next;
    if(tskhead->next==NULL) tskrear=tskhead;    //if empty after dequeue, set rear=head
}
void stack_init(unsigned long **p,void (*task)(void)){
    *(*p)--=0x08;
    *(*p)--=(unsigned long)task;
    *(*p)--=0xAAAAAAAA;    //eax
    *(*p)--=0xCCCCCCCC;    //ecx
    *(*p)--=0xDDDDDDDD;    //edx
    *(*p)--=0xBBBBBBBB;    //ebx
    *(*p)--=0x44444444;    //esp
    *(*p)--=0x55555555;    //ebp
    *(*p)--=0x66666666;    //esi
    *(*p)=0x77777777;      //edi
}
void createTsk(void (*task)(void)){
    //better scan all TCB, find one with state 2(end) then use it. not necessary in this lab
    myTCB *p=(myTCB*)eFPartitionAlloc(PtnTable);
    p->pid=pid;
    p->state=1;
    p->next=NULL;
    p->stktop=p->stack+127; //top of the stack
    stack_init(&(p->stktop),task);
    inqueue(p);
    pid++;
}
void destoryTsk(){
    eFPartitionFree(PtnTable,(unsigned long)tskcurrent);
}
void tskEnd(){
    dequeue();
    destoryTsk();
    schedule();
}
void schedule(){
    if(tskhead==tskrear) {              //idle left
        if(tskrear->state==0) return;   //idle already runing
        nextSP=tskrear->stktop;         //start idle
        tskrear->state=0;
        tskcurrent=tskrear;
    }
    else {                              //not just idle, start the first task
        nextSP=(tskhead->next)->stktop; 
        tskcurrent->state=1;
        tskcurrent=tskhead->next;
        tskcurrent->state=0;
    }
    CTX_SW();                           //jump to it
}
void idle(){
    while(1){
        schedule();
    }
}
void init(void){
    int *a=(int *)malloc(10*sizeof(int));
    int i;
    for(i=0;i<10;i++) a[i]=i;
    for(i=0;i<10;i++) myprintf(0x7,"%d\t",a[i]);
    putcharclr(0x7,'\n');
    free(a);
    tskEnd();
}
void osStart(){
    pMemInit();          //alloc enough size for tcb pool
    pid=1;
    tskhead=eFPartitionAlloc(PtnTable);                      //init the queue
    tskrear=tskhead;
    tskhead->next=NULL;
    tskhead->state=1;                       //create idle
    tskhead->stktop=tskhead->stack+127;                    
    stack_init(&(tskhead->stktop),idle);
    tskhead->pid=0;
    tskcurrent=tskhead;
    createTsk(initTskBody);             //create init
    //createTsk(init);
    schedule();                         //first schedule, run init in fact
}
