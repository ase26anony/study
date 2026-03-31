/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure TLS addresses are taken */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with default visibility */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* 2. Hidden visibility TLS */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 100;

/* 3. Protected visibility TLS */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - no initializer */
__thread int tls_common_var;

/* 5. External declaration (defined in another TU or later) */
extern __thread int tls_external_var;

/* 6. DLL import style (simulated with weak) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* On non-Windows, simulate with weak external */
    extern __thread int tls_dllimport_var __attribute__((weak));
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 999;
    tls_func_static++;
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_used_var __attribute__((used)) = 333;

/* 9. Public TLS variable */
__thread int tls_public_var = 444;

/* 10. Non-public (static) TLS */
static __thread int tls_static_var = 555;

/* ===== FUNCTION DECLARATIONS ===== */
NOINLINE void use_tls_address(int *ptr);
NOINLINE void modify_tls_via_volatile(volatile int *ptr);
NOINLINE int compute_tls_checksum(void);

/* ===== HELPER FUNCTIONS THAT TAKE TLS ADDRESSES ===== */

NOINLINE void use_tls_address(int *ptr) {
    /* Force TLS variable to be marked as used */
    static volatile int sink;
    sink = *ptr;
}

NOINLINE void modify_tls_via_volatile(volatile int *ptr) {
    /* Volatile access prevents optimization */
    *ptr = *ptr + 1;
}

NOINLINE int compute_tls_checksum(void) {
    int sum = 0;
    
    /* Access all TLS variables through their addresses */
    sum += tls_weak_var;
    sum += tls_hidden_var;
    sum += tls_protected_var;
    sum += tls_common_var;
    
    /* External might be zero if not defined */
    sum += tls_external_var;
    
    /* Use function with static TLS */
    func_with_static_tls();
    
    sum += tls_used_var;
    sum += tls_public_var;
    sum += tls_static_var;
    
    return sum;
}

/* ===== CONSTRUCTOR THAT USES TLS ===== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should trigger DECL_PRESERVE_P propagation */
    tls_used_var = 0xABCD;
    tls_common_var = 1234;
    
    /* Take address to ensure TREE_USED is set */
    volatile int *volatile ptr = &tls_used_var;
    (void)ptr;
}

/* ===== MAIN FUNCTION WITH COMPLEX CONTROL FLOW ===== */

int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* Initialize external TLS variable (definition) */
    __thread int tls_external_var = 777;
    
    /* Initialize DLL import variable */
    __thread int tls_dllimport_var = 888;
    
    /* Loop with volatile selector to prevent optimization */
    for (selector = 0; selector < 10; selector++) {
        switch (selector) {
            case 0:
                use_tls_address(&tls_weak_var);
                break;
            case 1:
                use_tls_address(&tls_hidden_var);
                break;
            case 2:
                use_tls_address(&tls_protected_var);
                break;
            case 3:
                modify_tls_via_volatile(&tls_common_var);
                break;
            case 4:
                use_tls_address(&tls_external_var);
                break;
            case 5:
                modify_tls_via_volatile(&tls_dllimport_var);
                break;
            case 6:
                use_tls_address(&tls_used_var);
                break;
            case 7:
                modify_tls_via_volatile(&tls_public_var);
                break;
            case 8:
                use_tls_address(&tls_static_var);
                break;
            case 9:
                func_with_static_tls();
                break;
        }
    }
    
    /* Compute final checksum */
    checksum = compute_tls_checksum();
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    printf("TLS variable addresses:\n");
    printf("  weak: %p\n", (void*)&tls_weak_var);
    printf("  hidden: %p\n", (void*)&tls_hidden_var);
    printf("  protected: %p\n", (void*)&tls_protected_var);
    printf("  common: %p\n", (void*)&tls_common_var);
    
    return 0;
}

/* ===== C++ VERSION WITH NAMESPACES ===== */
#ifdef __cplusplus

namespace tls_test {
    /* TLS in namespace with different visibility */
    __thread int namespace_tls_hidden __attribute__((visibility("hidden"))) = 1111;
    __thread int namespace_tls_default = 2222;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls() {
            /* Take address of namespace TLS */
            volatile int* ptr1 = &namespace_tls_hidden;
            volatile int* ptr2 = &namespace_tls_default;
            *ptr1 += 1;
            *ptr2 += 2;
        }
        
        static __thread int class_tls_var;
    };
    
    __thread int TLSUser::class_tls_var = 3333;
}

/* Additional C++ test function */
NOINLINE void test_cpp_tls(void) {
    tls_test::TLSUser user;
    user.use_namespace_tls();
    
    /* Use class static TLS */
    tls_test::TLSUser::class_tls_var = 4444;
}

#endif /* __cplusplus */
