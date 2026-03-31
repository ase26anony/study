#ifndef TLS_H
#define TLS_H

// External TLS declaration with various attributes
extern __thread int external_tls 
    __attribute__((weak, visibility("default"), used));

// Function prototypes
int __attribute__((noinline)) use_tls_variables(void);
void __attribute__((noipa)) opaque_function(void*);

#endif // TLS_H
