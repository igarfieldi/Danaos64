/**
 * cpp/x86/crti.s
 * 
 * '_init' and '_fini' provide the framework for the code generated by gcc responsible
 * for calling constructors/destructors of global objects etc.
**/

.global _init, _fini

.section .init
.type _init, @function
_init:
	push %ebp
	movl %esp, %ebp
	/* Content of .init in crtbegin.o */

.section .fini
.type _fini, @function
_fini:
	push %ebp
	movl %esp, %ebp
	/* Content of .fini in crtbegin.o */
