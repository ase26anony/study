/* Test for emulated TLS attribute copying - C version */

/* Force emulated TLS for coverage */
#pragma GCC tls_model emulated

#include <stdio.h>
#include <stdint.h>

/* Pattern A: Variables with DECL_PRESERVE_P set via __attribute__((used)) */
__thread int tls_used_attr __attribute__((used)) = 42;
__thread int tls_unused = 100;  /* No used attribute */

/* Pattern B: Different scopes for DECL_CONTEXT */
__thread int tls_file_scope = 1;
static __thread int tls_static_file_scope = 2;

/* Function with local static TLS */
void test_local_tls(void) {
    static __thread int tls_local_static = 3;
    tls_local_static++;
}

/* Pattern C: TREE_USED - ensure variables are referenced */
__thread int tls_used1 = 10;
__thread int tls_used2 = 20;

/* Pattern D: TREE_PUBLIC and DECL_EXTERNAL */
__thread int tls_public = 30;           /* Public, non-static */
static __thread int tls_non_public = 40; /* Static, not public */

/* Pattern E: DECL_COMMON - tentative definitions */
__thread int tls_common;  /* Tentative definition - should be COMMON */

/* Pattern F: DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 50;
__thread int tls_strong = 60;  /* Strong definition */

/* Pattern G: Visibility attributes */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 70;
__thread int tls_default __attribute__((visibility("default"))) = 80;
__thread int tls_protected __attribute__((visibility("protected"))) = 90;
__thread int tls_internal __attribute__((visibility("internal"))) = 100;

/* Pattern H: DLL import/export (target-specific) */
#ifdef _WIN32
__declspec(dllimport) extern __thread int tls_imported;
__declspec(dllexport) __thread int tls_exported = 110;
#elif defined(__CYGWIN__) || defined(__MINGW32__)
__attribute__((dllimport)) extern __thread int tls_imported;
__attribute__((dllexport)) __thread int tls_exported = 110;
#else
/* On non-Windows, use visibility for similar effect */
extern __thread int tls_imported;
__thread int tls_exported __attribute__((visibility("default"))) = 110;
#endif

/* Pattern I: Different TLS models mixed */
__thread int tls_emulated = 120;  /* Will use emulated due to -ftls-model=emulated */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 130;

/* Complex usage patterns to ensure processing */
__thread int* tls_pointer;
__thread struct {
    int a;
    int b;
} tls_struct = {140, 150};

/* Function that takes address of TLS variables */
void take_addresses(void) {
    tls_pointer = &tls_used1;
    
    /* Use in inline asm to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_used2) : "memory");
    
    /* Complex expression with TLS */
    tls_used1 = tls_used2 * 2 + tls_public;
}

/* Weak alias test */
extern __thread int tls_weak_alias __attribute__((weak, alias("tls_weak")));

int main(void) {
    int sum = 0;
    
    /* Ensure all TLS variables are used (TREE_USED) */
    sum += tls_used_attr;
    sum += tls_unused;
    sum += tls_file_scope;
    sum += tls_static_file_scope;
    
    test_local_tls();
    
    sum += tls_used1;
    sum += tls_used2;
    sum += tls_public;
    sum += tls_non_public;
    
    tls_common = 200;  /* Define the common variable */
    sum += tls_common;
    
    sum += tls_weak;
    sum += tls_strong;
    
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_protected;
    sum += tls_internal;
    
    sum += tls_exported;
    
    sum += tls_emulated;
    sum += tls_global_dynamic;
    
    take_addresses();
    sum += *tls_pointer;
    
    sum += tls_struct.a;
    sum += tls_struct.b;
    
    /* Use weak alias */
    sum += tls_weak_alias;
    
    /* Print to prevent optimization */
    printf("TLS sum: %d\n", sum);
    
    /* Additional complex usage */
    __thread int tls_local_main = 999;
    sum += tls_local_main;
    
    /* Take address and pass to external function */
    extern void use_tls_ptr(int*);
    use_tls_ptr(&tls_local_main);
    
    return sum > 0 ? 0 : 1;
}
