/* tls_helper.h - Simulate multi-file compilation */
#ifndef TLS_HELPER_H
#define TLS_HELPER_H

#include <setjmp.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);

/* Weak symbol to affect linkage decisions */
int weak_function(void) __attribute__((weak));

/* Volatile external variable */
extern volatile int ext_volatile;

#endif
