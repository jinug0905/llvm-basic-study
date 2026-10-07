	.build_version macos, 28, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:                                ; %entry
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	mov	w8, #1                          ; =0x1
	str	wzr, [sp, #12]
LBB0_1:                                 ; %for.cond
                                        ; =>This Inner Loop Header: Depth=1
	cmp	w8, #10
	str	w8, [sp, #8]
	b.gt	LBB0_3
; %bb.2:                                ; %for.body
                                        ;   in Loop: Header=BB0_1 Depth=1
	ldp	w9, w8, [sp, #8]
	add	w8, w8, w9
	str	w8, [sp, #12]
	add	w8, w9, #1
	b	LBB0_1
LBB0_3:                                 ; %for.end
	mov	w8, #1                          ; =0x1
	stp	w8, w8, [sp]
LBB0_4:                                 ; %while.cond
                                        ; =>This Inner Loop Header: Depth=1
	ldr	w8, [sp, #4]
	cmp	w8, #5
	b.gt	LBB0_6
; %bb.5:                                ; %while.body
                                        ;   in Loop: Header=BB0_4 Depth=1
	ldp	w8, w9, [sp]
	mul	w8, w8, w9
	add	w9, w9, #1
	stp	w8, w9, [sp]
	b	LBB0_4
LBB0_6:                                 ; %while.end
	ldr	w8, [sp, #12]
	ldr	w9, [sp], #16
	add	w0, w8, w9
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
