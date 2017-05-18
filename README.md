# 实验七 内存管理

实验内容：

- 实现一个简易的标准输出函数


- 实现物理内存有效性检查，获取内存大小
- 实现动态分区算法：初始化，分配，释放，并重新封装`malloc()`与`free()`
- 实现等大小固定分区算法：根据分区大小与个数计算所需总大小，初始化，分配与释放
- 修改任务池分配算法：从动态物理内存中分配一个动态分区来容纳所需个数的任务，使用固定分区算法来管理这个分区

[TOC]

## 标准输出函数

思路：

- 实现一个能识别`\t` `\n`与自动滚屏并能指定颜色的`putcharclr()`函数，在`myprintf()`中调用它
- `myprintf()`为可变参数函数，通过取格式字符串的地址来获取其后的参数
- `myprintf()`中以字节为单位读取格式字符串，读到`%`时读取下一个参数进行输出
- 定义一个清屏函数`clear_screen()` ，即为输出空格2000次

```c++
int offset;								//offset为以b8000为基址的偏移量
int putcharclr(int color,char c){
    char *vga=(char*)0xb8000;
    int i;    
    if(offset==(char*)4000){			//滚屏，此时已达屏幕末尾
        i=0xb8000;
        for(;i<0xb8000+3840;i+=1){		//前24行等于其下一行的值
            *(char*)i=*(char*)(i+160);
        }
        for(;i<0xb8000+4000;i+=2)		//将最后一行清空
            *(char*)i=' ';
        offset=(char*)3840;				//offset指向最后一行的开头
    }
    
    if(c=='\n'){						//换行，offset指向下一行开头
        offset-=(char*)((int)offset%160);
        offset+=160;
        return 0;
    }
    if(c=='\t'){						//tab，offset指向下一个16的倍数
        i=(int)(0xb8000+offset);
        offset-=(char*)((int)offset%16);
        offset=offset+16;
        return 0;
    }
    vga=(char*)(offset+0xb8000);
    *vga=c;
    *(vga+1)=(char)color;
    offset+=2;
    return 0;
}
void myprintf(int color,const char *format,...){
    char *s=format;
    unsigned long var=(unsigned long)&format+4;		//var指向format后的第一个参数
    char *format_s;                            	 	//当格式为%s，使用format_s指向字符串
    unsigned int d;									//储存整型参数
    int i;
    char num[10]="\0";								//储存需打印的整数
    while(*s){
        if(*s=='%'){                            	//扫描到格式
            s++;
            switch(*s){
                case 'c':{							//单个字符
                    putcharclr(color,*((char*)var));
                    var+=4;							//var指向下一个参数
                    break;
                }
                case 's':{							//字符串
                    format_s=*(char**)var;
                    while(*format_s){
                        putcharclr(color,*format_s);
                        format_s++;
                    }
                    var+=4;
                    break;
                }
                case 'u':{							//无符号整数
                    d=*(unsigned int*)var;
                    for(i=0;i<10;i++) {
                        num[i]=(char)('0'+d%10);	//将d每位逆向存入num数组
                        d=d/10;
                    }
                    
                    i=9;
                    while(num[i]=='0') i--;			//从num中第一个非0项开始打印
                    for(;i>=1;i--) putcharclr(color,num[i]);
                    putcharclr(color,num[0]);		//至少打印一位
                    var+=4;
                    break;
                }
                case 'd':{			//有符号整数，与无符号类似，只多一步检测正负并转换为正数
                    d=*(unsigned int*)var;
                    if((int)d<0) {
                        putcharclr(color,'-');
                        d=(unsigned)(-(int)d);
                    }
                    for(i=0;i<10;i++) {
                        num[i]=(char)('0'+d%10);
                        d=d/10;
                    }
                    i=9;
                    while(num[i]=='0') i--;
                    for(;i>=1;i--) putcharclr(color,num[i]);
                    putcharclr(color,num[0]);
                    var+=4;
                    break;
                }
                case 'x':{							//16进制整数
                    d=*(unsigned int*)var;
                    i=(d & 0xf0000000)>>28;
                    putcharclr(color,i>9?i+'a'-10:i+'0');
                    i=(d & 0xf000000)>>24;
                    putcharclr(color,i>9?i+'a'-10:i+'0');
                    i=(d & 0xf00000)>>20;
                    putcharclr(color,i>9?i+'a'-10:i+'0');
                    i=(d & 0xf0000)>>16;
                    putcharclr(color,i>9?i+'a'-10:i+'0');
                    i=(d & 0xf000)>>12;
                    putcharclr(color,i>9?i+'a'-10:i+'0');
                    i=(d & 0xf00)>>8;
                    putcharclr(color,i>9?i+'a'-10:i+'0');
                    i=(d & 0xf0)>>4;
                    putcharclr(color,i>9?i+'a'-10:i+'0');
                    i=(d & 0xf);
                    putcharclr(color,i>9?i+'a'-10:i+'0');
                    var+=4;
                    break;
                }
            }
            s++;
        }
        else{										//非格式，直接输出
            putcharclr(color,*s);
            s++;
        }
    }
}
```

