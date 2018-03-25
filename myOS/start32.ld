/*
 * start16.ld
 *
 * Linker script for the i386 bootsect code
 */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)


ENTRY(_start32)

SECTIONS
{
	. = 0x7e00;
	.bstext		: { *(.bstext) }
	.=ALIGN(16);
	.bsdata         : { *(.bsdata) }
	.=ALIGN(16);
	.bss            : { *(.bss) }
	.=ALIGN(16);
}
