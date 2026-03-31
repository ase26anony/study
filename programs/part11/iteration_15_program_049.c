/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Basic TLS variables with various attributes */
__thread int tls_basic = 42;  /* TREE_PUBLIC, DECL_COMMON? (has initializer) */
static __thread int tls_static = 100;  /* Not TREE_PUBLIC */
__thread int tls_common;  /* DECL_COMMON - tentative definition */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 200;

/* DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 300;

/* Visibility attributes */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;
__thread int tls_protected __attribute__((visibility("protected"))) = 500;
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* DECL_DLLIMPORT_P - target specific */
#ifdef _WIN32
extern __thread int tls_imported __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_imported __attribute__((dllimport));
#endif

/* Function to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Read and write to all TLS variables */
    tls_basic += 1;
    tls_static += 2;
    tls_common += 3;
    tls_used += 4;
    tls_weak += 5;
    tls_hidden += 6;
    tls_protected += 7;
    tls_internal += 8;
    
    /* Take addresses - forces more complex handling */
    int *ptr1 = &tls_basic;
    int *ptr2 = &tls_static;
    int *ptr3 = &tls_common;
    (void)ptr1; (void)ptr2; (void)ptr3;
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(tls_basic), "r"(tls_static));
}

/* DECL_CONTEXT test - TLS in function scope */
void func_with_tls(void) {
    static __thread int tls_in_func = 900;
    tls_in_func++;
    __asm__ volatile ("" : : "r"(tls_in_func));
}

/* Pattern D: Different TLS models */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 1000;
__thread int tls_emulated __attribute__((tls_model("emulated"))) = 1100;

/* Weak alias test */
__thread int tls_original = 1200;
extern __thread int tls_alias __attribute__((weak, alias("tls_original")));

int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    use_tls_variables();
    func_with_tls();
    
    /* Complex usage patterns */
    sum += tls_basic;
    sum += tls_static;
    sum += tls_common;
    sum += tls_used;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_global_dynamic;
    sum += tls_emulated;
    sum += tls_original;
    sum += tls_alias;
    
    /* Use in non-inlinable function call */
    printf("TLS sum: %d\n", sum);
    
    /* Ensure variables aren't optimized away */
    __asm__ volatile ("" : : "r"(tls_basic), "r"(tls_static), "r"(tls_common));
    
    return sum > 0 ? 0 : 1;
}
