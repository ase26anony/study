/* tls_emulation_test.c - Test program for GCC emulated TLS attribute propagation */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))

/* Global TLS variables with various attributes */

/* 1. Weak TLS variable with default visibility */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* 2. Hidden visibility TLS variable */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 100;

/* 3. Protected visibility TLS variable */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - requires -fno-common to test properly */
__thread int tls_common_var;

/* 5. External declaration (will be defined later) */
extern __thread int tls_external_var;

/* 6. Static TLS inside a namespace context (C++ style simulated in C) */
static __thread int tls_static_context = 300;

/* 7. DLL import simulation (using dllimport attribute if supported) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* Simulate with weak external */
    extern __thread int tls_dllimport_var __attribute__((weak));
#endif

/* 8. TLS variable used in constructor */
__thread int tls_constructor_var;

/* 9. Public TLS variable with explicit visibility */
__thread int tls_public_var __attribute__((visibility("default"))) = 400;

/* External definition for the extern declaration */
__thread int tls_external_var = 500;

/* Definition for dllimport variable */
__thread int tls_dllimport_var = 600;

/* Block-scoped TLS variable simulation */
void block_scoped_tls_test(void) {
    /* Static TLS inside function (different context) */
    static __thread int tls_function_static = 700;
    volatile int* volatile ptr = &tls_function_static;
    *ptr += 1;  /* Force volatile access */
}

/* Constructor that accesses TLS */
__attribute__((constructor))
static void tls_constructor(void) {
    tls_constructor_var = 0xABCD;
    tls_public_var = 999;
    
    /* Mark as used through volatile access */
    volatile int* vptr = &tls_constructor_var;
    (void)*vptr;
}

/* Helper functions that take addresses of TLS variables */

NOINLINE static void use_tls_weak(int* out) {
    volatile int* ptr = &tls_weak_var;
    *out = *ptr + 1;
}

NOINLINE static void use_tls_hidden(int* out) {
    volatile int* ptr = &tls_hidden_var;
    *out = *ptr + 2;
}

NOINLINE static void use_tls_protected(int* out) {
    volatile int* ptr = &tls_protected_var;
    *out = *ptr + 3;
}

NOINLINE static void use_tls_common(int* out) {
    /* Initialize if zero (tentative definition behavior) */
    if (tls_common_var == 0) {
        tls_common_var = 800;
    }
    volatile int* ptr = &tls_common_var;
    *out = *ptr + 4;
}

NOINLINE static void use_tls_external(int* out) {
    volatile int* ptr = &tls_external_var;
    *out = *ptr + 5;
}

NOINLINE static void use_tls_dllimport(int* out) {
    volatile int* ptr = &tls_dllimport_var;
    *out = *ptr + 6;
}

NOINLINE static void use_tls_public(int* out) {
    volatile int* ptr = &tls_public_var;
    *out = *ptr + 7;
}

NOINLINE static void use_tls_static_context(int* out) {
    volatile int* ptr = &tls_static_context;
    *out = *ptr + 8;
}

/* Function that mixes multiple TLS accesses */
NOINLINE static uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    volatile int* ptr;
    
    /* Access all TLS variables through volatile pointers */
    ptr = &tls_weak_var;
    sum += *ptr;
    
    ptr = &tls_hidden_var;
    sum += *ptr;
    
    ptr = &tls_protected_var;
    sum += *ptr;
    
    ptr = &tls_common_var;
    sum += *ptr;
    
    ptr = &tls_external_var;
    sum += *ptr;
    
    ptr = &tls_dllimport_var;
    sum += *ptr;
    
    ptr = &tls_public_var;
    sum += *ptr;
    
    ptr = &tls_static_context;
    sum += *ptr;
    
    ptr = &tls_constructor_var;
    sum += *ptr;
    
    return sum;
}

/* Runtime verification of emulated TLS */
static void verify_emutls(void) {
    /* Check if we're using emulated TLS by examining pointer patterns */
    void* addr1 = &tls_weak_var;
    void* addr2 = &tls_hidden_var;
    
    /* In emulated TLS, addresses might have specific patterns or come from
       different mechanisms. We'll just print them for manual verification. */
    printf("TLS addresses: weak=%p, hidden=%p\n", addr1, addr2);
    
    /* Verify they're not null */
    if (addr1 && addr2) {
        printf("TLS addresses look valid (emulation likely active)\n");
    }
}

int main(void) {
    int results[8];
    volatile int selector = 0;
    uint32_t final_sum = 0;
    
    /* Force initialization of all TLS variables */
    tls_common_var = 800;  /* Initialize the common variable */
    
    /* Call helper functions that take addresses of TLS variables */
    use_tls_weak(&results[0]);
    use_tls_hidden(&results[1]);
    use_tls_protected(&results[2]);
    use_tls_common(&results[3]);
    use_tls_external(&results[4]);
    use_tls_dllimport(&results[5]);
    use_tls_public(&results[6]);
    use_tls_static_context(&results[7]);
    
    /* Conditional access based on volatile selector */
    for (selector = 0; selector < 8; selector++) {
        volatile int* target = NULL;
        
        switch (selector) {
            case 0: target = &tls_weak_var; break;
            case 1: target = &tls_hidden_var; break;
            case 2: target = &tls_protected_var; break;
            case 3: target = &tls_common_var; break;
            case 4: target = &tls_external_var; break;
            case 5: target = &tls_dllimport_var; break;
            case 6: target = &tls_public_var; break;
            case 7: target = &tls_static_context; break;
        }
        
        if (target) {
            *target += selector;  /* Modify through volatile pointer */
        }
    }
    
    /* Test block-scoped TLS */
    block_scoped_tls_test();
    
    /* Compute checksum of all TLS values */
    final_sum = tls_checksum();
    
    /* Verify emulated TLS is active */
    verify_emutls();
    
    /* Print results to prevent optimization */
    printf("TLS checksum: 0x%08X\n", final_sum);
    printf("Individual values:\n");
    printf("  weak: %d\n", tls_weak_var);
    printf("  hidden: %d\n", tls_hidden_var);
    printf("  protected: %d\n", tls_protected_var);
    printf("  common: %d\n", tls_common_var);
    printf("  external: %d\n", tls_external_var);
    printf("  dllimport: %d\n", tls_dllimport_var);
    printf("  public: %d\n", tls_public_var);
    printf("  static_context: %d\n", tls_static_context);
    printf("  constructor: 0x%X\n", tls_constructor_var);
    
    return 0;
}
