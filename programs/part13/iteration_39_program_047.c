/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if native TLS is available */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===================== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===================== */

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

/* 6. DLL import simulation (Windows-style) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#else
/* Simulate with visibility and weak */
__thread int tls_dllimport_var __attribute__((weak, visibility("default")));
#endif

/* 7. Static TLS inside function will be tested separately */
/* 8. Preserved TLS (used in constructor) */
__thread int tls_preserved_var = 300;

/* 9. Used flag test - marked used via attribute */
__thread int tls_used_var __attribute__((used)) = 400;

/* ===================== FUNCTION DECLARATIONS ===================== */

NOINLINE static void use_tls_address(int *ptr);
NOINLINE static void modify_tls_via_volatile(volatile int *ptr);
NOINLINE static int compute_tls_checksum(void);

/* Constructor that uses TLS */
CONSTRUCTOR static void init_tls_values(void);

/* Destructor that verifies TLS */
DESTRUCTOR static void verify_tls_values(void);

/* ===================== TLS VARIABLE DEFINITIONS ===================== */

/* Define the external TLS variable */
__thread int tls_external_var = 500;

/* Define the DLL import variable */
__thread int tls_dllimport_var = 600;

/* ===================== HELPER FUNCTIONS ===================== */

NOINLINE static void use_tls_address(int *ptr) {
    /* Force TLS variable to be marked as used */
    static volatile int sink;
    sink = *ptr;
    (void)sink;
}

NOINLINE static void modify_tls_via_volatile(volatile int *ptr) {
    /* Volatile access prevents optimization */
    *ptr = *ptr + 1;
}

NOINLINE static int compute_tls_checksum(void) {
    /* Compute checksum of all TLS variables to ensure they're all used */
    int sum = 0;
    
    /* Access all TLS variables in different ways */
    sum += tls_weak_var;
    sum += tls_hidden_var;
    sum += tls_protected_var;
    sum += tls_common_var;
    sum += tls_external_var;
    sum += tls_dllimport_var;
    sum += tls_preserved_var;
    sum += tls_used_var;
    
    /* Take addresses to force TLS structure generation */
    volatile int *addrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_external_var,
        &tls_dllimport_var,
        &tls_preserved_var,
        &tls_used_var,
        NULL
    };
    
    /* Add address values to checksum (lower bits only) */
    for (int i = 0; addrs[i]; i++) {
        sum += (uintptr_t)addrs[i] & 0xFF;
    }
    
    return sum;
}

CONSTRUCTOR static void init_tls_values(void) {
    /* This constructor ensures DECL_PRESERVE_P is set */
    tls_preserved_var = 999;
    tls_common_var = 777;
    
    /* Take address in constructor */
    volatile int *addr = &tls_preserved_var;
    (void)addr;
}

DESTRUCTOR static void verify_tls_values(void) {
    /* Verify TLS values are accessible in destructor */
    volatile int check = tls_preserved_var;
    (void)check;
}

/* ===================== MAIN FUNCTION WITH COMPLEX CONTROL FLOW ===================== */

int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* Phase 1: Force TREE_USED flag by taking addresses */
    use_tls_address(&tls_weak_var);
    use_tls_address(&tls_hidden_var);
    use_tls_address(&tls_protected_var);
    use_tls_address(&tls_external_var);
    
    /* Phase 2: Volatile modifications in loop */
    for (volatile int i = 0; i < 3; i++) {
        selector = i;
        
        switch (selector) {
            case 0:
                modify_tls_via_volatile(&tls_weak_var);
                break;
            case 1:
                modify_tls_via_volatile(&tls_hidden_var);
                break;
            case 2:
                modify_tls_via_volatile(&tls_protected_var);
                break;
            default:
                modify_tls_via_volatile(&tls_external_var);
                break;
        }
    }
    
    /* Phase 3: Test static TLS inside function */
    {
        static __thread int tls_static_func_var = 800;
        volatile int *static_tls_addr = &tls_static_func_var;
        *static_tls_addr += 1;
        use_tls_address(&tls_static_func_var);
    }
    
    /* Phase 4: Conditional access based on volatile */
    volatile int *tls_ptr = NULL;
    
    for (volatile int j = 0; j < 8; j++) {
        switch (j % 4) {
            case 0: tls_ptr = (volatile int *)&tls_weak_var; break;
            case 1: tls_ptr = (volatile int *)&tls_hidden_var; break;
            case 2: tls_ptr = (volatile int *)&tls_protected_var; break;
            case 3: tls_ptr = (volatile int *)&tls_external_var; break;
        }
        
        if (tls_ptr) {
            *tls_ptr += j;
        }
    }
    
    /* Phase 5: Compute final checksum */
    checksum = compute_tls_checksum();
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional check for emulated TLS structure */
    printf("TLS variable addresses:\n");
    printf("  weak: %p\n", (void*)&tls_weak_var);
    printf("  hidden: %p\n", (void*)&tls_hidden_var);
    printf("  protected: %p\n", (void*)&tls_protected_var);
    printf("  external: %p\n", (void*)&tls_external_var);
    
    return 0;
}

/* ===================== C++ SPECIFIC TESTS (if compiled as C++) ===================== */
#ifdef __cplusplus

namespace tls_test {
    /* TLS in namespace */
    __thread int tls_namespace_var = 1234;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls() {
            /* Force TLS usage from class method */
            volatile int *addr = &tls_namespace_var;
            *addr += 1;
        }
        
        NOINLINE static void static_use_tls() {
            /* Static method using TLS */
            static __thread int tls_static_member_var = 4321;
            volatile int *addr = &tls_static_member_var;
            *addr += 2;
        }
    };
}

/* Additional main for C++ */
#ifdef __cplusplus
int cpp_tls_test() {
    tls_test::TLSUser user;
    user.use_namespace_tls();
    tls_test::TLSUser::static_use_tls();
    
    /* Use namespace TLS variable */
    volatile int *ns_addr = &tls_test::tls_namespace_var;
    *ns_addr += 3;
    
    return 0;
}
#endif

#endif /* __cplusplus */
