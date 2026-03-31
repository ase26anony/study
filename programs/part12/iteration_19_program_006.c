#ifndef TLS_H
#define TLS_H

// External TLS declaration with various attributes
extern __thread int external_tls 
    __attribute__((weak, visibility("default")));

// Function prototypes
int get_checksum(void) __attribute__((noinline));
void use_tls_addresses(void) __attribute__((noinline));

// Opaque function to prevent optimization
void opaque_func(void*) __attribute__((noipa));

#endif // TLS_H
