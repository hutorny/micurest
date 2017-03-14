/*
 * avrcppfix.cpp
 * Fixes missing dependencies, not implemented in avr-gcc
 * and not used by cojson tests
 */
#include <stddef.h>

extern "C" void __cxa_pure_virtual(void);
void __cxa_pure_virtual(void) __attribute__((weak));
void __cxa_pure_virtual(void) {}