## 内存有效性检查

​	从1MB（0x100000）开始，依次给每两个字节的内存填写0xaa55，读取判断是否为0xaa55，填写0x55aa，读取判断是否为0x55aa，直至读取值与写入不一致。

​	为什么填写0xaa55与0x55aa？从0xaa55到0x55aa，16bit中的每一bit都被改变一次，每一bit的有效性都能被检查到。同理，先后填写0x5555与0xaaaa也是可以的。

```c++
void check_mem(void){
    pMemStart=(unsigned short*)0x100000;		//从1MB开始检查
    pMemSize=(unsigned long)pMemStart;			//初始内存大小为1MB
    unsigned short temp;
    while(1){
        temp=*pMemStart;
        *pMemStart=0xaa55;						//写入 aa55 并读取，判断是否为 aa55
        if(*pMemStart!=0xaa55) {
            pMemSize=(unsigned long)pMemStart;
            break;
        }
        *pMemStart=0x55aa;						//写入 55aa 并读取，判断是否为 55aa
        if(*pMemStart!=0x55aa) {
            pMemSize=(unsigned long)pMemStart;
            break;
        }
        *pMemStart=temp;
        pMemStart++;
    }
}
myprintf(0x7,"\tMemory Size=0x%x\n",pMemSize);
```

​	为减少内存检查时间，qemu加上内存大小参数`-m 32M`

​	运行结果：![Memory_size](/home/oem/图片/Screenshot_20170515_173108.png)

## 动态分区算法

思路：

- 使用双向链表来管理分区，一个句柄指向一个链表，由于至少有一个结点故不设空结点
- 分配分区使用***首次适应算法*** 以方便测试，需要新结点时从结点池中寻找一个未使用的结点
- 释放分区根据前后结点存在与否以及占用与否需分很多情况，故使用一个整型值的低4位来记录各情况

