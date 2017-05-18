#include "myOS.h"
void check_mem(void){
    pMemStart=(unsigned short*)0x100000;                //start from 1MB
    pMemSize=(unsigned long)pMemStart;                  //init size is 1MB
    unsigned short temp;                                //stores memunit
    /*
    void *p=osStart;
    for(;p<0x10000;p++) *(int *)(p+0x10000)=*(int *)p;
    CTX_SW1();
    */
    
    while(1){
        temp=*pMemStart;
        *pMemStart=0xaa55;                          //write aa55 and check if gets aa55
        if(*pMemStart!=0xaa55) {
            pMemSize=(unsigned long)pMemStart;
            break;
        }
        *pMemStart=0x55aa;                          //write 55aa and check if gets 55aa
        if(*pMemStart!=0x55aa) {
            pMemSize=(unsigned long)pMemStart;
            break;
        }
        *pMemStart=temp;
        pMemStart++;
    }
}
void pMemInit(void){
    check_mem();
    myprintf(0x3,"\tMemory Size=0x%x\n",pMemSize);
    int i;
    for(i=0;i<10;i++) Ptnlst[i].state=0;
    //following code tests dPtnAlloc and Free
    /*
    PtnBase=0x100000;
    pMemHandler=dPartitionInit(PtnBase,0x10000000);      //init Memory
    PrintPtnLst();
    unsigned long x1=dPartitionAlloc(pMemHandler,512);
    PrintPtnLst();
    unsigned long x2=dPartitionAlloc(pMemHandler,511);
    PrintPtnLst();
    dPartitionFree(pMemHandler,x1);
    PrintPtnLst();
    dPartitionFree(pMemHandler,x2);
    PrintPtnLst();
    while(1);
    */
    //following code tests eFPtnAlloc and Free
    /*
    PtnTable=eFPartitionInit(0x100000,0x10000,3);
    PrintPtnTable(PtnTable);
    unsigned long x1=eFPartitionAlloc(PtnTable);
    PrintPtnTable(PtnTable);
    unsigned long x2=eFPartitionAlloc(PtnTable);
    PrintPtnTable(PtnTable);
    eFPartitionFree(PtnTable,x1);
    PrintPtnTable(PtnTable);
    while(1);
    */
    int efpsize=eFPartitionTotalSize(sizeof(myTCB),USER_TASK_NUM+1);
    PtnBase=0x100000;
    pMemHandler=dPartitionInit(PtnBase,0x10000000);      //init Memory
    ((fp*)pMemHandler)->pid=0;
    dPartitionAlloc(pMemHandler,efpsize);   
    ((fp*)pMemHandler)->pid=0;
    PtnTable=eFPartitionInit(PtnBase,sizeof(myTCB),USER_TASK_NUM+1);     //init ptntable for tcb pool
}
unsigned long dPartitionInit(unsigned long base,unsigned long size){
    fp* Ptnhead;
    int i=0;
    while(i!=PtnNodePoolSize && Ptnlst[i].state) i++;
    if(i==PtnNodePoolSize) return 0; //fail
    Ptnhead=&Ptnlst[i];                             //use Ptnlst's first node
    Ptnhead->state=1;
    Ptnhead->base=(void*)base;
    Ptnhead->size=size;
    Ptnhead->pre=NULL;
    Ptnhead->next=NULL;
    return (unsigned long)Ptnhead;
}
unsigned long dPartitionAlloc(unsigned long handler, unsigned long size){
    size=((size-1)| 0x3) +1;                    //align 4
    fp *p=(fp*)handler;
    int i=0;
    while(p){
        if(p->size>=size&&p->state==1) {                    //find a free ptn with enough size
            p->pid=tskcurrent->pid;
            p->state=2;
            if(p->size-size<MinPtnSize) {                   //if p->size-size < min allowed ptn size,alloc the entire ptn
                return (int)(p->base);
            }
            else {
                while(i!=PtnNodePoolSize && Ptnlst[i].state) i++;        //p->size - size > min
                if(i==PtnNodePoolSize) {                                 //all Ptnnodes are used, have to alloc the entire ptn
                    
                    return (int)(p->base);
                }
                else {                                      //place Ptnlst[i] behind p
                    if(!p->next) p->next->pre=&Ptnlst[i];
                    Ptnlst[i].next=p->next;
                    p->next=&Ptnlst[i];
                    Ptnlst[i].pre=p;
                    Ptnlst[i].size=p->size-size;
                    p->size=size;
                    Ptnlst[i].base=p->base+size;
                    Ptnlst[i].state=1;
                    return (int)(p->base);
                }
            }
        }
        else p=p->next;
    }
    return 0;//0 no enough;other wise succeed
}
unsigned long dPartitionFree_(fp *p){                             //this function is used after located the ptnnode to free
    int i;
    i=(p->pre?8:0)+(p->next?4:0);                       //uses i's low 4 bits
    if(p->pre && p->pre->state==1) i+=2;
    if(p->next && p->next->state==1) i+=1;
    switch(i){
        case 15:    //pre next exist,all free
                    p->state=0;
                    p->next->state=0;
                    p->pre->size=p->pre->size+p->size+p->next->size;
                    p->pre->next=p->next->next;
                    if(p->next->next) p->next->next->pre=p->pre;
                    return 1;
        case 14:    //pre next exist, pre free
                    p->pre->size=p->pre->size+p->size;
                    p->state=0;
                    p->pre->next=p->next;
                    p->next->pre=p->pre;
                    return 1;
        case 13:    //pre next exist, next free
                    p->next->size=p->size+p->next->size;
                    p->next->base=p->base;
                    p->state=0;
                    p->pre->next=p->next;
                    p->next->pre=p->pre;
                    return 1;
        case 10:    //pre exist and free, no next
                    p->pre->size=p->pre->size+p->size;
                    p->state=0;
                    p->pre->next=NULL;
                    return 1;
        case 5:     //next exist and free, no pre
                    p->state=1;
                    p->next->state=0;
                    p->size=p->size+p->next->size;
                    p->next=p->next->next;
                    if(p->next) p->next->pre=p;
                    return 1;
        case 12:    //pre next exist, not free
                    //p->state=1;
                    //return 0;
        case 8:     //pre exist and not free, no next
                    //p->state=1;
                    //return 0;
        case 4:     //next exist and not free, no pre
                    //p->state=1;
                    //return 0;
        case 0:     //no next or pre
                    p->state=1;
                    return 1;
        default:    return 0;
    }
}
unsigned long dPartitionFreePid(unsigned long pid){            //use pid to locate the ptnnode
    fp *p=(fp*)pMemHandler;
    while(p && p->pid!=pid) p=p->next;
    if(!p) return 0;
    dPartitionFree_(p);
    return 1;
}
unsigned long dPartitionFree(unsigned long handler, unsigned long base){
    fp *p=(fp*)handler;
    while(p && p->base!=base) p=p->next;
    if(p) 
        return dPartitionFree_(p);
    else return 0;
}
void PrintPtnLst(unsigned long handler){
    fp *p=(fp*)handler;
    //fp *q;
    while(p){
        myprintf(0x7,"base:%x size:%x state:%u\n",p->base,p->size,p->state);
        //if(p) q=p;
        p=p->next;
    }
    //***following codes were used to reverse scan the queue to check the 'pre' point
    /*
    while(q){
        myprintf(0x7,"base:%x size:%x state:%u\n",q->base,q->size,q->state);
        q=q->pre;
    }
    */
    putcharclr(0x7,'\n');
}
/*
void* malloc(unsigned int size){
    return dPartitionAlloc(size,tskcurrent->pid);
}
    
void free(void* ptr){                   //uses base to locate ptnnode
    fp *p=Ptnhead;
    while(p && p->base!=ptr) p=p->next;
    if(p) dPartitionFree_(p);
} 
*/
unsigned long eFPartitionTotalSize(unsigned long size, unsigned long number){
    size=((size-1)| 0x3) +1;                    //align 4
    int totalsize=(size+sizeof(efp))*number;    //1 node,1 size area
    return totalsize;
}
unsigned long eFPartitionInit(unsigned long base,unsigned long size,unsigned long number){
    size=((size-1)| 0x3) +1;
    efp* PtnTable;
    PtnTable=(efp*)(base+size*number);          //place ptntable at the end of the allocated area
    int i;
    for(i=0;i<number;i++){
        PtnTable[i].base=base+i*size;
        PtnTable[i].size=size;
        PtnTable[i].state=0;    //free
        PtnTable[i].pid=0;
        PtnTable[i].end=0;
    }
    PtnTable[i-1].end=1;
    //PtnTableNum=number;
    return (unsigned long)PtnTable;
}
unsigned long eFPartitionAlloc(unsigned long handler){
    int i=0;
    while(!((efp*)handler)[i].end && ((efp*)handler)[i].state==1) i++;   //find a free ptn
    if(((efp*)handler)[i].state==1) return 0;
    ((efp*)handler)[i].state=1;
    ((efp*)handler)[i].pid=tskcurrent->pid;
    return ((efp*)handler)[i].base;
}
unsigned long eFPartitionFree(unsigned long handler,unsigned long base){      //locate ptnnode by base 
    int i=0;
    while(!((efp*)handler)[i].end && ((efp*)handler)[i].base!=base) i++;
    if(((efp*)handler)[i].base!=base) return 0;
    ((efp*)handler)[i].state=0;
    return 1;
}
unsigned long eFPartitionFree_(unsigned long handler,unsigned long pid){          //locate ptnnode by pid
    int i=0;
    while(!((efp*)handler)[i].end && ((efp*)handler)[i].pid!=pid) i++;
    if(((efp*)handler)[i].pid!=pid) return 0;
    ((efp*)handler)[i].state=0;
    return 1;
}
void PrintPtnTable(unsigned long PtnTable){
    //while(1);
    int i=-1;
    do {
        i++;
        myprintf(0x7,"base=%x,size=%x,state=%d\n",((efp*)PtnTable)[i].base,((efp*)PtnTable)[i].size,((efp*)PtnTable)[i].state);
    }while(!((efp*)PtnTable)[i].end);
    putcharclr(0x7,'\n');
}
    