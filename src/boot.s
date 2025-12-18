/* Multiboot header */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/* Multiboot declaration on first 8 KiB of the file */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* Declaring the stack */
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/* Entry point of stack */
.section .text
.global _start
.type _start, @function
_start:

	mov $0xb8000, %edi
	movb $'X', (%edi)
	movb $0x0f, 1(%edi)

	/* Setting up the stack */
	mov $stack_top, %esp

	/* Call c code */
	call kernel_main

    /* Nothing to do -> Infinite loop while waiting */
	cli
1:	hlt
	jmp 1b

/* Set the size of the _start symbol to the current location '.' minus its start. */
.size _start, . - _start

/*
boot.s =
  - "Hello GRUB, I’m a kernel"
  - "Here is a stack"
  - "Jump to C safely"
  - "Never return"
*/