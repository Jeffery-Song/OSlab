clean:
	rm -rf output
	mkdir output
	mkdir output/userApp
	mkdir output/myOS
output/start16.o:start16.s
	cc -c -m32 $< -o $@
output/start16.elf:output/start16.o
	ld -T start16.ld $< -o $@
output/start16.bin:output/start16.elf
	objcopy -O binary $< $@
output/myOS/start32.o:myOS/start32.s
	cc -c -m32 $< -o $@
output/userApp/main.o:userApp/main.c myOS/myOS.h userApp/userApp.h myOS/stdio.h
	cc -c -m32 $< -o $@ -fno-stack-protector
output/myOS/printf.o:myOS/printf.c
	cc -c -m32 $< -o $@ -fno-stack-protector
output/myOS/memory.o:myOS/memory.c
	cc -c -m32 $< -o $@ -fno-stack-protector
output/userApp/init.o:userApp/init.c myOS/myOS.h
	cc -c -m32 $< -o $@ -fno-stack-protector
output/myOS/task.o:myOS/task.c myOS/myOS.h
	cc -c -m32 $< -o $@ -fno-stack-protector
output/myOS/int.o:myOS/int.c myOS/myOS.h
	cc -c -m32 $< -o $@ -fno-stack-protector
output/os.elf:output/userApp/main.o output/myOS/start32.o output/myOS/int.o output/myOS/task.o output/userApp/init.o myOS/start32.ld output/myOS/printf.o output/myOS/memory.o 
	ld -T  myOS/start32.ld output/userApp/main.o output/myOS/start32.o output/myOS/int.o output/myOS/task.o output/userApp/init.o output/myOS/printf.o output/myOS/memory.o  -o $@
output/os.bin:output/os.elf
	objcopy -O binary $< $@
new.img:
	sudo dd if=/dev/zero of=new.img bs=512 count=2880
fp.img:
	cp new.img fp.img
os:output/os.bin output/start16.bin new.img fp.img
	rm fp.img
	cp new.img fp.img
	sudo losetup /dev/loop4 fp.img
	sudo dd if=output/start16.bin of=/dev/loop4 bs=512 count=1
	sudo dd if=output/os.bin of=/dev/loop4 bs=512 seek=1
	sudo losetup -d /dev/loop4
zip:
	python3 zip.py
run:
	sudo qemu-system-i386 -fda fp.img -m 32M
dbg:
	sudo qemu-system-i386 -fda fp.img -m 32M -s -S