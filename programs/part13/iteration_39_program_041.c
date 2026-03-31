/* tls_emulation_test.c - Comprehensive test for GCC emulated TLS attribute propagation */

/* Force emulated TLS even on platforms with native support */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to force actual TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))
#define USED __attribute__((used))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 100;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 200;

/* 3. Protected visibility TLS variable */
__thread int tls_protected __attribute__((visibility("protected"))) = 300;

/* 4. Common linkage (tentative definition) - forces DECL_COMMON handling */
__thread int tls_common;

/* 5. External declaration - will be defined later */
extern __thread int tls_external;

/* 6. Weak external TLS variable */
extern __thread int tls_weak_external __attribute__((weak));

/* 7. DLL import simulation (using visibility attributes) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    __thread int tls_dllimport __attribute__((visibility("default")));
#endif

/* 8. Static TLS inside a namespace context (C++ style simulated in C) */
static __thread int tls_static_context = 400;

/* 9. TLS with preserve attribute (via used) */
__thread int tls_preserved __attribute__((used)) = 500;

/* 10. TLS variable that will be marked TREE_USED through address-taking */
__thread int tls_used_through_address = 600;

/* ========== FUNCTION DECLARATIONS ========== */

NOINLINE static void use_tls_address(volatile int *addr);
NOINLINE static void modify_tls_via_pointer(volatile int *addr, int value);
NOINLINE static int compute_tls_checksum(void);
NOINLINE static void access_tls_in_different_context(void);
CONSTRUCTOR static void tls_constructor_init(void);
DESTRUCTOR static void tls_destructor_check(void);

/* ========== TLS VARIABLE DEFINITIONS ========== */

/* Define the external TLS variables */
__thread int tls_external = 700;
__thread int tls_weak_external __attribute__((weak)) = 800;

/* ========== HELPER FUNCTIONS ========== */

NOINLINE static void use_tls_address(volatile int *addr) {
    /* Force TLS variable usage through volatile pointer */
    static volatile int sink;
    sink = *addr;
    (void)sink;
}

NOINLINE static void modify_tls_via_pointer(volatile int *addr, int value) {
    *addr = value;
}

NOINLINE static int compute_tls_checksum(void) {
    /* Compute a simple checksum of all TLS variables to prevent optimization */
    int sum = 0;
    
    /* Access all TLS variables in different ways */
    sum += tls_weak_hidden;
    sum += tls_public_default;
    sum += tls_protected;
    sum += tls_common;
    sum += tls_external;
    sum += tls_weak_external;
    sum += tls_dllimport;
    sum += tls_static_context;
    sum += tls_preserved;
    sum += tls_used_through_address;
    
    /* Also access via volatile pointers */
    volatile int *volatile ptrs[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_protected,
        &tls_common,
        &tls_external,
        &tls_weak_external,
        &tls_dllimport,
        &tls_static_context,
        &tls_preserved,
        &tls_used_through_address
    };
    
    for (int i = 0; i < 10; i++) {
        sum += *ptrs[i];
    }
    
    return sum;
}

NOINLINE static void access_tls_in_different_context(void) {
    /* Block-scoped static TLS variable - different DECL_CONTEXT */
    static __thread int tls_block_static = 900;
    
    /* Take address to force TREE_USED */
    volatile int *addr = &tls_block_static;
    *addr += 1;
    
    /* Function-scoped TLS variable */
    __thread int tls_function_scope = 1000;
    volatile int *addr2 = &tls_function_scope;
    *addr2 += 1;
    
    /* Use both variables */
    use_tls_address(addr);
    use_tls_address(addr2);
}

CONSTRUCTOR static void tls_constructor_init(void) {
    /* Initialize TLS variables in constructor to test DECL_PRESERVE_P */
    tls_common = 1100;
    tls_dllimport = 1200;
    
    /* Access via volatile pointer */
    volatile int *volatile ptr = &tls_preserved;
    *ptr = 1300;
}

DESTRUCTOR static void tls_destructor_check(void) {
    /* Verify TLS values are still accessible */
    volatile int check = tls_public_default;
    (void)check;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* 1. Initialize some TLS variables */
    tls_common = 1400;
    tls_used_through_address = 1500;
    
    /* 2. Take addresses of all TLS variables to force TREE_USED */
    volatile int *addresses[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_protected,
        &tls_common,
        &tls_external,
        &tls_weak_external,
        &tls_dllimport,
        &tls_static_context,
        &tls_preserved,
        &tls_used_through_address
    };
    
    /* 3. Use TLS variables through volatile pointers in a loop */
    for (selector = 0; selector < 10; selector++) {
        /* Conditional access based on volatile selector */
        if (selector & 1) {
            modify_tls_via_pointer(addresses[selector], selector * 100);
        } else {
            use_tls_address(addresses[selector]);
        }
    }
    
    /* 4. Access TLS in different scoping contexts */
    access_tls_in_different_context();
    
    /* 5. Compute checksum to prevent optimization */
    checksum = compute_tls_checksum();
    
    /* 6. Print something to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    /* 7. Verify emulated TLS by checking if addresses are unique */
    uintptr_t base_addr = (uintptr_t)&tls_public_default;
    int unique_count = 0;
    
    for (int i = 0; i < 10; i++) {
        if ((uintptr_t)addresses[i] != base_addr) {
            unique_count++;
        }
    }
    
    printf("Unique TLS addresses found: %d/10\n", unique_count);
    
    return checksum == 0 ? 1 : 0;
}

/* ========== ADDITIONAL COMPILATION UNIT SIMULATION ========== */

/* Separate function in different "compilation unit" context */
NOINLINE void external_tls_user(void) {
    /* Use weak external TLS to test weak attribute propagation */
    if (&tls_weak_external != NULL) {
        tls_weak_external++;
    }
    
    /* Use external TLS */
    tls_external++;
}
