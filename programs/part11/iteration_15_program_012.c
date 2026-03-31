/* tls_main.c - Main test file for emulated TLS attribute coverage */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS for all variables */
#pragma GCC tls_model emulated

/* DECL_PRESERVE_P: Variables that should not be eliminated */
__thread int tls_preserve_used __attribute__((used));
__thread int tls_preserve_regular;

/* TREE_USED: Variables that will be referenced */
__thread int tls_used_var = 42;
__thread int tls_unused_var;

/* TREE_PUBLIC: Public (non-static) TLS variables */
__thread int tls_public_var = 100;
static __thread int tls_static_var = 200;

/* DECL_COMMON: Tentative definitions (common symbols) */
__thread int tls_common;  /* No initializer at file scope */

/* DECL_WEAK: Weak TLS variables */
__thread int tls_weak_var __attribute__((weak)) = 300;

/* DECL_VISIBILITY: Different visibility attributes */
__thread int tls_default_vis __attribute__((visibility("default"))) = 400;
__thread int tls_hidden_vis __attribute__((visibility("hidden"))) = 500;
__thread int tls_protected_vis __attribute__((visibility("protected"))) = 600;

/* DECL_EXTERNAL: External declaration (defined in another file) */
extern __thread int tls_external_var;

/* DECL_DLLIMPORT_P: DLL import attribute (conditional) */
#ifdef _WIN32
extern __thread int tls_dllimport_var __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_dllimport_var __attribute__((dllimport));
#endif

/* Function to ensure TREE_USED is set for all variables */
void use_all_tls_vars(void) {
    /* Use preserve variables */
    tls_preserve_used = 1;
    tls_preserve_regular = 2;
    
    /* Use used variables */
    int val = tls_used_var;
    tls_unused_var = val + 1;
    
    /* Use public/static variables */
    tls_public_var++;
    tls_static_var--;
    
    /* Use common variable */
    tls_common = 123;
    
    /* Use weak variable */
    if (&tls_weak_var != NULL) {
        tls_weak_var = 456;
    }
    
    /* Use visibility variables */
    tls_default_vis = tls_hidden_vis + tls_protected_vis;
    
    /* Use external variable */
    tls_external_var = 789;
    
    /* Use dllimport variable if available */
#ifdef TLS_DLLIMPORT_AVAILABLE
    tls_dllimport_var = 999;
#endif
}

/* Complex expressions with TLS variables */
int* get_tls_address(void) {
    /* Taking addresses ensures complex processing */
    static int* addresses[10];
    addresses[0] = &tls_public_var;
    addresses[1] = &tls_static_var;
    addresses[2] = &tls_common;
    addresses[3] = &tls_weak_var;
    return addresses[0];
}

/* Inline assembly using TLS variables (triggers special handling) */
void asm_use_tls(void) {
    int result;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (result)
        : "m" (tls_used_var)
        : "%eax"
    );
    tls_used_var = result;
}

int main(void) {
    /* Initialize all TLS variables */
    tls_preserve_used = 10;
    tls_preserve_regular = 20;
    tls_used_var = 30;
    tls_unused_var = 40;
    tls_public_var = 50;
    tls_static_var = 60;
    tls_common = 70;
    tls_weak_var = 80;
    tls_default_vis = 90;
    tls_hidden_vis = 100;
    tls_protected_vis = 110;
    
    /* Use external variable */
    tls_external_var = 120;
    
    /* Call functions that use TLS variables */
    use_all_tls_vars();
    get_tls_address();
    asm_use_tls();
    
    /* Calculate sum for observable output */
    int sum = 
        tls_preserve_used +
        tls_preserve_regular +
        tls_used_var +
        tls_unused_var +
        tls_public_var +
        tls_static_var +
        tls_common +
        tls_weak_var +
        tls_default_vis +
        tls_hidden_vis +
        tls_protected_vis +
        tls_external_var;
    
    printf("TLS sum: %d\n", sum);
    return 0;
}
