/**
 * cpp/x86_64/crtn.s
 * 
 * '_init' and '_fini' provide the framework for the code generated by gcc responsible
 * for calling constructors/destructors of global objects etc.
**/

/* x86_64 crtn.s */
.section .init
	/* Content of .init in crtend.o */
	popq %rbp
	ret

.section .fini
	/* Content of .fini in crtend.o */
	popq %rbp
	ret
