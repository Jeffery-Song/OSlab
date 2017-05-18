#include "stdio.h"
/*
int putchar(char c){
    char *vga=(char*)0xb8000;
    int i;    
    if(offset==(char*)4000){                       //when reach the end of the screen, move above each char
        i=0xb8000;
        for(;i<0xb8000+3840;i+=1){
            *(char*)i=*(char*)(i+160);
        }
        for(;i<0xb8000+4000;i+=2)           //fill the last line with space
            *(char*)i=' ';
        offset=(char*)3840;                        //points to the beginning of the last line
    }
    
    if(c=='\n'){
        offset-=(char*)((int)offset%160);   //newline detected, offset mod 160 then add 160
        offset+=160;
        return 0;
    }
    if(c=='\t'){
        i=(int)(0xb8000+offset);            
        offset-=(char*)((int)offset%16);
        offset=offset+16;
        return 0;
    }
    vga=(char*)(offset+0xb8000);
    *vga=c;
    *(vga+1)=0x7;
    offset+=2;
    return 0;
}
*/
int putcharclr(int color,char c){
    char *vga=(char*)0xb8000;
    int i;    
    if(offset==(char*)4000){                       //when reach the end of the screen, move above each char
        i=0xb8000;
        for(;i<0xb8000+3840;i+=1){
            *(char*)i=*(char*)(i+160);
        }
        for(;i<0xb8000+4000;i+=2)           //fill the last line with space
            *(char*)i=' ';
        offset=(char*)3840;                        //points to the beginning of the last line
    }
    
    if(c=='\n'){
        offset-=(char*)((int)offset%160);   //newline detected, offset mod 160 then add 160
        offset+=160;
        return 0;
    }
    if(c=='\t'){
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
/*
int printf(const char *format,...){
    char *s=format;
    int var=(int)&format+4;                     //var points to the second parameter
    char *format_s;                             //when format is %s, use format_s to point to the string parameter
    unsigned int d;
    int i;
    char num[10]="\0";
    while(*s){
        if(*s=='%'){                            //format scanned
            s++;
            switch(*s){
                case 'c':{
                    putchar(*((char*)var));
                    var+=4;
                    break;
                }
                case 's':{
                    format_s=*(char**)var;
                    while(*format_s){
                        putchar(*format_s);
                        format_s++;
                    }
                    var+=4;
                    break;
                }
                case 'u':{
                    d=*(unsigned int*)var;
                    for(i=0;i<10;i++) {
                        num[i]=(char)('0'+d%10);
                        d=d/10;
                    }
                    
                    i=9;
                    while(num[i]=='0') i--;
                    for(;i>=1;i--) putchar(num[i]);
                    putchar(num[0]);
                    var+=4;
                    break;
                }
                case 'd':{
                    d=*(unsigned int*)var;
                    if((int)d<0) {
                        putchar('-');
                        d=(unsigned)(-(int)d);
                    }
                    for(i=0;i<10;i++) {
                        num[i]=(char)('0'+d%10);
                        d=d/10;
                    }
                    i=9;
                    while(num[i]=='0') i--;
                    for(;i>=1;i--) putchar(num[i]);
                    putchar(num[0]);
                    var+=4;
                    break;
                }
                case 'x':{
                    d=*(unsigned int*)var;
                    //putchar('0');
                    //putchar('x');
                    i=(d & 0xf0000000)>>28;
                    putchar(i>9?i+'a'-10:i+'0');
                    i=(d & 0xf000000)>>24;
                    putchar(i>9?i+'a'-10:i+'0');
                    i=(d & 0xf00000)>>20;
                    putchar(i>9?i+'a'-10:i+'0');
                    i=(d & 0xf0000)>>16;
                    putchar(i>9?i+'a'-10:i+'0');
                    i=(d & 0xf000)>>12;
                    putchar(i>9?i+'a'-10:i+'0');
                    i=(d & 0xf00)>>8;
                    putchar(i>9?i+'a'-10:i+'0');
                    i=(d & 0xf0)>>4;
                    putchar(i>9?i+'a'-10:i+'0');
                    i=(d & 0xf);
                    putchar(i>9?i+'a'-10:i+'0');
                    var+=4;
                    break;
                }
            }
            s++;
        }
        else{
            putchar(*s);
            s++;
        }
    }
    return 0;
}
*/
void clear_screen(void){
    int i=0;
    for(i=0;i<2000;i++) putcharclr(0x7,' ');
}

void myprintf(int color,const char *format,...){
    char *s=format;
    unsigned long var=(unsigned long)&format+4;                     //var points to the second parameter
    char *format_s;                             //when format is %s, use format_s to point to the string parameter
    unsigned int d;
    int i;
    char num[10]="\0";
    while(*s){
        if(*s=='%'){                            //format scanned
            s++;
            switch(*s){
                case 'c':{
                    putcharclr(color,*((char*)var));
                    var+=4;
                    break;
                }
                case 's':{
                    format_s=*(char**)var;
                    while(*format_s){
                        putcharclr(color,*format_s);
                        format_s++;
                    }
                    var+=4;
                    break;
                }
                case 'u':{
                    d=*(unsigned int*)var;
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
                case 'd':{
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
                case 'x':{
                    d=*(unsigned int*)var;
                    //putchar('0');
                    //putchar('x');
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
        else{
            putcharclr(color,*s);
            s++;
        }
    }
}


