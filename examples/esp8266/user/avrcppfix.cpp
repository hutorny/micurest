/*
 * avrcppfix.cpp
 * Fixes missing dependencies, not implemented in avr-gcc
 * and not used by cojson tests
 */
#include <stddef.h>


typedef int __guard __attribute__((mode (__DI__)));

extern "C" int __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release (__guard *);
extern "C" void __cxa_guard_abort (__guard *);
extern "C" void __cxa_pure_virtual(void);
int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);}
void __cxa_guard_release (__guard *g) {*(char *)g = 1;}
void __cxa_guard_abort (__guard *) {}
void __cxa_pure_virtual(void) __attribute__((weak));
void __cxa_pure_virtual(void) {}

/* this is just a stub, must never be used */
void * operator new(size_t) __attribute__((weak));
void * operator new(size_t size) {
  return (void*)0x100;
}

void operator delete(void *) __attribute__((weak));
void operator delete(void * ptr) { }
extern "C" void	atexit( void ) __attribute__((weak));
extern "C" void	atexit( void ) { }


