/*
 * start16.ld
 *
 * Linker script for the i386 bootsect code
 */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
/*ENTRY(bootsect_start)*/

SECTIONS
{
	. = 0x7C00;
	.text		: { *(.text) }
	.CODE32         : { *(.CODE32) }
	.data		: { *(.data) }
	.DATA32         : { *(.DATA32) }

	. = 0x7c00 + 510;
	.signature		: { *(.signature) }
}
