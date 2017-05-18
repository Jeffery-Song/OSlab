.section    .data
size:       .word   2000
string1:    .ascii "Hello PB15000301\n\n\rPress any key to get into protect mode...\0"
string3:    .ascii "Switch to protect mode...\0"
ldos:       .ascii "Loading OS...\0"
ok:         .ascii "O.K.!\0"
.globl      _GDT
_GDT:       .long   0
            .long   0
            
_DESC_CODE32:
            .word   0xffff              #limit 0-15
            .word   0                   #base 0-15
            .byte   0                   #base 16-23
            .byte   0x98                #10011000
                                        #TYPE=1000
                                        #S=1 DPL=00 P=1
            .byte   0xcf  
                                        #limit 16-19
                                        #G=1,D/B=1,other 0
            .byte   0                   #base 24-31
            
_DESC_VIDEO:
            .word   0xffff              #limit 0-15
            .word   0x8000              #base 0-15
            .byte   0x0b                #base 16-23
            .byte   0x92                #10010010
                                        #TYPE=0010
                                        #S=1 DPL=00 P=1
            .byte   0xf                 #limit 16-19
                                        #G,D/B,AVL=0
            .byte   0                   #base 24-31
            
_DESC_DATA:
            .word   0xffff              #limit 0-15
            .word   0                   #base 0-15
            .byte   0                   #base 16-23
            .byte   0x92                #10010010
                                        #TYPE=0010
                                        #S=1 DPL=0 P=1
            .byte   0xcf                #limit 16-19
                                        #G=1,D/B=1,AVL=0
            .byte   0                   #base 24-31
            
_DESC_STACK:
            .word   0xffff              #limit 0-15
            .word   0                   #base 0-15
            .byte   0                   #base 16-23
            .byte   0x93                #10010011
                                        #TYPE=0110
                                        #S=0 DPL=11 P=1
            .byte   0x40  
                                        #limit 16-19
                                        #D/B=1,other 0
            .byte   0                   #base 24-31
            
Gdtend:     
GdtPtr:     .word   Gdtend - _GDT - 1
            .long   0
end_data16:


.section    ".DATA32",  "a"
.globl  data32
data32:
inpm:       .ascii  "O.K.!\n\nWe are now in protect mode\n\nGOING TO START32...\0"
end_data32:


.section    .text
.code16
.globl  _start
_start:
cli
movw    $0xb800,    %ax
movw    %ax,        %es
movw    $0,         %ax

loop_wipe: 
cmpw    size,       %ax
je      wipedone
movb    $0x20,      %es:0(,%eax,2)
incw    %ax
jmp     loop_wipe
wipedone:

movw    %ds,        %ax
movw    %ax,        %es     #string1 at es:ebp
movw    $0x1301,    %ax     #int No.13
movw    $0x0007,    %bx     #color
movw    $60,        %cx     #length
movw    $0x0100,    %dx     #row&column
leal    string1,    %ebp
int     $0x10               #bios int

movb    $0,         %ah     #get keyboard input
int     $0x16

#fill GDTR
xorl    %eax,       %eax
movw    %ds,        %ax
shl     $4,         %eax
add     $_GDT,      %eax                #calculate GDT base
leal    GdtPtr,     %ebx
movl    %eax,       %ds:2(,%ebx,1)      #fill GDT base

lgdt    %ds:0(,%ebx,1)                  #load GDTR

#print load os
movw    $0x1301,    %ax     #int No.13
movw    $0x0007,    %bx     #color
movw    $16,        %cx     #length
movw    $0x0500,    %dx     #row&column
leal    ldos,       %ebp
int     $0x10               #bios int

#load os
movw    $0,         %ax
movw    %ax,        %es     #os store to es:bx
movw    $0x7e00,    %bx                 
movw    $0x0212,    %ax     #ah=02h,read from drive
                            #al=01,read 18 sector
movw    $0,    %dx          #dh head num,0
                            #dl drive,0 is the first fdisk
movw    $0x0002,    %cx     #cylinder0,sector2
int     $0x13

#print ok
movw    $0x1301,    %ax     #int No.13
movw    $0x0007,    %bx     #color
movw    $16,        %cx     #length
movw    $0x050d,    %dx     #row&column
leal    ok,    %ebp
int     $0x10               #bios int


#movw    $0x1301,    %ax     #print after key press
#movw    $0x0007,    %bx
movw    $25,        %cx
movw    $0x0700,    %dx
leal    string3,    %ebp
int     $0x10


movw    $0x2401,    %ax                 #turn on A20
int     $0x15

movl    %cr0,       %eax
orl     $1,         %eax                #in order to protect high bits of cr0
movl    %eax,       %cr0                #and set pe to 1 at same time
ljmp    $8,         $code32             #ljmp between segment to 8:0
                                        #8 is code32seg's index (selector)
                                        #0 is offset
end_code16:


.section ".CODE32", "a"
.globl  code32
.code32
code32:
cli
leal    _DESC_STACK,    %edx
movl    $24,    %eax
movl    %eax,   %ds             #24 is the offset of data desc in gdt
movl    $16,    %eax
movl    %eax,   %gs             #16 is ... of video

movw    $585,   %bx             #where to print
leal    inpm,   %esi            #string location
call    _print
/*
movw    $880,   %bx             #where to print
leal    goto32,   %esi          #string location
call    _print
*/
ljmp    $8,         $0x7e00     #jmp to os
                                #8 is code selector, base is 0
                                #offset is 7e00, location of os

_print:
movb    $0x7,   %ah             #color
cld                             #set si's direction
lodsb

print_msg:
andb    %al,    %al             #if reaches string's end
jz      _printend
cmpb    $10,    %al             #if char to print is \n
je      newline
movw    %ax,    %gs:0(,%ebx,2)
incw    %bx
newline_end:                    #when get a \n, do not print in this loop,with bx set
lodsb
jmp     print_msg
_printend: ret

newline:
movw    %bx,        %cx         #store current location

movw    %bx,        %ax         #'divb s' will do ax/s, s must not be an imm
movw    $80,        %bx         #ax/s stores in al, ax%s stores in ah
divb    %bl

movw    %cx,        %bx         #get location back

movb    %ah,        %al
movb    $0,         %ah         
subw    %ax,        %bx         #bx-ax stores in bx,so must move ah to al

addw    $80,        %bx         #this is the beginning of newline
movb    $0x7,       %ah         #get color back
jmp     newline_end
end_code32:


.section ".signature",  "a"
.word   0xaa55
