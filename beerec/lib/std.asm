extern WriteConsoleA
extern GetStdHandle

.std_print:
	push	rbp
	mov	rbp, rsp
	
	mov	rcx, -11
	call	GetStdHandle

	mov	rcx, rax
	mov	rdx, r10
	add	rdx, 8
	mov	r8d, dword ptr [r10]
	xor	r9d, r9d
	
	sub	rsp, 32
	mov	[rsp+24], 0
	call	WriteConsoleA
	add	rsp, 32

	leave
	ret