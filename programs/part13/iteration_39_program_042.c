/* tls_emulation_test.c - Comprehensive test for GCC emulated TLS attribute propagation */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with default visibility */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* 2. Hidden visibility TLS */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 100;

/* 3. Protected visibility TLS */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - no initializer */
__thread int tls_common_var;

/* 5. External declaration (defined in same file but declared extern first) */
extern __thread int tls_external_var;
__thread int tls_external_var = 300;

/* 6. Static TLS inside function (tests DECL_CONTEXT) */
static void function_with_static_tls(void) {
    static __thread int tls_static_func = 500;
    volatile int* volatile ptr = &tls_static_func;
    *ptr += 1; /* Force usage through volatile pointer */
}

/* 7. DLL import simulation (using dllimport attribute if supported) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* Simulate with weak external on non-Windows */
    extern __thread int tls_dllimport_var __attribute__((weak));
#endif

/* 8. Public TLS with explicit visibility */
__thread int tls_public_var __attribute__((visibility("default"))) = 400;

/* 9. TLS with preserve_flag testing */
__thread int tls_preserve_var __attribute__((used)) = 600;

/* ========== HELPER FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE static void use_tls_weak_addr(volatile int** out) {
    *out = &tls_weak_var;
    tls_weak_var++; /* Ensure variable is marked as used */
}

NOINLINE static void use_tls_hidden_addr(volatile int** out) {
    *out = &tls_hidden_var;
    tls_hidden_var += 2;
}

NOINLINE static void use_tls_protected_addr(volatile int** out) {
    *out = &tls_protected_var;
    tls_protected_var += 3;
}

NOINLINE static void use_tls_common_addr(volatile int** out) {
    *out = &tls_common_var;
    tls_common_var = 123; /* Initialize the common variable */
}

NOINLINE static void use_tls_external_addr(volatile int** out) {
    *out = &tls_external_var;
    tls_external_var += 4;
}

NOINLINE static void use_tls_public_addr(volatile int** out) {
    *out = &tls_public_var;
    tls_public_var += 5;
}

NOINLINE static void use_tls_preserve_addr(volatile int** out) {
    *out = &tls_preserve_var;
    tls_preserve_var += 6;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

CONSTRUCTOR static void tls_constructor(void) {
    /* Access and modify TLS in constructor - tests DECL_PRESERVE_P */
    tls_preserve_var = 999;
    tls_hidden_var = 888;
    
    /* Take addresses in constructor */
    volatile int* ptr1 = &tls_weak_var;
    volatile int* ptr2 = &tls_protected_var;
    (void)ptr1;
    (void)ptr2;
}

DESTRUCTOR static void tls_destructor(void) {
    /* Verify TLS is still accessible in destructor */
    volatile int check = tls_preserve_var;
    (void)check;
}

/* ========== COMPLEX CONTROL FLOW WITH VOLATILE ========== */

NOINLINE static uint32_t compute_tls_checksum(void) {
    volatile int selector = 0;
    uint32_t checksum = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 7; i++) {
        selector = i;
        volatile int* tls_ptr = NULL;
        
        /* Switch on volatile selector forces multiple code paths */
        switch (selector) {
            case 0: use_tls_weak_addr(&tls_ptr); break;
            case 1: use_tls_hidden_addr(&tls_ptr); break;
            case 2: use_tls_protected_addr(&tls_ptr); break;
            case 3: use_tls_common_addr(&tls_ptr); break;
            case 4: use_tls_external_addr(&tls_ptr); break;
            case 5: use_tls_public_addr(&tls_ptr); break;
            case 6: use_tls_preserve_addr(&tls_ptr); break;
        }
        
        if (tls_ptr) {
            checksum += *tls_ptr;
            checksum = (checksum << 3) | (checksum >> 29); /* Simple mixing */
        }
    }
    
    return checksum;
}

/* ========== MAIN EXECUTION FLOW ========== */

int main(void) {
    uint32_t final_checksum = 0;
    
    /* Initialize common TLS variable */
    tls_common_var = 777;
    
    /* Call function with static TLS */
    function_with_static_tls();
    
    /* Force address taking of all TLS variables */
    volatile int* addrs[7];
    use_tls_weak_addr(&addrs[0]);
    use_tls_hidden_addr(&addrs[1]);
    use_tls_protected_addr(&addrs[2]);
    use_tls_common_addr(&addrs[3]);
    use_tls_external_addr(&addrs[4]);
    use_tls_public_addr(&addrs[5]);
    use_tls_preserve_addr(&addrs[6]);
    
    /* Compute checksum through complex control flow */
    final_checksum = compute_tls_checksum();
    
    /* Use all addresses to prevent optimization */
    for (int i = 0; i < 7; i++) {
        if (addrs[i]) {
            final_checksum ^= (uintptr_t)addrs[i];
        }
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: 0x%08x\n", final_checksum);
    
    /* Verify emulated TLS structure by checking addresses aren't NULL */
    int valid_count = 0;
    for (int i = 0; i < 7; i++) {
        if (addrs[i] != NULL) valid_count++;
    }
    
    printf("Valid TLS addresses: %d/7\n", valid_count);
    
    return (valid_count == 7) ? 0 : 1;
}
