; Wrapper around cpuid instruction for x64 since 64-bit MSVC compiler does not
; support inline assembly.

; void cpuid64(unsigned long func,unsigned long arg,unsigned long *words);
; func => rcx
; arg => rdx
; words => r8

PUBLIC cpuid64
.CODE
	ALIGN 8
cpuid64	PROC FRAME
	sub rsp,32
	.allocstack 32
	push rbx
	.pushreg rbx
	.endprolog

	; eax is lower part of rax.
	mov rax,rcx
	; ecx is lower part of rcx.
	mov rcx,rdx
	cpuid
	mov DWORD PTR [r8 +  0],eax
	mov DWORD PTR [r8 +  4],ebx
	mov DWORD PTR [r8 +  8],ecx
	mov DWORD PTR [r8 + 12],edx

	pop rbx
	add rsp,32
        
	ret
	ALIGN 8
cpuid64 ENDP

END