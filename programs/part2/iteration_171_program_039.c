/* Test for TLS emulation attribute copying - Main file */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with initialization - tests DECL_PRESERVE_P, TREE_USED */
__thread int tls_initialized = 42;

/* Test 2: Public TLS with visibility attribute - tests TREE_PUBLIC, DECL_VISIBILITY */
__attribute__((visibility("default")))
__thread int tls_public_default = 100;

/* Test 3: Hidden visibility - tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 200;

/* Test 4: Protected visibility */
__attribute__((visibility("protected")))
__thread int tls_protected = 300;

/* Test 5: Weak TLS variable - tests DECL_WEAK */
__attribute__((weak))
__thread int tls_weak = 400;

/* Test 6: Used attribute to ensure preservation - tests DECL_PRESERVE_P */
__attribute__((used))
__thread int tls_used_attr = 500;

/* Test 7: External declaration (defined in aux file) - tests DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 8: Common linkage (tentative definition) - tests DECL_COMMON */
__thread int tls_common;

/* Test 9: Static TLS inside function - tests DECL_CONTEXT */
static void func_with_tls(void) {
    static __thread int tls_in_function = 600;
    /* Take address to ensure it's used */
    volatile int *p = &tls_in_function;
    (void)p;
}

/* Test 10: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
__attribute__((dllimport)) __thread int tls_dllimport;
#endif

/* Forward declaration for function in aux file */
void use_tls_variables(void);

/* Helper to prevent optimization */
static void touch_variable(volatile void *ptr) {
    __asm__ volatile ("" : : "r"(ptr) : "memory");
}

int main(void) {
    /* Use all TLS variables to ensure they're not optimized away */
    
    /* Test 1: Basic initialized TLS */
    tls_initialized += 1;
    touch_variable(&tls_initialized);
    
    /* Test 2: Public with default visibility */
    tls_public_default *= 2;
    touch_variable(&tls_public_default);
    
    /* Test 3: Hidden visibility */
    tls_hidden -= 10;
    touch_variable(&tls_hidden);
    
    /* Test 4: Protected visibility */
    tls_protected /= 3;
    touch_variable(&tls_protected);
    
    /* Test 5: Weak variable */
    if (&tls_weak != NULL) {
        tls_weak = 999;
    }
    touch_variable(&tls_weak);
    
    /* Test 6: Used attribute */
    tls_used_attr = 777;
    touch_variable(&tls_used_attr);
    
    /* Test 7: External (defined in aux) */
    tls_external = 888;
    touch_variable(&tls_external);
    
    /* Test 8: Common linkage */
    tls_common = 111;
    touch_variable(&tls_common);
    
    /* Test 9: TLS in function context */
    func_with_tls();
    
    /* Test 10: DLL import simulation */
#ifdef _WIN32
    touch_variable(&tls_dllimport);
#endif
    
    /* Call function from aux file that uses TLS */
    use_tls_variables();
    
    /* Additional asm to ensure variables are marked used */
    __asm__ volatile (
        "# TLS variable references\n"
        : : 
        "r"(&tls_initialized),
        "r"(&tls_public_default),
        "r"(&tls_hidden),
        "r"(&tls_protected),
        "r"(&tls_weak),
        "r"(&tls_used_attr),
        "r"(&tls_external),
        "r"(&tls_common)
    );
    
    return 0;
}

#ifdef __cplusplus
}
#endif
