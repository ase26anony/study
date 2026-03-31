/* Test program for TLS emulation attribute copying coverage */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#include <stdio.h>

/* Forward declarations for variables defined in emutls_aux.c */
extern __thread int tls_extern_var;
extern __thread int tls_weak_alias_target;

/* Function prototype from auxiliary file */
void use_tls_variables(void);

/* Test 1: Regular TLS definition with initialization - tests DECL_PRESERVE_P via TREE_USED */
/* This will be used, so DECL_PRESERVE_P should be true */
__thread int tls_defined = 42;

/* Test 2: Public TLS with default visibility - tests TREE_PUBLIC and default visibility */
__thread int tls_public __attribute__((visibility("default")));

/* Test 3: Weak TLS variable - tests DECL_WEAK */
__thread int tls_weak __attribute__((weak));

/* Test 4: Hidden visibility TLS - tests DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* Test 5: Protected visibility TLS - tests another visibility variant */
__thread int tls_protected __attribute__((visibility("protected")));

/* Test 6: Used attribute to ensure preservation - tests DECL_PRESERVE_P explicitly */
__thread int tls_explicitly_used __attribute__((used));

/* Test 7: Static TLS (non-public) for contrast - tests !TREE_PUBLIC */
static __thread int tls_static;

/* Test 8: Common TLS (tentative definition) - tests DECL_COMMON */
__thread int tls_common;

/* Test 9: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* On non-Windows, use a weak external to simulate similar behavior */
extern __thread int tls_dllimport __attribute__((weak));
#endif

/* Test 10: TLS in function scope for DECL_CONTEXT testing */
void test_function_scope(void) {
    /* Local TLS variable - will have DECL_CONTEXT set to the function */
    static __thread int tls_local_scope;
    tls_local_scope++;
}

/* Helper function to take addresses and prevent optimization */
static void take_addresses(void) {
    /* Take addresses of all TLS variables to ensure they're processed */
    volatile int *addrs[] = {
        &tls_defined,
        &tls_public,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_explicitly_used,
        &tls_static,
        &tls_common,
        &tls_dllimport,
        &tls_extern_var,
        &tls_weak_alias_target
    };
    
    /* Use asm to ensure addresses are taken without being optimized out */
    for (int i = 0; i < (int)(sizeof(addrs)/sizeof(addrs[0])); i++) {
        __asm__ volatile ("" : : "r"(addrs[i]) : "memory");
    }
}

int main(void) {
    printf("Testing TLS emulation attribute copying...\n");
    
    /* Modify some TLS variables to ensure they're used */
    tls_defined = 100;
    tls_public = 200;
    tls_hidden = 300;
    
    /* Access common variable to give it a definition */
    tls_common = 400;
    
    /* Call function with local TLS */
    test_function_scope();
    
    /* Use external TLS variables */
    tls_extern_var = 500;
    
    /* Take addresses to prevent dead code elimination */
    take_addresses();
    
    /* Call function from auxiliary file */
    use_tls_variables();
    
    /* Simple validation */
    printf("tls_defined = %d\n", tls_defined);
    printf("tls_extern_var = %d\n", tls_extern_var);
    
    return 0;
}
