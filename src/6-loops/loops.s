	.build_version macos, 27, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:                                ; %entry
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	str	wzr, [sp, #28]
	str	wzr, [sp, #24]
	mov	w8, #1                          ; =0x1
	str	w8, [sp, #20]
LBB0_1:                                 ; %for.cond
                                        ; =>This Inner Loop Header: Depth=1
	ldr	w8, [sp, #20]
	cmp	w8, #10
	b.gt	LBB0_4
; %bb.2:                                ; %for.body
                                        ;   in Loop: Header=BB0_1 Depth=1
	ldr	w8, [sp, #24]
	ldr	w9, [sp, #20]
	add	w8, w8, w9
	str	w8, [sp, #24]
; %bb.3:                                ; %for.inc
                                        ;   in Loop: Header=BB0_1 Depth=1
	ldr	w8, [sp, #20]
	add	w8, w8, #1
	str	w8, [sp, #20]
	b	LBB0_1
LBB0_4:                                 ; %for.end
	mov	w8, #1                          ; =0x1
	str	w8, [sp, #16]
	mov	w8, #1                          ; =0x1
	str	w8, [sp, #12]
LBB0_5:                                 ; %while.cond
                                        ; =>This Inner Loop Header: Depth=1
	ldr	w8, [sp, #16]
	cmp	w8, #5
	b.gt	LBB0_7
; %bb.6:                                ; %while.body
                                        ;   in Loop: Header=BB0_5 Depth=1
	ldr	w8, [sp, #12]
	ldr	w9, [sp, #16]
	mul	w8, w8, w9
	str	w8, [sp, #12]
	ldr	w8, [sp, #16]
	add	w8, w8, #1
	str	w8, [sp, #16]
	b	LBB0_5
LBB0_7:                                 ; %while.end
	ldr	w8, [sp, #24]
	ldr	w9, [sp, #12]
	add	w0, w8, w9
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
