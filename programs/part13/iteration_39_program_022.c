/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")  /* Some GCC versions support this to force emulation */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure TLS addresses are taken */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 100;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 200;

/* 3. Protected visibility TLS variable */
__thread int tls_protected __attribute__((visibility("protected"))) = 300;

/* 4. Common linkage (tentative definition) - no initializer */
__thread int tls_common __attribute__((common));

/* 5. External declaration (defined in another TU or later) */
extern __thread int tls_external;

/* 6. DLL import simulation (Windows-specific attribute) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with weak external on non-Windows */
extern __thread int tls_dllimport __attribute__((weak));
#endif

/* 7. Static TLS inside a function context (tests DECL_CONTEXT) */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 700;
    volatile int* volatile ptr = &tls_func_static;
    *ptr += 1;  /* Force access through volatile pointer */
}

/* 8. TLS with preserve attribute (used in constructor) */
__thread int tls_preserve __attribute__((used)) = 800;

/* ========== HELPER FUNCTIONS ========== */

NOINLINE static void use_tls_weak_hidden(volatile int* counter) {
    volatile int* ptr = &tls_weak_hidden;
    *ptr += *counter;
    tls_weak_hidden++;  /* Direct access too */
}

NOINLINE static void use_tls_public_default(void) {
    /* Take address and use in computation */
    int* ptr = &tls_public_default;
    *ptr = (*ptr * 3) / 2;
}

NOINLINE static void use_tls_protected(int modifier) {
    tls_protected ^= modifier;  /* XOR operation */
}

NOINLINE static void use_tls_common(volatile int* trigger) {
    if (*trigger) {
        tls_common = 999;
    } else {
        tls_common = -999;
    }
}

NOINLINE static void* get_tls_external_addr(void) {
    /* Force reference to external TLS */
    return (void*)&tls_external;
}

NOINLINE static void use_tls_dllimport(int value) {
    /* Simulate access to DLL imported TLS */
    volatile int* ptr = &tls_dllimport;
    (void)ptr;  /* Suppress unused warning */
    /* Can't actually write to dllimport in this test */
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTION ========== */

CONSTRUCTOR static void init_tls_values(void) {
    /* This constructor runs before main */
    tls_preserve = 0xABCD;
    tls_public_default = 1234;
    
    /* Access weak TLS to mark it used */
    volatile int* p = &tls_weak_hidden;
    (void)p;
}

DESTRUCTOR static void cleanup_tls(void) {
    /* Ensure TLS variables are preserved through destructor */
    volatile int dummy = tls_preserve;
    (void)dummy;
}

/* ========== COMPLEX CONTROL FLOW ========== */

NOINLINE static uintptr_t compute_tls_checksum(void) {
    uintptr_t sum = 0;
    volatile int selector = 0;
    
    /* Loop with volatile control variable */
    for (volatile int i = 0; i < 5; i++) {
        selector = i;
        
        /* Conditional TLS access based on volatile */
        switch (selector) {
            case 0:
                sum += (uintptr_t)&tls_weak_hidden;
                use_tls_weak_hidden((int*)&i);
                break;
            case 1:
                sum += tls_public_default;
                use_tls_public_default();
                break;
            case 2:
                sum += tls_protected;
                use_tls_protected((int)sum);
                break;
            case 3:
                sum += tls_common;
                use_tls_common(&i);
                break;
            case 4:
                sum += (uintptr_t)get_tls_external_addr();
                use_tls_dllimport((int)sum);
                break;
        }
    }
    
    /* Access function-static TLS */
    func_with_static_tls();
    
    return sum;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This simulates a TLS variable defined in another translation unit */
__thread int tls_external = 500;

/* For DLL import simulation */
#ifndef _WIN32
__thread int tls_dllimport = 600;
#endif

/* ========== MAIN FUNCTION ========== */

int main(void) {
    volatile int loop_counter = 0;
    uintptr_t final_checksum = 0;
    
    /* Phase 1: Initial TLS access pattern */
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        use_tls_weak_hidden(&loop_counter);
        use_tls_protected(loop_counter);
        
        /* Volatile pointer access to prevent optimization */
        volatile int* volatile vptr = &tls_public_default;
        *vptr += loop_counter;
    }
    
    /* Phase 2: Compute checksum with complex control flow */
    final_checksum = compute_tls_checksum();
    
    /* Phase 3: Verify TLS addresses are unique and non-null */
    void* tls_addresses[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_protected,
        &tls_common,
        &tls_external,
        &tls_dllimport,
        &tls_preserve
    };
    
    printf("TLS Emulation Test - Checksum: 0x%lx\n", 
           (unsigned long)final_checksum);
    
    /* Print addresses to ensure they're used */
    for (int i = 0; i < (int)(sizeof(tls_addresses)/sizeof(tls_addresses[0])); i++) {
        printf("TLS var %d: %p\n", i, tls_addresses[i]);
    }
    
    /* Return value based on TLS state */
    return (tls_public_default != 0) ? 0 : 1;
}
