#ifndef _myOS_h_    //avoid redeclaration
#define _myOS_h_

#define NULL 0
#define MinPtnSize 4

#define malloc(size)  dPartitionAlloc(pMemHandler,size)     //return basepoint
#define free(start) dPartitionFree(pMemHandler,start)

#define PtnNodePoolSize 20

#include "../userApp/userApp.h"
#include "stdio.h"
//void myprintf(int color, const char *format,...);
typedef struct myTCB{
    unsigned int pid;
    unsigned int state;//0 runing,1 waiting,2 end
    struct myTCB *next;
    unsigned long *stktop;
    unsigned long stack[128];
} myTCB;
typedef struct fp{
    struct fp *pre,*next;
    void* base;
    unsigned long size,state,pid;   //state:0 unused,1 free,2 occupied
} fp;
typedef struct efp{
    void* base;
    unsigned long size,state,pid,end;   //state:0 free,1 occupied; end 1:end node
} efp;
//task
void initTskBody(void);
void CTX_SW(void);
void CTX_SW1(void);
void inqueue(myTCB *p);
void dequeue(void);
void idle(void);
void stack_init(unsigned long **p,void (*task)(void));
void createTsk(void (*task)(void));
void destoryTsk(void);
void tskEnd(void);
void schedule(void);

void osStart(void);

//memory
void pMemInit(void);
unsigned long dPartitionInit(unsigned long base,unsigned long size);
unsigned long dPartitionAlloc(unsigned long handler, unsigned long size);
unsigned long dPartitionFree_(fp *p);
unsigned long dPartitionFree(unsigned long handler,unsigned long base);
void PrintPtnLst(unsigned long handler);
//void* malloc(unsigned int size);
//void free(void* ptr);

unsigned long eFPartitionTotalSize(unsigned long size, unsigned long number);
unsigned long eFPartitionAlloc(unsigned long handler);
unsigned long eFPartitionInit(unsigned long base,unsigned long size,unsigned long number);
unsigned long eFPartitionFree(unsigned long handler,unsigned long base);
unsigned long eFPartitionFree_(unsigned long handler,unsigned long pid);
//unsigned int PtnTableNum;

myTCB *tskhead,*tskrear,*tskcurrent;
myTCB TCB[USER_TASK_NUM+1];
unsigned long *prevSP,*nextSP;
unsigned short *pMemStart;
unsigned long pMemSize;
int pid;
fp Ptnlst[10];
unsigned long pMemHandler;
//fp* Ptnhead;
unsigned long PtnTable;
unsigned long PtnBase;

void smallint(void);
void tick(void);
void updatetime(void);
volatile int tick_number;
volatile int rrtime;
int hour,min,sec;
void clkinit(void);
void disableRQ(void);
void enableRQ(void);
unsigned long espstore;
void CTX_SW_iret(void);
#endif