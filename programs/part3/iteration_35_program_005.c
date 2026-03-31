/* Test for EMUTLS attribute copying - Main file */
#include <stdio.h>

/* Force EMUTLS transformation by targeting architecture without native TLS */
#ifdef __ARM_ARCH
#undef __ARM_ARCH
#endif

/* DECL_PRESERVE_P: TLS variable with 'used' attribute */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: Non-static (public) TLS variable */
__thread int tls_public = 100;

/* TREE_PUBLIC: Static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY: TLS variable with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: TLS variable with default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
void function_with_tls(void) {
    __thread int tls_local = 600;
    tls_local++;  /* Ensure TREE_USED */
}

/* DECL_EXTERNAL: External TLS variable declaration */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: DLL import (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Helper function to use all TLS variables */
void use_all_tls_variables(void) {
    /* TREE_USED: Reference all TLS variables */
    tls_used++;
    tls_public++;
    tls_static++;
    tls_common = 700;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    tls_external++;
    
#ifdef _WIN32
    tls_dllimport++;
#endif
    
    printf("tls_used: %d\n", tls_used);
    printf("tls_public: %d\n", tls_public);
    printf("tls_static: %d\n", tls_static);
    printf("tls_common: %d\n", tls_common);
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_default: %d\n", tls_default);
    printf("tls_external: %d\n", tls_external);
}

int main(void) {
    /* Call function with local TLS */
    function_with_tls();
    
    /* Use all TLS variables */
    use_all_tls_variables();
    
    /* Additional usage to ensure TREE_USED is set */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_external;
    
    return sum > 0 ? 0 : 1;
}
