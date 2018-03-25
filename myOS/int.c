#include "myOS.h"
void tick(void){
    tick_number++;
    rrtime++;
    if (tick_number%100==0){
        if (sec==59){
            sec=0;
            if(min==59){
                min=0;
                if(hour==99) hour=0;
                else hour++;
            }
            else min++;
        }
        else sec++;
        updatetime();
    }
    if (rrtime%50 == 0 ) schedule();
    //if (tick_number==1) while(1);
} 
void updatetime(void){
    unsigned short *p=0xb8000+144;
    *p=0x7000 + (0xff & (hour/10)) + '0';
    p++;
    *p=0x7000 + (0xff & (hour%10)) + '0';
    p++;
    *p=0x7000 + ':';
    p++;
    *p=0x7000 + (0xff & (min/10)) + '0';
    p++;
    *p=0x7000 + (0xff & (min%10)) + '0';
    p++;
    *p=0x7000 + ':';
    p++;
    *p=0x7000 + (0xff & (sec/10)) + '0';
    p++;
    *p=0x7000 + (0xff & (sec%10)) + '0';
}
void clkinit(void){
    tick_number=0;
    hour=min=sec=0;
    updatetime();
}