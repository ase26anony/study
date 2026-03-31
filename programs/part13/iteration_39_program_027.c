/* tls_emulation_test.c - Test program for GCC emulated TLS attribute copying */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 100;

/* 3. Protected visibility TLS variable */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - requires -fno-common to test properly */
__thread int tls_common;

/* 5. External declaration - will be defined later */
extern __thread int tls_external;

/* 6. Weak external TLS variable */
extern __thread int tls_weak_external __attribute__((weak));

/* 7. Static TLS inside a function context (tests DECL_CONTEXT) */
static void function_with_static_tls(void) {
    static __thread int tls_static_func = 300;
    volatile int* volatile ptr = &tls_static_func;
    *ptr += 1;  /* Force usage through volatile pointer */
}

/* 8. DLL import simulation (using dllimport attribute if available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* On non-Windows, use symver or other attribute to simulate */
    __thread int tls_dllimport __attribute__((visibility("default")));
#endif

/* ===== EXTERNAL TLS DEFINITION ===== */
__thread int tls_external = 500;

/* ===== WEAK EXTERNAL TLS DEFINITION (provides definition if not linked) ===== */
__thread int tls_weak_external __attribute__((weak)) = 600;

/* ===== NAMESPACE (C++ style simulation in C) ===== */
/* Use struct to simulate namespace context */
struct NamespaceSim {
    __thread int tls_in_namespace;
    __thread int tls_namespace_hidden __attribute__((visibility("hidden")));
};

/* Define the namespace TLS variables */
__thread int struct NamespaceSim namespace_tls = {700, 800};

/* ===== HELPER FUNCTIONS THAT TAKE TLS ADDRESSES ===== */

NOINLINE static void use_tls_weak_hidden(volatile int* counter) {
    volatile int* ptr = &tls_weak_hidden;
    *ptr += *counter;
    tls_weak_hidden++;  /* Direct access too */
}

NOINLINE static void use_tls_public_default(volatile int* counter) {
    volatile int* ptr = &tls_public_default;
    *ptr -= *counter;
}

NOINLINE static void use_tls_protected(volatile int* counter) {
    /* Multiple access patterns */
    tls_protected = tls_protected * 2 + *counter;
    volatile int* ptr = &tls_protected;
    *ptr = *ptr % 1000;
}

NOINLINE static void use_tls_common(volatile int* counter) {
    tls_common = *counter;
    volatile int* ptr = &tls_common;
    for (int i = 0; i < 3; i++) {
        *ptr += i;
    }
}

NOINLINE static void use_tls_external(volatile int* counter) {
    volatile int* ptr = &tls_external;
    *ptr = (*ptr + *counter) & 0xFF;
}

NOINLINE static void use_namespace_tls(volatile int* counter) {
    volatile int* ptr1 = &namespace_tls.tls_in_namespace;
    volatile int* ptr2 = &namespace_tls.tls_namespace_hidden;
    *ptr1 = (*ptr1 + *counter) ^ 0x55;
    *ptr2 = (*ptr2 - *counter) ^ 0xAA;
}

/* ===== CONSTRUCTOR FUNCTION ===== */
static void tls_constructor(void) __attribute__((constructor));
static void tls_constructor(void) {
    /* Initialize TLS variables in constructor */
    tls_common = 999;
    tls_public_default = 123;
    namespace_tls.tls_in_namespace = 456;
    
    /* Access through volatile pointer to prevent optimization */
    volatile int* volatile ptr = &tls_external;
    *ptr = 789;
}

/* ===== COMPLEX CONTROL FLOW WITH TLS ===== */

NOINLINE static uint32_t compute_tls_checksum(volatile int selector) {
    uint32_t sum = 0;
    volatile int* ptrs[8];
    int count = 0;
    
    /* Collect TLS addresses based on selector */
    if (selector & 1) ptrs[count++] = &tls_weak_hidden;
    if (selector & 2) ptrs[count++] = &tls_public_default;
    if (selector & 4) ptrs[count++] = &tls_protected;
    if (selector & 8) ptrs[count++] = &tls_common;
    if (selector & 16) ptrs[count++] = &tls_external;
    if (selector & 32) ptrs[count++] = &namespace_tls.tls_in_namespace;
    if (selector & 64) ptrs[count++] = &namespace_tls.tls_namespace_hidden;
    if (selector & 128) ptrs[count++] = &tls_weak_external;
    
    /* Compute checksum with volatile accesses */
    for (int i = 0; i < count; i++) {
        sum += (uint32_t)(*ptrs[i]);
        sum = (sum << 3) | (sum >> 29);  /* Rotate */
    }
    
    return sum;
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    volatile int counter = 0;
    uint32_t final_checksum = 0;
    
    /* Force usage of static TLS in function context */
    function_with_static_tls();
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 10; i++) {
        counter = i;
        
        /* Call different TLS usage functions based on loop counter */
        switch (i % 7) {
            case 0: use_tls_weak_hidden(&counter); break;
            case 1: use_tls_public_default(&counter); break;
            case 2: use_tls_protected(&counter); break;
            case 3: use_tls_common(&counter); break;
            case 4: use_tls_external(&counter); break;
            case 5: use_namespace_tls(&counter); break;
            case 6: 
                /* Access DLL import simulation */
                tls_dllimport = i;
                break;
        }
        
        /* Compute checksum with varying selector */
        final_checksum ^= compute_tls_checksum(i);
    }
    
    /* Final computation using all TLS variables */
    printf("TLS values:\n");
    printf("  tls_weak_hidden: %d\n", tls_weak_hidden);
    printf("  tls_public_default: %d\n", tls_public_default);
    printf("  tls_protected: %d\n", tls_protected);
    printf("  tls_common: %d\n", tls_common);
    printf("  tls_external: %d\n", tls_external);
    printf("  tls_weak_external: %d\n", tls_weak_external);
    printf("  namespace.tls_in_namespace: %d\n", namespace_tls.tls_in_namespace);
    printf("  namespace.tls_namespace_hidden: %d\n", namespace_tls.tls_namespace_hidden);
    
    printf("\nFinal checksum: 0x%08x\n", final_checksum);
    
    /* Return value based on checksum to prevent dead code elimination */
    return (final_checksum & 0xFF) == 0 ? 0 : 1;
}
