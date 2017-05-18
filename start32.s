.section    ".bsdata",  "ax"
char:       .ascii  "O.K.!\n\nCall myMain...\0"
.section    .bss
_bss_start:
.org        0x1000,      0           #bss has 1kB
_bss_end:
.section    ".bstext",  "ax"
.code32
.globl      _start32
.globl      _printf
_start32:
#leal        _bss_end,   %eax        #edx is stack descriptor's real location
#movw        %ax,        2(%edx)     #fill stack base:bssend
movl        $0xffff,    %eax         #top of stack
#subw        $_bss_end,  %ax         #limit of stack
#movw        %ax,        0(%edx)     #stack:from _bss_end to 0x83ff
movl        %eax,       %esp        #set stack point
movl        %eax,       %ebp        #set base point
movw        $32,        %ax         #set stack selector
movw        %ax,        %ss
leal        _bss_start, %eax        #wipe bss
movb        $0,         %bl
clean_bss:
cmpl        $_bss_end,  %eax
jz          clean_bss_end
movb        %bl,        (%eax)      #1B per loop
incl        %eax
jmp         clean_bss
clean_bss_end:
xorl    %ebx,   %ebx                #print in os now
movw    $899,   %bx                 #end of "load os..."
leal    char,   %esi
call    _print                    
call    myMain                        #call main function
dead:
jmp     dead                        #deal loop

_print:
movb    $0x7,   %ah                 #color
cld
lodsb

print_msg:
andb    %al,    %al                 #if reaches string's end
jz      print_end
cmpb    $10,     %al                #if current char is \n
je      newline
movw    %ax,    %gs:0(,%ebx,2)
incw    %bx
newline_end:                        #when get a \n, do not print in this loop
lodsb
jmp     print_msg

newline:
movw    %bx,        %cx             #store current location

movw    %bx,        %ax             #'divb s' will do ax/s, s must not be an imm
movw    $80,        %bx             #ax/s stores in al, ax%s stores in ah
divb    %bl

movw    %cx,        %bx             #get location back

movb    %ah,        %al
movb    $0,         %ah         
subw    %ax,        %bx             #bx-ax stores in bx,so must move ah to al

addw    $80,        %bx             #this is the beginning of newline
movb    $0x7,       %ah             #get color back
jmp     newline_end

print_end:
ret

_printf:
movl    8(%esp),  %ebx              #print to gs:%ebx
movl    4(%esp),  %esi              #string addr
call    _print
ret

.globl CTX_SW
CTX_SW:
pusha
movl    %esp,       prevSP
movl    nextSP,     %esp
popa
ret

.globl CTX_SW1
CTX_SW1:
push    %eax
movl    4(%esp),    %eax
addl    0x10000,    %eax
movl    %eax,       4(%esp)
pop     %eax
ret
