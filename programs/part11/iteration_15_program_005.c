/* Test for emulated TLS attribute copying - Main file */

/* Force emulated TLS model */
#pragma GCC tls_model emulated

#include <stdio.h>
#include <stdint.h>

/* DECL_PRESERVE_P: Used attribute ensures preservation */
__thread int tls_used __attribute__((used)) = 42;
__thread int tls_not_used = 0;

/* TREE_USED: Variables that will be referenced */
__thread int tls_referenced = 100;
static __thread int tls_static_referenced = 200;

/* TREE_PUBLIC: Public vs static TLS */
__thread int tls_public = 300;
static __thread int tls_static = 400;

/* DECL_COMMON: Tentative definitions (common symbols) */
__thread int tls_common;  /* Tentative definition */

/* DECL_WEAK: Weak TLS variables */
__thread int tls_weak __attribute__((weak)) = 500;
__thread int tls_strong = 600;

/* DECL_VISIBILITY: Different visibility attributes */
__thread int tls_default __attribute__((visibility("default"))) = 700;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 800;
__thread int tls_protected __attribute__((visibility("protected"))) = 900;
__thread int tls_internal __attribute__((visibility("internal"))) = 1000;

/* DECL_EXTERNAL: External declaration (defined in another file) */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: DLL import attribute (target-specific) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Complex usage to ensure processing */
__thread int* tls_pointer;
__thread struct { int a; double b; } tls_struct = { 1100, 3.14 };

/* Function with local static TLS */
static void use_local_tls(void) {
    static __thread int local_static_tls = 1200;
    local_static_tls++;
    tls_referenced += local_static_tls;
}

/* Force address-taking and complex expressions */
void take_addresses(void) {
    tls_pointer = &tls_referenced;
    
    /* Use in inline asm to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_public) : "memory");
    __asm__ volatile ("" : : "r"(&tls_hidden) : "memory");
}

int main(void) {
    int sum = 0;
    
    /* Ensure all TLS variables are TREE_USED */
    sum += tls_used;
    sum += tls_not_used;
    sum += tls_referenced;
    sum += tls_static_referenced;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_strong;
    sum += tls_default;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_external;
    
#ifdef _WIN32
    sum += tls_dllimport;
#endif
    
    sum += tls_struct.a;
    
    /* Modify values */
    tls_common = 1300;
    tls_not_used = 1400;
    
    /* Use local TLS function */
    use_local_tls();
    
    /* Take addresses */
    take_addresses();
    
    /* Use different TLS models for contrast */
    __thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 1500;
    sum += tls_global_dynamic;
    
    printf("TLS sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
