#ifndef SHARED_H
#define SHARED_H

// Extern TLS declaration that will be defined in tls.c
extern __thread int tls_extern_var __attribute__((visibility("default")));

// Function prototype
void tls_operations(int n);

#endif
