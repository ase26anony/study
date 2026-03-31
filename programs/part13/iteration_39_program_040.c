/* tls_emulation_test.c - Test program for GCC emulated TLS attribute copying */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

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

/* 6. DLL import simulation (using dllimport attribute if available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* Simulate with weak external */
    extern __thread int tls_dllimport_var __attribute__((weak));
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 300;
    tls_func_static++;
}

/* 8. TLS with preserved attribute (via constructor usage) */
__thread int tls_preserved_var = 999;

/* ===== HELPER FUNCTIONS FOR ADDRESS TAKING ===== */

NOINLINE static void use_tls_weak_addr(volatile int **addr) {
    *addr = &tls_weak_var;
}

NOINLINE static void use_tls_hidden_addr(volatile int **addr) {
    *addr = &tls_hidden_var;
}

NOINLINE static void use_tls_protected_addr(volatile int **addr) {
    *addr = &tls_protected_var;
}

NOINLINE static void use_tls_common_addr(volatile int **addr) {
    *addr = &tls_common_var;
}

NOINLINE static void use_tls_external_addr(volatile int **addr) {
    /* External might be undefined, so check */
    if (&tls_external_var != NULL) {
        *addr = &tls_external_var;
    }
}

/* ===== VOLATILE ACCESS PATTERNS ===== */

NOINLINE static void modify_tls_via_volatile(volatile int *addr, int count) {
    for (volatile int i = 0; i < count; i++) {
        *addr += 1;
    }
}

NOINLINE static int checksum_tls_variables(void) {
    volatile int sum = 0;
    volatile int *addrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
    };
    
    for (int i = 0; i < 4; i++) {
        sum += *addrs[i];
    }
    
    return sum;
}

/* ===== CONSTRUCTOR INTERACTION ===== */

__attribute__((constructor))
static void init_tls_in_constructor(void) {
    /* This should mark DECL_PRESERVE_P on tls_preserved_var */
    tls_preserved_var = 1234;
    
    /* Also touch other TLS variables */
    tls_common_var = 555;
}

/* ===== MIXED CONTROL FLOW ===== */

NOINLINE static void conditional_tls_access(volatile int selector) {
    volatile int *target = NULL;
    
    switch (selector % 4) {
        case 0:
            target = &tls_weak_var;
            break;
        case 1:
            target = &tls_hidden_var;
            break;
        case 2:
            target = &tls_protected_var;
            break;
        case 3:
            target = &tls_common_var;
            break;
    }
    
    if (target) {
        *target += selector;
    }
}

/* ===== EXTERNAL TLS DEFINITION (simulating another TU) ===== */
__thread int tls_external_var = 777;

/* For DLL import simulation */
__thread int tls_dllimport_var = 888;

/* ===== NAMESPACE TEST (C++ style simulated in C) ===== */
#ifdef __cplusplus
namespace TestNamespace {
    __thread int tls_in_namespace = 333;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls() {
            tls_in_namespace++;
        }
    };
}
#else
/* C version - use static to simulate namespace */
static __thread int tls_namespace_sim = 333;
#endif

/* ===== MAIN EXECUTION FLOW ===== */

int main(void) {
    volatile int selector = 0;
    
    /* 1. Take addresses of all TLS variables */
    volatile int *addr1, *addr2, *addr3, *addr4;
    
    use_tls_weak_addr(&addr1);
    use_tls_hidden_addr(&addr2);
    use_tls_protected_addr(&addr3);
    use_tls_common_addr(&addr4);
    
    /* 2. Modify via volatile pointers */
    modify_tls_via_volatile(addr1, 5);
    modify_tls_via_volatile(addr2, 3);
    modify_tls_via_volatile(addr3, 2);
    modify_tls_via_volatile(addr4, 1);
    
    /* 3. Conditional access in loop */
    for (volatile int i = 0; i < 10; i++) {
        conditional_tls_access(i);
        selector = i; /* Volatile write to prevent loop optimization */
    }
    
    /* 4. Use function-static TLS */
    func_with_static_tls();
    
    /* 5. C++ namespace style test */
    #ifdef __cplusplus
    {
        TestNamespace::TLSUser user;
        user.use_namespace_tls();
    }
    #else
    {
        tls_namespace_sim++;
    }
    #endif
    
    /* 6. Compute and print checksum */
    int sum = checksum_tls_variables();
    sum += tls_preserved_var;
    
    #ifdef __cplusplus
    sum += TestNamespace::tls_in_namespace;
    #else
    sum += tls_namespace_sim;
    #endif
    
    printf("TLS checksum: %d\n", sum);
    printf("tls_preserved_var: %d\n", tls_preserved_var);
    
    /* Return value based on checksum for verification */
    return (sum > 0 && sum < 100000) ? 0 : 1;
}
