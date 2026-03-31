/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if native TLS is available */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure TLS variables are actually referenced */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with default visibility */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* 2. Hidden visibility TLS variable */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 100;

/* 3. Protected visibility TLS variable */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - no initializer */
__thread int tls_common_var;

/* 5. External declaration (defined in another TU or later) */
extern __thread int tls_external_var;

/* 6. DLL import simulation (using visibility attributes) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* On non-Windows, simulate with visibility */
    __thread int tls_dllimport_var __attribute__((visibility("default")));
#endif

/* 7. Static TLS variable inside a function context */
static void function_with_static_tls(void) {
    static __thread int tls_static_func_var = 999;
    (void)tls_static_func_var;
}

/* 8. TLS variable used in constructor */
__thread int tls_constructor_var;

/* 9. TLS variable with both weak and hidden attributes */
__thread int tls_weak_hidden_var __attribute__((weak, visibility("hidden"))) = 333;

/* ========== HELPER FUNCTIONS ========== */

NOINLINE static void use_tls_weak(int *out) {
    volatile int *volatile ptr = &tls_weak_var;
    *out += *ptr;
    tls_weak_var++;  /* Modify to ensure it's not optimized away */
}

NOINLINE static void use_tls_hidden(int *out) {
    volatile int *volatile ptr = &tls_hidden_var;
    *out += *ptr;
    tls_hidden_var += 2;
}

NOINLINE static void use_tls_protected(int *out) {
    volatile int *volatile ptr = &tls_protected_var;
    *out += *ptr;
    tls_protected_var += 3;
}

NOINLINE static void use_tls_common(int *out) {
    /* Force initialization of common variable */
    if (tls_common_var == 0) {
        tls_common_var = 400;
    }
    volatile int *volatile ptr = &tls_common_var;
    *out += *ptr;
    tls_common_var += 4;
}

NOINLINE static void use_tls_external(int *out) {
    /* External variable - will be defined later in this file */
    volatile int *volatile ptr = &tls_external_var;
    *out += *ptr;
    tls_external_var += 5;
}

NOINLINE static void use_tls_dllimport(int *out) {
    volatile int *volatile ptr = &tls_dllimport_var;
    *out += *ptr;
    tls_dllimport_var += 6;
}

NOINLINE static void use_tls_constructor(int *out) {
    volatile int *volatile ptr = &tls_constructor_var;
    *out += *ptr;
    tls_constructor_var += 7;
}

NOINLINE static void use_tls_weak_hidden(int *out) {
    volatile int *volatile ptr = &tls_weak_hidden_var;
    *out += *ptr;
    tls_weak_hidden_var += 8;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTION ========== */

CONSTRUCTOR static void init_tls_vars(void) {
    /* Initialize TLS variables in constructor */
    tls_constructor_var = 777;
    tls_external_var = 888;  /* Define the external variable */
    tls_dllimport_var = 999;
    
    /* Access all TLS variables to ensure they're marked used */
    volatile int dummy = 
        tls_weak_var + 
        tls_hidden_var + 
        tls_protected_var + 
        tls_common_var;
    (void)dummy;
}

DESTRUCTOR static void cleanup_tls_vars(void) {
    /* Verify TLS variables are still accessible in destructor */
    volatile int sum = 
        tls_weak_var + 
        tls_hidden_var + 
        tls_protected_var;
    (void)sum;
}

/* ========== EXTERNAL TLS VARIABLE DEFINITION ========== */
/* This must come after the extern declaration to test DECL_EXTERNAL handling */
__thread int tls_external_var = 555;

/* ========== MAIN TEST FUNCTION ========== */

int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* Force reference to function with static TLS */
    function_with_static_tls();
    
    /* Loop with volatile selector to prevent optimization */
    for (volatile int i = 0; i < 10; i++) {
        selector = i % 8;
        
        /* Conditional access based on volatile selector */
        switch (selector) {
            case 0:
                use_tls_weak(&checksum);
                break;
            case 1:
                use_tls_hidden(&checksum);
                break;
            case 2:
                use_tls_protected(&checksum);
                break;
            case 3:
                use_tls_common(&checksum);
                break;
            case 4:
                use_tls_external(&checksum);
                break;
            case 5:
                use_tls_dllimport(&checksum);
                break;
            case 6:
                use_tls_constructor(&checksum);
                break;
            case 7:
                use_tls_weak_hidden(&checksum);
                break;
        }
    }
    
    /* Compute final checksum through volatile pointers */
    volatile int *volatile ptrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_external_var,
        &tls_dllimport_var,
        &tls_constructor_var,
        &tls_weak_hidden_var
    };
    
    for (int i = 0; i < 8; i++) {
        checksum += *ptrs[i];
    }
    
    printf("TLS checksum: %d\n", checksum);
    printf("All TLS attributes tested:\n");
    printf("  - Weak: %d\n", tls_weak_var);
    printf("  - Hidden: %d\n", tls_hidden_var);
    printf("  - Protected: %d\n", tls_protected_var);
    printf("  - Common: %d\n", tls_common_var);
    printf("  - External: %d\n", tls_external_var);
    printf("  - DLLImport-like: %d\n", tls_dllimport_var);
    printf("  - Constructor-used: %d\n", tls_constructor_var);
    printf("  - Weak+Hidden: %d\n", tls_weak_hidden_var);
    
    return checksum != 0 ? 0 : 1;
}
