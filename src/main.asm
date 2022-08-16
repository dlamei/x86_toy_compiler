global main
extern printf

section .data

section .text

main:
	push ebp
	mov ebp, esp

	mov eax, 1
	mov ebx, 2
	int 0x80
