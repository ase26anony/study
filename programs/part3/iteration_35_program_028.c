/* Test for EMUTLS attribute copying - Main file */
#include <stdio.h>

/* Force EMUTLS transformation by using non-TLS-supporting target flags */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

/* 1. DECL_PRESERVE_P: TLS variable with 'used' attribute */
__thread int tls_used __attribute__((used)) = 42;

/* 2. TREE_PUBLIC: Non-static (public) TLS variable */
__thread int tls_public = 100;

/* 3. TREE_PUBLIC: Static (non-public) TLS variable */
static __thread int tls_static = 200;

/* 4. DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;  /* No initializer */

/* 5. DECL_WEAK: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* 6. DECL_VISIBILITY: TLS variable with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* 7. DECL_VISIBILITY: TLS variable with default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* 8. DECL_CONTEXT: TLS variable inside a function (local scope) */
void function_with_tls(void) {
    static __thread int tls_in_function = 600;
    tls_in_function++;  /* Ensure TREE_USED */
}

/* 9. DECL_DLLIMPORT_P: Simulate DLL import (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* For non-Windows, we'll simulate with an external declaration */
extern __thread int tls_external;
#endif

/* External TLS variable declaration (for DECL_EXTERNAL) */
extern __thread int tls_external_def;

/* Function that uses all TLS variables to ensure TREE_USED is set */
void use_all_tls_variables(void) {
    /* Reference each TLS variable to mark them as TREE_USED */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 50;
    tls_common = 999;
    tls_weak = tls_weak / 3;
    tls_hidden = tls_hidden + 1000;
    tls_default = tls_default - 100;
    
    function_with_tls();
    
#ifdef _WIN32
    tls_imported = 777;
#else
    tls_external = 888;
#endif
    
    tls_external_def = 1111;
    
    /* Print addresses to prevent optimization */
    printf("TLS addresses: %p %p %p %p\n", 
           (void*)&tls_used, 
           (void*)&tls_public,
           (void*)&tls_hidden,
           (void*)&tls_default);
}

int main(void) {
    use_all_tls_variables();
    
    /* Additional uses to ensure variables are marked used */
    tls_common = 1234;
    tls_weak = 5678;
    
    return 0;
}