```c++
#define PtnNodePoolSize 10	//结点池大小
#define MinPtnSize 4		//最小分区大小
#define malloc(size) dPartitionAlloc(pMemHandler,size)
#define free(start) dPartitionFree(pMemHandler,start)
typedef struct fp{			//结点类型
    struct fp *pre,*next;
    void* base;
    unsigned long size,state,pid;   		//state:0 结点未使用,1 分区空闲,2 分区被使用
} fp;
unsigned long dPartitionInit(unsigned long base,unsigned long size){
    fp* Ptnhead;
    int i=0;
    while(i!=10 && Ptnlst[i].state) i++;	//查找未被使用的结点
    if(i==10) return 0;						//所有结点均被使用，分配失败
    Ptnhead=&Ptnlst[i];
    Ptnhead->state=1;						//空闲
    Ptnhead->base=(void*)base;
    Ptnhead->size=size;
    Ptnhead->pre=NULL;
    Ptnhead->next=NULL;
    return (unsigned long)Ptnhead;			//返回句柄
}
unsigned long dPartitionAlloc(unsigned long handler, unsigned long size){
    size=((size-1)| 0x3) +1;                //4字节对齐
    fp *p=(fp*)handler;
    int i=0;
    while(p){
        if(p->size>=size&&p->state==1) {	//找到第一个大小足够的空闲的分区
            p->pid=tskcurrent->pid;
            p->state=2;
            if(p->size-size<MinPtnSize) {//如果分区后剩余大小 小于 最小大小，分配整个区域
                return (int)(p->base);
            }
            else {				//分区后剩余大小 大于 最小大小，需找一个未使用结点插入队列
                while(i!=PtnNodePoolSize && Ptnlst[i].state) i++;
                if(i==PtnNodePoolSize) {	//所有结点均已被使用，只得分配整个分区
                    return (int)(p->base);
                }
                else {						//将Ptnlst[i]插入队列
                    if(!p->next) p->next->pre=&Ptnlst[i];
                    Ptnlst[i].next=p->next;
                    p->next=&Ptnlst[i];
                    Ptnlst[i].pre=p;
                    Ptnlst[i].size=p->size-size;
                    p->size=size;
                    Ptnlst[i].base=p->base+size;
                    Ptnlst[i].state=1;
                    return (int)(p->base);	//返回分区的基址
                }
            }
        }
        else p=p->next;
    }
    return 0;								//找不到大小足够的空闲分区，0 为分区失败
}
unsigned long dPartitionFree_(fp *p){		//该函数为定位到被释放的分区后调用的函数
    int i;									//使用i的低4位来区分前后结点的情况
    i=(p->pre?8:0)+(p->next?4:0);			//3位为有前结点，2位为有后结点
    if(p->pre && p->pre->state==1) i+=2;	//1位为前结点空闲
    if(p->next && p->next->state==1) i+=1;	//0位为后结点空闲
    switch(i){
        case 15:    //前后结点都存在并且空闲
                    p->state=0;
                    p->next->state=0;
                    p->pre->size=p->pre->size+p->size+p->next->size;
                    p->pre->next=p->next->next;
                    if(p->next->next) p->next->next->pre=p->pre;
                    return 1;
        case 14:    //前后结点都存在，前结点空闲
                    p->pre->size=p->pre->size+p->size;
                    p->state=0;
                    p->pre->next=p->next;
                    p->next->pre=p->pre;
                    return 1;
        case 13:    //前后结点都存在，后结点空闲
                    p->next->size=p->size+p->next->size;
                    p->next->base=p->base;
                    p->state=0;
                    p->pre->next=p->next;
                    p->next->pre=p->pre;
                    return 1;
        case 10:    //前结点存在且空闲，无后结点
                    p->pre->size=p->pre->size+p->size;
                    p->state=0;
                    p->pre->next=NULL;
                    return 1;
        case 5:     //后结点存在且空闲，无前结点
        			p->state=1;
                    p->next->state=0;
                    p->size=p->size+p->next->size;
                    p->next=p->next->next;
                    if(p->next) p->next->pre=p;
                    return 1;
		case 12:	//前后结点均存在且都被占用		//该4种情况操作一样
		case 8:		//前结点存在且被占用，无后结点
		case 4:		//后结点存在且被占用，无前结点	
		case 0:		//前后结点均不存在				
                    p->state=1;
                    return 1;
        default:    return 0;
    }
}
unsigned long dPartitionFree(unsigned long handler, unsigned long base){
    fp *p=(fp*)handler;
    while(p && p->base!=base) p=p->next;		//定位以base值对应的分区
    if(p) 
        return dPartitionFree_(p);				//找到对应结点，调用free_
    else return 0;
}
```

接下来写了一组测试代码来检查上述代码

```c++
void PrintPtnLst(void){
    fp *p=(fp*)pMemHandler;
    fp *q;
    while(p){			//遍历链表，输出每个分区信息
        myprintf(0x7,"base:%x size:%x state:%u\n",p->base,p->size,p->state);
        if(p) q=p;
        p=p->next;
    }					//此时q指向最后一个结点
    while(q){			//反方向遍历链表，输出分区信息以检查pre指针的正确性
        myprintf(0x7,"base:%x size:%x state:%u\n",q->base,q->size,q->state);
        q=q->pre;
    }
}
void pMemInit(void){
    check_mem();
    myprintf(0x3,"\tMemory Size=0x%x\n",pMemSize);
    int i;
    for(i=0;i<10;i++) Ptnlst[i].state=0;					//初始化结点池
    //接下来的代码测试动态分区算法相关函数
    PtnBase=0x100000;
    pMemHandler=dPartitionInit(PtnBase,0x10000000);      	//初始化分区，大小为16M
    PrintPtnLst();
    putcharclr(0x7,'\n');
    unsigned long x1=dPartitionAlloc(pMemHandler,512);
    PrintPtnLst();
    putcharclr(0x7,'\n');
    unsigned long x2=dPartitionAlloc(pMemHandler,511);		//检测是否能对齐
    PrintPtnLst();
    putcharclr(0x7,'\n');
    dPartitionFree(pMemHandler,x1);
    PrintPtnLst();
    putcharclr(0x7,'\n');
    dPartitionFree(pMemHandler,x2);
    PrintPtnLst();
    while(1);
}
```

在`osStart()`中调用`pMemInit()`，qemu运行结果如图

![Test_Ptnlst](/home/oem/图片/Screenshot_20170515_200729.png)

说明分区算法工作正常

## 等大小固定分区算法

思路：

- 在所需分配的总空间的结尾放置一个数组作为分区表，在分区表结点类型中加入end标志位来表示表的结尾

