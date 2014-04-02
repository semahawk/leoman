	.file	"kernel_path.i"
	.text
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	pushq	%rbp
.Ltmp2:
	.cfi_def_cfa_offset 16
.Ltmp3:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp4:
	.cfi_def_cfa_register %rbp
	subq	$448, %rsp              # imm = 0x1C0
	leaq	.L.str, %rdi
	leaq	-272(%rbp), %rax
	movl	$0, %esi
	movabsq	$256, %rdx              # imm = 0x100
	leaq	kernel_path, %rcx
	movl	$0, -4(%rbp)
	movq	%rcx, -16(%rbp)
	movq	%rax, %rcx
	movq	%rdi, -288(%rbp)        # 8-byte Spill
	movq	%rcx, %rdi
	movq	%rax, -296(%rbp)        # 8-byte Spill
	callq	memset
	movq	-296(%rbp), %rax        # 8-byte Reload
	movq	%rax, -280(%rbp)
	#APP
	nop
nop
nop
nop

	#NO_APP
	movq	-288(%rbp), %rdi        # 8-byte Reload
	movb	$0, %al
	callq	printf
	movl	%eax, -300(%rbp)        # 4-byte Spill
.LBB0_1:                                # %while.cond
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_3 Depth 2
	movq	-16(%rbp), %rax
	movq	%rax, %rcx
	addq	$1, %rcx
	movq	%rcx, -16(%rbp)
	movsbl	(%rax), %edx
	cmpl	$0, %edx
	je	.LBB0_9
