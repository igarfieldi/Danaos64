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