```c++
typedef struct efp{							//分区表结点类型
    void* base;
    unsigned long size,state,pid,end;   	//state:0 空闲，1 被占用；end：1 该结点为结尾
} efp;
unsigned long eFPartitionTotalSize(unsigned long size, unsigned long number){
    size=((size-1)| 0x3) +1;				//4B对齐
    int totalsize=(size+sizeof(efp))*number;
    return totalsize;
}
unsigned long eFPartitionInit(unsigned long base,unsigned long size,unsigned long number){
    size=((size-1)| 0x3) +1;
    efp* PtnTable;
    PtnTable=(efp*)(base+size*number);      //将分区表放在区域的结尾
    int i;
    for(i=0;i<number;i++){					//初始化分区表
        PtnTable[i].base=base+i*size;
        PtnTable[i].size=size;
        PtnTable[i].state=0;
        PtnTable[i].pid=0;
        PtnTable[i].end=0;
    }
    PtnTable[i-1].end=1;					//设置结尾
    return (unsigned long)PtnTable;
}
unsigned long eFPartitionAlloc(unsigned long handler){
    int i=0;
    while(!((efp*)handler)[i].end && ((efp*)handler)[i].state==1) i++;  //找一个空闲分区
    if(((efp*)handler)[i].state==1) return 0;//可能为因最后一项而退出while
    ((efp*)handler)[i].state=1;
    ((efp*)handler)[i].pid=tskcurrent->pid;
    return ((efp*)handler)[i].base;
}
unsigned long eFPartitionFree(unsigned long handler,unsigned long base){
    int i=0;
    while(!((efp*)handler)[i].end && ((efp*)handler)[i].base!=base) i++;
    if(((efp*)handler)[i].base!=base) return 0;	//找不到指定分区
    ((efp*)handler)[i].state=0;
    return 1;
}
```

类似的，写一组测试代码

```c++
void PrintPtnTable(unsigned long PtnTable){
    int i=-1;
    do {
        i++;
        myprintf(0x7,"base=%x,size=%x,state=%d\n",((efp*)PtnTable)[i].base,((efp*)PtnTable)[i].size,((efp*)PtnTable)[i].state);
    }while(!((efp*)PtnTable)[i].end);				//遍历分区表
    putcharclr(0x7,'\n');
}
void pMemInit(void){
    check_mem();
    myprintf(0x3,"\tMemory Size=0x%x\n",pMemSize);
    //接下来的代码测试固定分区算法相关函数
    PtnTable=eFPartitionInit(0x100000,0x10000,3);	//初始化3个分区
    PrintPtnTable(PtnTable);
    unsigned long x1=eFPartitionAlloc(PtnTable);	//分配第一个分区
    PrintPtnTable(PtnTable);
    unsigned long x2=eFPartitionAlloc(PtnTable);	//分配第二个分区
    PrintPtnTable(PtnTable);
    eFPartitionFree(PtnTable,x1);					//释放第一个分区
    PrintPtnTable(PtnTable);
    while(1);
}
```

测试结果

![Screenshot_20170515_214500](/home/oem/图片/Screenshot_20170515_214500.png)

说明代码运行正常

## 综合使用

建一个足够大的动态分区，从中分配足够空间交由固定大小分区管理，并将从TCB pool中分配TCB给进程改为从固定大小分区中分配

```c++
void pMemInit(void){
    check_mem();
    myprintf(0x3,"\tMemory Size=0x%x\n",pMemSize);
    int i;
    for(i=0;i<10;i++) Ptnlst[i].state=0;			//初始化动态分区结点池
    int efpsize=eFPartitionTotalSize(sizeof(myTCB),USER_TASK_NUM+1);//计算进程所需总空间
    PtnBase=0x100000;								//1MB
    pMemHandler=dPartitionInit(PtnBase,0x10000000);
    ((fp*)pMemHandler)->pid=0;
    dPartitionAlloc(pMemHandler,efpsize);			//分配进程所需总空间并交给固定分区算法管理
    ((fp*)pMemHandler)->pid=0;
    PtnTable=eFPartitionInit(PtnBase,sizeof(myTCB),USER_TASK_NUM+1);
}
void createTsk(void (*task)(void)){	//创建进程
    myTCB *p=eFPartitionAlloc(PtnTable);
  	p->pid=pid;						//分配pid
    p->state=1;						//等待
    p->next=NULL;
    p->stktop=p->stack+127; 		//栈顶
    stack_init(&(p->stktop),task);
    inqueue(p);
    pid++;
}
void destoryTsk(){
    eFPartitionFree(PtnTable,tskcurrent);
}
void osStart(){
    pMemInit();          					//内存初始化
    pid=1;
    tskhead=eFPartitionAlloc(PtnTable);		//初始化进程队列
    tskrear=tskhead;
    tskhead->next=NULL;
    tskhead->state=1;                       //创建idle进程
    tskhead->stktop=tskhead->stack+127;                    
    stack_init(&(tskhead->stktop),idle);
    tskhead->pid=0;
    tskcurrent=tskhead;
    createTsk(initTskBody);             	//创建init进程
    schedule();                         	//第一次调度，进入init
}
```