# BB#2:                                 # %while.body
                                        #   in Loop: Header=BB0_1 Depth=1
	leaq	.L.str1, %rdi
	movl	i, %esi
	movb	$0, %al
	callq	printf
	leaq	.L.str2, %rdi
	movl	%eax, -304(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str3, %rdi
	leaq	-272(%rbp), %rcx
	movl	$0, %esi
	movabsq	$256, %rdx              # imm = 0x100
	movq	%rcx, %r8
	movq	%rdi, -312(%rbp)        # 8-byte Spill
	movq	%r8, %rdi
	movl	%eax, -316(%rbp)        # 4-byte Spill
	movq	%rcx, -328(%rbp)        # 8-byte Spill
	callq	memset
	movq	-328(%rbp), %rcx        # 8-byte Reload
	movq	%rcx, -280(%rbp)
	movq	-312(%rbp), %rdi        # 8-byte Reload
	movb	$0, %al
	callq	printf
	movl	%eax, -332(%rbp)        # 4-byte Spill
.LBB0_3:                                # %for.cond
                                        #   Parent Loop BB0_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movb	$0, %al
	movq	-16(%rbp), %rcx
	movsbl	(%rcx), %edx
	cmpl	$0, %edx
	movb	%al, -333(%rbp)         # 1-byte Spill
	je	.LBB0_5
# BB#4:                                 # %land.rhs
                                        #   in Loop: Header=BB0_3 Depth=2
	movq	-16(%rbp), %rax
	movsbl	(%rax), %ecx
	cmpl	$47, %ecx
	setne	%dl
	movb	%dl, -333(%rbp)         # 1-byte Spill
.LBB0_5:                                # %land.end
                                        #   in Loop: Header=BB0_3 Depth=2
	movb	-333(%rbp), %al         # 1-byte Reload
	testb	$1, %al
	jne	.LBB0_6
	jmp	.LBB0_8
.LBB0_6:                                # %for.body
                                        #   in Loop: Header=BB0_3 Depth=2
	movq	-16(%rbp), %rax
	movb	(%rax), %cl
	movq	-280(%rbp), %rax
	movq	%rax, %rdx
	addq	$1, %rdx
	movq	%rdx, -280(%rbp)
	movb	%cl, (%rax)
# BB#7:                                 # %for.inc
                                        #   in Loop: Header=BB0_3 Depth=2
	movq	-16(%rbp), %rax
	addq	$1, %rax
	movq	%rax, -16(%rbp)
	jmp	.LBB0_3
.LBB0_8:                                # %for.end
                                        #   in Loop: Header=BB0_1 Depth=1
	leaq	.L.str4, %rdi
	leaq	-272(%rbp), %rsi
	movb	$0, %al
	callq	printf
	leaq	.L.str5, %rdi
	movl	%eax, -340(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str6, %rdi
	movl	%eax, -344(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str7, %rdi
	movl	%eax, -348(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str8, %rdi
	movl	%eax, -352(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str9, %rdi
	movl	%eax, -356(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str10, %rdi
	movl	%eax, -360(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str11, %rdi
	movl	%eax, -364(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str12, %rdi
	movl	%eax, -368(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str13, %rdi
	leaq	kernel_path, %rsi
	movl	%eax, -372(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str14, %rdi
	movl	%eax, -376(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str15, %rdi
	movl	%eax, -380(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str16, %rdi
	movl	%eax, -384(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str10, %rdi
	movl	%eax, -388(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str17, %rdi
	movl	i, %ecx
	addl	$1, %ecx
	movl	%ecx, %esi
	movl	%eax, -392(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str12, %rdi
	movl	%eax, -396(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str18, %rdi
	leaq	-272(%rbp), %rsi
	movl	%eax, -400(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str14, %rdi
	movl	%eax, -404(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str19, %rdi
	movl	%eax, -408(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str20, %rdi
	movl	%eax, -412(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str21, %rdi
	movl	%eax, -416(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str22, %rdi
	movl	%eax, -420(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str15, %rdi
	movl	%eax, -424(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str23, %rdi
	movl	%eax, -428(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str24, %rdi
	movl	%eax, -432(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	leaq	.L.str25, %rdi
	movl	%eax, -436(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	movl	i, %ecx
	addl	$1, %ecx
	movl	%ecx, i
	movl	%eax, -440(%rbp)        # 4-byte Spill
	jmp	.LBB0_1
.LBB0_9:                                # %while.end
	leaq	.L.str26, %rdi
	movb	$0, %al
	callq	printf
	leaq	.L.str27, %rdi
	movl	%eax, -444(%rbp)        # 4-byte Spill
	movb	$0, %al
	callq	printf
	movl	$0, %ecx
	movl	%eax, -448(%rbp)        # 4-byte Spill
	movl	%ecx, %eax
	addq	$448, %rsp              # imm = 0x1C0
	popq	%rbp
	ret
.Ltmp5:
	.size	main, .Ltmp5-main
	.cfi_endproc

	.type	kernel_path,@object     # @kernel_path
	.data
	.align	16
kernel_path:
	.asciz	 "/boot/kernel/kernel"
	.size	kernel_path, 20

	.type	.L.str,@object          # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	 "load inode 0x02 into the memory\n"
	.size	.L.str, 33

	.type	.L.str1,@object         # @.str1
.L.str1:
	.asciz	 "seek%u:\n"
	.size	.L.str1, 9

	.type	i,@object               # @i
	.local	i
	.comm	i,4,4
	.type	.L.str2,@object         # @.str2
.L.str2:
	.asciz	 "  zero-out the name buffer\n"
	.size	.L.str2, 28

	.type	.L.str3,@object         # @.str3
.L.str3:
	.asciz	 "  fetch the next path segment into the name buffer\n"
	.size	.L.str3, 52

	.type	.L.str4,@object         # @.str4
.L.str4:
	.asciz	 "  begin search for '%s'\n"
	.size	.L.str4, 25

	.type	.L.str5,@object         # @.str5
.L.str5:
	.asciz	 "  load the first block of the inode which is currently in the memory\n"
	.size	.L.str5, 70

	.type	.L.str6,@object         # @.str6
.L.str6:
	.asciz	 "  traverse:\n"
	.size	.L.str6, 13

	.type	.L.str7,@object         # @.str7
.L.str7:
	.asciz	 "    if found\n"
	.size	.L.str7, 14

	.type	.L.str8,@object         # @.str8
.L.str8:
	.asciz	 "      if last segment (ie. *p == '\\0')\n"
	.size	.L.str8, 40

	.type	.L.str9,@object         # @.str9
.L.str9:
	.asciz	 "        if is a file\n"
	.size	.L.str9, 22

	.type	.L.str10,@object        # @.str10
.L.str10:
	.asciz	 "          load it's inode in place of the previous one\n"
	.size	.L.str10, 56

	.type	.L.str11,@object        # @.str11
.L.str11:
	.asciz	 "          jmp found\n"
	.size	.L.str11, 21

	.type	.L.str12,@object        # @.str12
.L.str12:
	.asciz	 "        else\n"
	.size	.L.str12, 14

	.type	.L.str13,@object        # @.str13
.L.str13:
	.asciz	 "          print '%s is not a regular file'\n"
	.size	.L.str13, 44

	.type	.L.str14,@object        # @.str14
.L.str14:
	.asciz	 "          jmp halt\n"
	.size	.L.str14, 20

	.type	.L.str15,@object        # @.str15
.L.str15:
	.asciz	 "      else\n"
	.size	.L.str15, 12

	.type	.L.str16,@object        # @.str16
.L.str16:
	.asciz	 "        if is a directory\n"
	.size	.L.str16, 27

	.type	.L.str17,@object        # @.str17
.L.str17:
	.asciz	 "          jmp seek%u\n"
	.size	.L.str17, 22

	.type	.L.str18,@object        # @.str18
.L.str18:
	.asciz	 "          print '%s is not a directory'\n"
	.size	.L.str18, 41

	.type	.L.str19,@object        # @.str19
.L.str19:
	.asciz	 "    else\n"
	.size	.L.str19, 10

	.type	.L.str20,@object        # @.str20
.L.str20:
	.asciz	 "      if this was the last block\n"
	.size	.L.str20, 34

	.type	.L.str21,@object        # @.str21
.L.str21:
	.asciz	 "        print kernel not found!\n"
	.size	.L.str21, 33

	.type	.L.str22,@object        # @.str22
.L.str22:
	.asciz	 "        jmp halt\n"
	.size	.L.str22, 18

	.type	.L.str23,@object        # @.str23
.L.str23:
	.asciz	 "        load the next block into the memory\n"
	.size	.L.str23, 45

	.type	.L.str24,@object        # @.str24
.L.str24:
	.asciz	 "        jmp traverse\n"
	.size	.L.str24, 22

	.type	.L.str25,@object        # @.str25
.L.str25:
	.asciz	 "\n"
	.size	.L.str25, 2

	.type	.L.str26,@object        # @.str26
.L.str26:
	.asciz	 "found:\n"
	.size	.L.str26, 8

	.type	.L.str27,@object        # @.str27
.L.str27:
	.asciz	 "  given the inode structure in the memory, fetch all the datablocks and store them in memory\n"
	.size	.L.str27, 94


	.section	".note.GNU-stack","",@progbits
