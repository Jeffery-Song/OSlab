.section    ".IDT",     "ax"
.align      32
IDT:
.rept       64
.word       clkhandler
.word       0x8
.word       0x8E00
.word       0
.endr
IDTPtr:
.word       IDTPtr - IDT - 1
IDTBase:
.long       IDT

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
call    init8259A
call    init8253

xorl    %eax,   %eax
movl    $IDT,   %eax
#movl    %eax,   IDTBase
movl    $IDTPtr,    %ebx
movl    %eax,       %ds:2(,%ebx,1)
lidt    %ds:0(,%ebx,1)
call    clkinit
#int     $0x31
call    myMain                        #call main function
dead:
jmp     dead                        #deal loop



.globl myfrstint
myfrstint:
movw    $0x0221,  %ax
movl    $79,    %ebx
movw    %ax,    %gs:0(,%ebx,2)
intloop:
jmp intloop
iret

.globl clkhandler
clkhandler:
pushf
pusha
movl    %esp,   espstore
call tick
popa
popf
iret

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

.globl CTX_SW_iret
CTX_SW_iret:
movl    prevSP,     %eax
movl    espstore,   %ecx
movl    %ecx,       (%eax)
movl    nextSP,     %esp
popa 
popf
sti
ret

.global CTX_SW
CTX_SW: 
#pushf 
#pusha 
#movl    prevSP,     %eax
#movl    espstore,   %ecx
#movl    %ecx,       (%eax)
movl    nextSP,     %esp
popa 
popf
sti
ret

io_delay:
nop
nop
nop
ret

.globl init8259A
init8259A:
mov $0xff, %al
out %al, $0x21
out %al, $0xA1
	
mov $0x11, %al
out %al, $0x20
	
mov $0x20, %al
out %al, $0x21
	
mov $0x04, %al
out %al, $0x21
##define AUTO_EOI 1
##if AUTO_EOI
mov $0x03, %al
out %al, $0x21
##else
#mov $0x01, %al
#out %al, $0x21
##endif
  	
mov $0x11, %al
out %al, $0xA0
	
mov $0x28, %al
out %al, $0xA1
	
mov $0x02, %al
out %al, $0xA1

mov $0x01, %al
out %al, $0xA1

ret

.globl init8253
init8253:
mov $0x34, %al
out %al, $0x43

# ?100HZ?
mov $(11932 & 0xff), %al
out %al, $0x40
	
mov $(11932 >> 8), %al
out	%al, $0x40	
	
in $0x21, %al 
andb $0xFE, %al
out %al, $0x21
ret

.globl  smallint
smallint:
int $0x20
ret

.globl  disableRQ
disableRQ:
cli
ret
.globl  enableRQ
enableRQ:
sti
ret