运行测试用例结果

![Screenshot_20170515_223300](/home/oem/图片/Screenshot_20170515_223300.png)

分析：

TSK0：

- buf1与buf2为局部变量，其储存于进程栈内，地址相隔4字节，与图中一致。

```c++
char*buf1 = (char*)malloc(19);
char*buf2 = (char*)malloc(24);
```

- buf1与buf2指向的字符串为malloc分配的。动态分区中第一个分区为初始化的几个进程的TCB所占用的空间，第二个分区空闲，起始地址为0x00100ab4，故buf1值应为0x00100ab4，buf2值则应为20(*19进行4B对齐*)+0x00100ab4=0x00100ac8，与图中相符。

TSK1：

- 与TSK0类似，值得注意的是TSK0中的buf1、buf2已被free了，故TSK1中malloc的空间仍从0x00100ab4开始

TSK2：

- 首先测试最大可行的动态分区大小，以KB为单位从1开始测试，测试结果即为`pMemInit()`中分配的大小

```c++
while(1){
	x = malloc(i); //myprintf(0x7,"X:0x%x:0x%x ",x,i);
	if(x) free(x);
	else break;
	i+=0x1000;
}
```

- 接下来分配了一个大小为0x100的动态分区，输出为`dP:`并将该区域作为一块空闲内存进行动态内存初始化

```c++
tsize = 0x100;
x = malloc(tsize); 
if (x){
	myprintf(0x7,"dP:0x%x:0x%x\n",x,tsize);
	xHandler = dPartitionInit(x,tsize);
```

- 从该区域分配大小为0x10，0x20...的分区，依次输出为`EMB:`并释放直至分配失败。分配失败后再重新分配一大小为0x10的分区，输出为`EMB_again`，可见这一系列分区应均以0x00100ab4为起点，与结果一致

```c++
	i=0x10;
	while(1){
		x1 = dPartitionAlloc(xHandler,i); 
		if(x1) {
			myprintf(0x7,"EMB:0x%x:0x%x ",x1,i);				
			dPartitionFree(xHandler,x1);
		}	else break;
		i+=0x10;
	}
	x1 = dPartitionAlloc(xHandler,0x10);
	if(x1) {
		myprintf(0x7,"\nEMB_again:0x%x:0x10\n",x1);
		dPartitionFree(xHandler,x1);
	}
	free(x);
}	else myprintf(0x7,"TSK2: MALLOC FAILED, CAN't TEST dPartition\n");
```

- 接下来从动态内存中建立了4个psize大小的固定大小分区

```c++
tsize = eFPartitionTotalSize(psize,n);
x = malloc(tsize); myprintf(0x7,"X:0x%x:%d ",x,tsize);
if(x){
	xHandler = eFPartitionInit(x,psize,n);
```

- 分配释放了两次，输出为x1与x2，其值相同

```c++
	x1=eFPartitionAlloc(xHandler); myprintf(0x7,"X1:0x%x ",x1);
	eFPartitionFree(xHandler,x1);
	x1=eFPartitionAlloc(xHandler); myprintf(0x7,"X2:0x%x \n",x1);
	eFPartitionFree(xHandler,x1);
```

- 再连续分配4次，输出为y0，y1，y2，y3，释放，可见地址递增，相差为32，即31对齐结果

```c++
	for(i=0;i<n;i++) {
		y[i] = eFPartitionAlloc(xHandler); myprintf(0x7,"Y%d:0x%x ",i,y[i]);
	}
	for(i=0;i<n;i++) {
		eFPartitionFree(xHandler,y[i]);
	}
```

- 后再次分配一个固定分区输出为x3，值为0x00100ab4

```c++
	x1=eFPartitionAlloc(xHandler); myprintf(0x7,"\nX3:0x%x\n",x1);
	eFPartitionFree(xHandler,x1);
}else myprintf(0x7,"TSK2: MALLOC FAILED, CAN't TEST eFPartition\n");
```

可见输出与预期相符

## 总结

本次实验没有太大难点，基本都是比较简单的数据结构。动态分区的测试用例比较难写，有些释放的情况没能测试到
