/* Test for emulated TLS attribute copying coverage */
#include <stdio.h>
#include <stddef.h>

/* Pattern A: Force emulated TLS with compiler flag */
/* We'll compile with -ftls-model=emulated */

/* DECL_PRESERVE_P: Variables that should not be eliminated */
__thread int tls_preserve __attribute__((used));
__thread int tls_not_preserve;

/* TREE_USED: Variables that are referenced */
__thread int tls_used = 42;
static __thread int tls_unused;

/* TREE_PUBLIC/DECL_EXTERNAL: Mix of public and static */
__thread int tls_public = 100;
static __thread int tls_static = 200;

/* DECL_COMMON: Tentative definitions */
__thread int tls_common;  /* Should become common symbol */

/* DECL_WEAK: Weak symbols */
__thread int tls_weak __attribute__((weak)) = 300;
extern __thread int tls_weak_extern __attribute__((weak));

/* DECL_VISIBILITY: Different visibility attributes */
__thread int tls_default __attribute__((visibility("default"))) = 400;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 500;
__thread int tls_protected __attribute__((visibility("protected"))) = 600;
#ifdef __GNUC__
__thread int tls_internal __attribute__((visibility("internal"))) = 700;
#endif

/* DECL_DLLIMPORT_P: DLL import attributes (Windows-specific) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_CONTEXT: Different scopes */
static void func_with_tls(void) {
    /* Function scope TLS */
    static __thread int tls_func_scope = 800;
    tls_func_scope++;
}

/* Complex usage patterns to ensure processing */
__thread int* tls_ptr;
__thread int tls_array[10];

/* Use in asm statement to prevent optimization */
static void use_in_asm(void) {
    __thread int tls_asm;
    __asm__ volatile ("" : : "r"(&tls_asm));
}

/* Different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 900;

int main(void) {
    int sum = 0;
    
    /* Ensure TREE_USED is set for these variables */
    tls_preserve = 1;
    tls_not_preserve = 2;
    sum += tls_preserve + tls_not_preserve;
    
    /* Reference used variables */
    sum += tls_used;
    tls_unused = 3;  /* Now it's used */
    sum += tls_unused;
    
    /* Public and static TLS */
    sum += tls_public;
    sum += tls_static;
    
    /* Common TLS */
    tls_common = 4;
    sum += tls_common;
    
    /* Weak TLS */
    if (&tls_weak) {
        sum += tls_weak;
    }
    
    /* Visibility TLS variables */
    sum += tls_default;
    sum += tls_hidden;
    sum += tls_protected;
#ifdef __GNUC__
    sum += tls_internal;
#endif
    
    /* Function scope TLS */
    func_with_tls();
    
    /* Complex usage */
    tls_ptr = &tls_public;
    sum += *tls_ptr;
    
    for (int i = 0; i < 10; i++) {
        tls_array[i] = i;
        sum += tls_array[i];
    }
    
    /* Use in asm */
    use_in_asm();
    
    /* Global dynamic for contrast */
    sum += tls_global_dynamic;
    
    printf("TLS sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
