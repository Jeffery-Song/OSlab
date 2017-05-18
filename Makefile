clean:
	rm *.o
	rm *.bin
	rm *.elf
start16.o:start16.s
	cc -c -m32 $< -o $@
start16.elf:start16.o
	ld -T start16.ld $< -o $@
start16.bin:start16.elf
	objcopy -O binary $< $@
start32.o:start32.s
	cc -c -m32 $< -o $@
main.o:main.c myOS.h userApp.h stdio.h
	cc -c -m32 $< -o $@ -fno-stack-protector
printf.o:printf.c
	cc -c -m32 $< -o $@ -fno-stack-protector
memory.o:memory.c
	cc -c -m32 $< -o $@ -fno-stack-protector
init.o:init.c myOS.h
	cc -c -m32 $< -o $@ -fno-stack-protector
task.o:task.c myOS.h
	cc -c -m32 $< -o $@ -fno-stack-protector
os.elf:main.o start32.o task.o init.o start32.ld printf.o memory.o
	ld -T start32.ld printf.o memory.o main.o task.o init.o start32.o -o $@
os.bin:os.elf
	objcopy -O binary $< $@
os:os.bin start16.bin new.img
	rm fp.img
	cp new.img fp.img
	sudo losetup /dev/loop4 fp.img
	sudo dd if=start16.bin of=/dev/loop4 bs=512 count=1
	sudo dd if=os.bin of=/dev/loop4 bs=512 seek=1
	sudo losetup -d /dev/loop4
