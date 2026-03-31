/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Basic TLS variables with different attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC - non-static at file scope */
__thread int tls_public = 100;

/* Static TLS - not public */
static __thread int tls_static = 200;

/* DECL_COMMON - tentative definition */
__thread int tls_common;  /* No initializer */

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED - hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* Default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* Protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 600;

/* Internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 700;

/* Pattern B: Extern declaration (will be defined in another file) */
extern __thread int tls_extern;

/* Pattern C: DLL import attribute (conditional) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Pattern D: Different TLS models */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 800;

/* Function-scoped TLS - tests DECL_CONTEXT */
void test_function_scope(void) {
    static __thread int tls_func_static = 900;
    tls_func_static++;
}

/* Take address to ensure TREE_USED */
void take_addresses(void) {
    int *ptr;
    
    ptr = &tls_used;
    ptr = &tls_public;
    ptr = &tls_static;
    ptr = &tls_common;
    ptr = &tls_weak;
    ptr = &tls_hidden;
    ptr = &tls_default;
    ptr = &tls_protected;
    ptr = &tls_internal;
    ptr = &tls_global_dynamic;
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(ptr) : "memory");
}

/* Complex usage patterns */
int use_tls_variables(void) {
    int sum = 0;
    
    /* Read and modify all TLS variables */
    sum += tls_used;
    tls_used++;
    
    sum += tls_public;
    tls_public += 2;
    
    sum += tls_static;
    tls_static += 3;
    
    sum += tls_common;
    tls_common = sum;  /* Initialize */
    
    sum += tls_weak;
    tls_weak += 5;
    
    sum += tls_hidden;
    tls_hidden += 6;
    
    sum += tls_default;
    tls_default += 7;
    
    sum += tls_protected;
    tls_protected += 8;
    
    sum += tls_internal;
    tls_internal += 9;
    
    sum += tls_global_dynamic;
    tls_global_dynamic += 10;
    
    /* Use extern if available */
    #ifdef TLS_EXTERN_DEFINED
    sum += tls_extern;
    tls_extern += 11;
    #endif
    
    test_function_scope();
    take_addresses();
    
    return sum;
}

int main(void) {
    int result = use_tls_variables();
    
    /* Use the variables again to ensure they persist */
    result += use_tls_variables();
    
    printf("Result: %d\n", result);
    return 0;
}
