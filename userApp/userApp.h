#ifndef __USER_APP_H__
#define __USER_APP_H__

//========output API, user defined=======================================
#define WHITE 0x7
void clear_screen(void);
int putcharclr(int color,char c);
//int printf(const char *format,...);
void myprintf(int color,const char *format,...);
char *offset;
//int printf(const char *format,...);
//========memory API, user defined======================================
#define malloc(size)  dPartitionAlloc(pMemHandler,size)     //return basepoint
#define free(start) dPartitionFree(pMemHandler,start)
//=======APP, user should define this==================================
#define USER_TASK_NUM 4   // init + myTSK0-2

#endif
