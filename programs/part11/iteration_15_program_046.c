/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

#include <stdio.h>
#include <stdint.h>

/* Pattern A: Force emulated TLS with compiler flag */
/* We'll compile with -ftls-model=emulated */

/* 1. DECL_PRESERVE_P - used attribute prevents elimination */
__thread int tls_preserve __attribute__((used)) = 42;
__thread int tls_not_preserve = 100;

/* 2. TREE_USED - ensure variables are referenced */
__thread int tls_used_early = 1;
__thread int tls_used_late = 2;

/* 3. TREE_PUBLIC / DECL_EXTERNAL mix */
__thread int tls_public = 10;           /* Public, non-static */
static __thread int tls_static = 20;    /* Static, not public */
extern __thread int tls_external;       /* External declaration */

/* 4. DECL_COMMON - tentative definitions */
__thread int tls_common;                /* Common symbol */
__thread int tls_common_init = 0;       /* Also common (zero-init) */
__thread int tls_not_common = 300;      /* Not common (has initializer) */

/* 5. DECL_WEAK - weak symbols */
__thread int tls_weak __attribute__((weak)) = 50;
extern __thread int tls_weak_undefined __attribute__((weak));

/* 6. DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_default __attribute__((visibility("default"))) = 60;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 70;
__thread int tls_protected __attribute__((visibility("protected"))) = 80;
#ifdef __GNUC__
__thread int tls_internal __attribute__((visibility("internal"))) = 90;
#endif

/* 7. DECL_DLLIMPORT_P - Windows-specific */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* 8. DECL_CONTEXT - different scopes */
static void function_scope(void) {
    /* Function-local TLS */
    static __thread int tls_function_local = 123;
    tls_function_local++;
}

/* Pattern C: Use in complex expressions */
void use_tls_complex(void) {
    /* Take addresses */
    int *ptr1 = &tls_public;
    int *ptr2 = &tls_hidden;
    
    /* Use in asm (if supported) to prevent optimization */
    #ifdef __GNUC__
    asm volatile("" : "+m" (tls_public));
    #endif
    
    /* Complex expressions */
    tls_used_early = tls_used_late * 2 + tls_public;
}

/* Pattern D: Mix with other TLS models */
#ifdef __GNUC__
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 999;
#endif

int main(void) {
    int sum = 0;
    
    /* Ensure TREE_USED is set for all variables */
    sum += tls_preserve;
    sum += tls_not_preserve;
    sum += tls_used_early;
    sum += tls_used_late;
    sum += tls_public;
    sum += tls_static;
    
    /* Use external TLS */
    tls_external = 30;
    sum += tls_external;
    
    /* Use common TLS */
    tls_common = 40;
    sum += tls_common;
    sum += tls_common_init;
    sum += tls_not_common;
    
    /* Use weak TLS */
    sum += tls_weak;
    /* Weak undefined is optional */
    
    /* Use visibility TLS */
    sum += tls_default;
    sum += tls_hidden;
    sum += tls_protected;
    #ifdef __GNUC__
    sum += tls_internal;
    #endif
    
    /* Use function scope */
    function_scope();
    
    /* Complex usage */
    use_tls_complex();
    sum += tls_used_early;
    
    /* Use global-dynamic model for contrast */
    #ifdef __GNUC__
    sum += tls_global_dynamic;
    #endif
    
    printf("TLS sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
