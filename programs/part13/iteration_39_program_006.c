/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_test.c -o tls_test */
/* For C++: g++ -O2 -femulated-tls -fno-common -fvisibility=hidden tls_test.cpp -o tls_test_cpp */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default __attribute__((visibility("default"))) = 100;

/* 3. Protected visibility TLS variable */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - forces DECL_COMMON handling */
__thread int tls_common;

/* 5. External declaration - will be defined later */
extern __thread int tls_external;

/* 6. Static TLS inside function context (tests DECL_CONTEXT) */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 300;
    volatile int* volatile ptr = &tls_function_static;
    *ptr += 1;  /* Volatile access prevents optimization */
}

/* 7. DLL import simulation (using dllimport attribute when available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* On non-Windows, use weak to simulate similar behavior */
    __thread int tls_dllimport __attribute__((weak));
#endif

/* 8. TLS variable used in constructor */
__thread int tls_in_constructor;

/* ========== EXTERNAL DEFINITION ========== */
__thread int tls_external = 500;

/* ========== NOINLINE HELPER FUNCTIONS ========== */

NOINLINE static void use_tls_weak_hidden(volatile int* counter) {
    volatile int* ptr = &tls_weak_hidden;
    *ptr += *counter;
    tls_weak_hidden++;  /* Direct access too */
}

NOINLINE static void use_tls_public_default(void) {
    /* Take address and use in computation */
    int* ptr = &tls_public_default;
    *ptr = (*ptr * 3) / 2;
    
    /* Volatile pointer access */
    volatile int* vptr = ptr;
    (void)*vptr;  /* Ensure access */
}

NOINLINE static void use_tls_protected(int modifier) {
    /* Conditional access based on parameter */
    if (modifier > 0) {
        tls_protected += modifier;
    } else {
        tls_protected -= (-modifier);
    }
}

NOINLINE static void use_tls_common_and_external(void) {
    /* Use both common and external in same function */
    tls_common = tls_external % 7;
    tls_external = tls_common * 11;
}

NOINLINE static void use_tls_dllimport_like(void) {
    /* Force usage of weak/DLL import style variable */
    if (&tls_dllimport != NULL) {  /* Address comparison */
        volatile int* ptr = &tls_dllimport;
        *ptr = 999;
    }
}

/* ========== CONSTRUCTOR FUNCTION ========== */

__attribute__((constructor)) 
static void init_tls_in_constructor(void) {
    /* This should trigger DECL_PRESERVE_P handling */
    tls_in_constructor = 0xABCD;
    
    /* Also use other TLS variables in constructor */
    tls_public_default = 1234;
    
    /* Volatile access pattern */
    volatile int* vptr = &tls_in_constructor;
    *vptr += 1;
}

/* ========== COMPLEX CONTROL FLOW ========== */

NOINLINE static uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    volatile int selector = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 5; i++) {
        selector = i % 5;
        
        /* Switch on volatile selector forces multiple code paths */
        switch (selector) {
            case 0:
                sum += tls_weak_hidden;
                use_tls_weak_hidden((volatile int*)&i);
                break;
            case 1:
                sum += tls_public_default;
                use_tls_public_default();
                break;
            case 2:
                sum += tls_protected;
                use_tls_protected(i);
                break;
            case 3:
                sum += tls_common;
                use_tls_common_and_external();
                break;
            case 4:
                sum += tls_external;
                use_tls_dllimport_like();
                break;
        }
    }
    
    /* Access function-static TLS */
    function_with_static_tls();
    
    /* Access constructor-initialized TLS */
    sum += tls_in_constructor;
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize common TLS variable */
    tls_common = 777;
    
    /* Initialize DLL-import-like variable if defined */
    tls_dllimport = 888;
    
    /* Call all helper functions to ensure TLS variables are marked used */
    volatile int counter = 1;
    use_tls_weak_hidden(&counter);
    use_tls_public_default();
    use_tls_protected(10);
    use_tls_common_and_external();
    use_tls_dllimport_like();
    
    /* Compute checksum using complex control flow */
    uint32_t checksum = compute_tls_checksum();
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: %u\n", checksum);
    
    /* Print addresses to verify emulated TLS structures */
    printf("TLS addresses:\n");
    printf("  tls_weak_hidden: %p\n", (void*)&tls_weak_hidden);
    printf("  tls_public_default: %p\n", (void*)&tls_public_default);
    printf("  tls_protected: %p\n", (void*)&tls_protected);
    printf("  tls_common: %p\n", (void*)&tls_common);
    printf("  tls_external: %p\n", (void*)&tls_external);
    printf("  tls_dllimport: %p\n", (void*)&tls_dllimport);
    printf("  tls_in_constructor: %p\n", (void*)&tls_in_constructor);
    
    return (int)(checksum % 256);
}
