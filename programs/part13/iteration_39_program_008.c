/* tls_emulation_test.c - Test program for GCC emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) 
int tls_weak_hidden = 0x1234;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 0x5678;

/* 3. Protected visibility TLS variable */
__thread __attribute__((visibility("protected"))) 
int tls_protected = 0x9ABC;

/* 4. Common linkage (tentative definition) - requires -fno-common to test properly */
__thread int tls_common;

/* 5. External TLS declaration (defined in another TU or later) */
extern __thread int tls_external;

/* 6. DLL import simulation (using dllimport attribute when available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* On non-Windows, use symver or other attribute to simulate */
    __thread __attribute__((weak)) int tls_dllimport;
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 0xDEF0;
    volatile int* volatile ptr = &tls_func_static;
    *ptr += 1;  /* Force usage through volatile pointer */
}

/* 8. TLS with specified visibility */
__thread __attribute__((visibility("hidden"), used))
int tls_used_hidden = 0x1111;

/* ===== NOINLINE HELPER FUNCTIONS ===== */

NOINLINE static void use_tls_weak_hidden(volatile int* counter) {
    volatile int* ptr = &tls_weak_hidden;
    *ptr += *counter;
    tls_weak_hidden++;  /* Direct access too */
}

NOINLINE static void use_tls_public_default(void) {
    /* Take address and use in computation */
    int* ptr = &tls_public_default;
    *ptr = (*ptr * 3 + 1) & 0xFFFF;
}

NOINLINE static void use_tls_protected(volatile int mod) {
    /* Complex access pattern to prevent optimization */
    for (volatile int i = 0; i < 3; i++) {
        tls_protected = (tls_protected + mod + i) % 1000;
    }
}

NOINLINE static void* get_tls_common_addr(void) {
    /* Force address-taking of common TLS */
    return (void*)&tls_common;
}

NOINLINE static void use_tls_external(void) {
    /* External TLS usage - compiler must handle declaration */
    volatile int* ptr = &tls_external;
    (void)ptr;  /* Use ptr to prevent elimination */
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTION ===== */

CONSTRUCTOR static void init_tls_values(void) {
    /* This runs before main, testing DECL_PRESERVE_P propagation */
    tls_public_default = 0x8888;
    tls_protected = 0x9999;
    
    /* Access weak TLS to mark it used */
    volatile int tmp = tls_weak_hidden;
    (void)tmp;
}

DESTRUCTOR static void cleanup_tls(void) {
    /* Final TLS access in destructor */
    volatile int tmp = tls_public_default;
    (void)tmp;
}

/* ===== VOLATILE CONTROL FLOW ===== */

NOINLINE static uint32_t compute_tls_checksum(void) {
    volatile int selector = 0;
    uint32_t checksum = 0;
    
    /* Volatile loop accessing different TLS variables conditionally */
    for (volatile int i = 0; i < 10; i++) {
        selector = (selector * 1103515245 + 12345) & 0x7FFF;
        
        switch (selector % 5) {
            case 0:
                checksum ^= (uint32_t)tls_weak_hidden;
                break;
            case 1:
                checksum += (uint32_t)tls_public_default;
                break;
            case 2:
                checksum ^= (uint32_t)tls_protected << 8;
                break;
            case 3:
                checksum += (uint32_t)(uintptr_t)get_tls_common_addr();
                break;
            case 4:
                checksum = (checksum * 31) + 17;
                break;
        }
    }
    
    return checksum;
}

/* ===== NAMESPACE TEST (C++ STYLE IN C) ===== */

/* Simulate namespace with static linkage */
struct tls_namespace {
    __thread static int tls_in_namespace;
    __thread static int tls_private __attribute__((visibility("hidden")));
};

/* Define the namespace TLS variables */
__thread int tls_namespace::tls_in_namespace = 0xAAAA;
__thread int tls_namespace::tls_private = 0xBBBB;

NOINLINE static void use_namespace_tls(void) {
    /* Access namespace-style TLS */
    volatile int* ptr1 = &tls_namespace::tls_in_namespace;
    volatile int* ptr2 = &tls_namespace::tls_private;
    *ptr1 += 1;
    *ptr2 ^= 0x5555;
}

/* ===== MAIN FUNCTION ===== */

/* Define the external TLS variable */
__thread int tls_external = 0xCCCC;

int main(void) {
    volatile int counter = 1;
    
    /* 1. Force usage of all TLS variables through noinline functions */
    use_tls_weak_hidden(&counter);
    use_tls_public_default();
    use_tls_protected(counter);
    use_tls_external();
    use_namespace_tls();
    
    /* 2. Function with static TLS */
    func_with_static_tls();
    
    /* 3. Initialize common TLS */
    tls_common = 0xDDDD;
    
    /* 4. DLL import-like TLS usage */
    tls_dllimport = 0xEEEE;
    
    /* 5. Complex volatile control flow accessing TLS */
    uint32_t checksum = compute_tls_checksum();
    
    /* 6. Additional direct accesses to ensure TREE_USED is set */
    TREE_USED_ACCESS:  /* Label for clarity, not functional */
    tls_used_hidden = checksum & 0xFFFF;
    
    /* 7. Print checksum to prevent dead code elimination */
    printf("TLS checksum: 0x%08X\n", checksum);
    
    /* 8. Return value based on TLS state for verification */
    int result = (tls_weak_hidden ^ tls_public_default ^ tls_protected ^ 
                  tls_common ^ tls_external ^ tls_used_hidden) & 0xFF;
    
    return result;
}

/* ===== ADDITIONAL COMPILATION UNIT SIMULATION ===== */

/* This would normally be in a separate file, but included here for completeness */
#ifdef SECOND_TU
/* External references to test DECL_EXTERNAL handling */
extern __thread int tls_weak_hidden;
extern __thread int tls_public_default;

void secondary_function(void) {
    /* Access external TLS variables */
    volatile int* ptr1 = &tls_weak_hidden;
    volatile int* ptr2 = &tls_public_default;
    *ptr1 += 100;
    *ptr2 -= 50;
}
#endif
