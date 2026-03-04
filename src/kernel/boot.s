[bits 32]

section .text.entry
global _start
extern kernel_main

_start:
    cli
    
    ; Setup Stack (0x90000 is safe)
    mov esp, 0x90000
    
    ; We expect eax = magic, ebx = info (passed from Stage 2)
    push ebx          ; arg2: info
    push eax          ; arg1: magic
    
    call kernel_main

    ; Should not return
    cli
.hang:
    hlt
    jmp .hang

section .multiboot alloc
align 4
	dd 0x1BADB002
	dd 0x00000007
	dd -(0x1BADB002 + 0x00000007)
	dd 0, 0, 0, 0, 0
	dd 0
	dd 1024
	dd 768
	dd 32

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
