/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure TLS variables are marked as used */
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

/* 5. External declaration (defined in another TU if linking) */
extern __thread int tls_external_var;

/* 6. DLL import style (simulated with weak) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#else
__thread int tls_dllimport_var __attribute__((weak));
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 999;
    volatile int* volatile ptr = &tls_func_static;
    *ptr += 1;
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_preserved_var __attribute__((used)) = 333;

/* ========== HELPER FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE static void use_tls_weak_addr(volatile int** out) {
    *out = &tls_weak_var;
}

NOINLINE static void use_tls_hidden_addr(volatile int** out) {
    *out = &tls_hidden_var;
}

NOINLINE static void use_tls_protected_addr(volatile int** out) {
    *out = &tls_protected_var;
}

NOINLINE static void use_tls_common_addr(volatile int** out) {
    *out = &tls_common_var;
}

NOINLINE static void modify_tls_via_volatile(volatile int* ptr) {
    if (ptr) {
        *ptr = (*ptr * 3) + 7;
    }
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should trigger DECL_PRESERVE_P propagation */
    tls_preserved_var = 0xABCD;
    tls_common_var = 1234;
    
    /* Take address to ensure TREE_USED is set */
    volatile int* volatile ptr = &tls_preserved_var;
    (void)ptr;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Access TLS in destructor */
    tls_hidden_var = 0;
}

/* ========== COMPLEX CONTROL FLOW WITH TLS ========== */

NOINLINE static uint32_t compute_tls_checksum(volatile int selector) {
    uint32_t sum = 0;
    volatile int* addrs[5];
    
    /* Conditional TLS address taking based on volatile selector */
    switch (selector % 5) {
        case 0:
            use_tls_weak_addr(&addrs[0]);
            sum += *addrs[0];
            break;
        case 1:
            use_tls_hidden_addr(&addrs[1]);
            sum += *addrs[1] * 2;
            break;
        case 2:
            use_tls_protected_addr(&addrs[2]);
            sum += *addrs[2] ^ 0xFF;
            break;
        case 3:
            use_tls_common_addr(&addrs[3]);
            sum += *addrs[3] + 1000;
            break;
        case 4:
            addrs[4] = &tls_preserved_var;
            sum += *addrs[4] / 2;
            break;
    }
    
    /* Loop with volatile counter accessing TLS */
    for (volatile int i = 0; i < 3; i++) {
        if (i == 0) modify_tls_via_volatile(&tls_weak_var);
        if (i == 1) modify_tls_via_volatile(&tls_hidden_var);
        if (i == 2) modify_tls_via_volatile(&tls_protected_var);
    }
    
    return sum;
}

/* ========== C++ SPECIFIC TESTS (if compiled as C++) ========== */
#ifdef __cplusplus

namespace {
    /* TLS in anonymous namespace */
    __thread int tls_anon_ns_var = 555;
}

namespace TestNamespace {
    /* TLS in named namespace */
    __thread int tls_ns_var __attribute__((visibility("default"))) = 777;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls() {
            volatile int* volatile ptr1 = &tls_ns_var;
            volatile int* volatile ptr2 = &tls_anon_ns_var;
            *ptr1 += *ptr2;
        }
    };
}

#endif /* __cplusplus */

/* ========== MAIN EXECUTION FLOW ========== */

int main(void) {
    volatile int selector = 0;
    uint32_t total_checksum = 0;
    
    /* Initialize common TLS variable */
    tls_common_var = 4321;
    
    /* Call function with static TLS */
    func_with_static_tls();
    
    /* Complex loop accessing TLS with volatile control */
    for (volatile int iter = 0; iter < 10; iter++) {
        selector = iter;
        total_checksum += compute_tls_checksum(selector);
        
        /* Alternate between different TLS variables */
        if (iter % 2 == 0) {
            tls_weak_var ^= total_checksum;
        } else {
            tls_hidden_var += iter;
        }
    }
    
#ifdef __cplusplus
    /* Use C++ TLS variables if compiled as C++ */
    TestNamespace::TLSUser user;
    user.use_namespace_tls();
    
    volatile int* volatile cpp_ptr = &TestNamespace::tls_ns_var;
    total_checksum += *cpp_ptr;
#endif
    
    /* Take addresses of all TLS variables to ensure they're marked used */
    volatile int* volatile addrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_preserved_var,
#ifdef __cplusplus
        &tls_anon_ns_var,
#endif
    };
    
    /* Compute final checksum to prevent optimization */
    for (size_t i = 0; i < sizeof(addrs)/sizeof(addrs[0]); i++) {
        if (addrs[i]) {
            total_checksum = (total_checksum * 31) + *addrs[i];
        }
    }
    
    /* Print something to prevent dead code elimination */
    printf("TLS checksum: %u (0x%08X)\n", 
           (unsigned)total_checksum, (unsigned)total_checksum);
    
    /* Runtime verification of emulated TLS */
    printf("TLS variable addresses:\n");
    printf("  weak: %p\n", (void*)&tls_weak_var);
    printf("  hidden: %p\n", (void*)&tls_hidden_var);
    printf("  protected: %p\n", (void*)&tls_protected_var);
    
    return (total_checksum == 0) ? 1 : 0;
}

/* External TLS definition (if linked) */
__thread int tls_external_var = 9999;
