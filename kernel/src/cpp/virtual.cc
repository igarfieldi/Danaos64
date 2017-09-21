/**
 * cpp/virtual.cc
 *
 * Contains the (empty) definitions for some of the functions required by g++.
**/

// TODO: Trigger kernel panic if one of these occur?

// Function for invalid virtual calls
extern "C" void __cxa_pure_virtual() {
}

extern "C" void __cxa__virtual() {
}

// We don't want to call anything at kernel exit (yet)
extern "C" void __cxa_atexit() {
}

namespace __cxxabiv1 
{
    // TODO: these are only stubs!
    // The ABI requires a 64-bit type.
	__extension__ typedef int __guard __attribute__((mode(__DI__)));
 
	extern "C" int __cxa_guard_acquire (__guard *g) 
	{
		return !*(char *)(g);
	}
 
	extern "C" void __cxa_guard_release (__guard *g)
	{
		*(char *)g = 1;
	}
 
	extern "C" void __cxa_guard_abort (__guard *)
	{
 
	}
}
