/* Compile with: g++ -O2 -femulated-tls -fno-common -fvisibility=hidden -o test_tls test_tls.cc */
/* Also test with: g++ -O0 -femulated-tls -fvisibility=hidden */
/* And with: g++ -O3 -femulated-tls -fno-omit-frame-pointer */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
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

/* 4. Common linkage (tentative definition) - use -fno-common to test */
__thread int tls_common_var;

/* 5. External declaration (defined in same file but declared extern first) */
extern __thread int tls_external_var;
__thread int tls_external_var = 300;

/* 6. DLL import simulation (using dllimport attribute when available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* On non-Windows, use visibility to simulate similar behavior */
    __thread int tls_dllimport_var __attribute__((visibility("default"))) = 400;
#endif

/* 7. Static TLS inside a namespace (C++ specific) */
namespace TLSNamespace {
    __thread int tls_namespace_var = 500;
    static __thread int tls_static_namespace_var = 600;
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_used_var __attribute__((used)) = 700;

/* ===== FUNCTION DECLARATIONS ===== */
NOINLINE void use_tls_address(int* addr);
NOINLINE void modify_tls_via_volatile(volatile int* addr);
NOINLINE int compute_tls_checksum();

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ===== */
CONSTRUCTOR static void init_tls_in_constructor() {
    /* This should force DECL_PRESERVE_P to be set */
    tls_used_var = 999;
    TLSNamespace::tls_namespace_var = 888;
}

DESTRUCTOR static void cleanup_tls_in_destructor() {
    /* Access TLS in destructor */
    volatile int check = tls_used_var + tls_hidden_var;
    (void)check; /* Suppress unused warning */
}

/* ===== HELPER FUNCTIONS THAT TAKE TLS ADDRESSES ===== */
NOINLINE void use_tls_address(int* addr) {
    /* Force TREE_USED to be set */
    static volatile int sink;
    sink = *addr;
    *addr += 1;
}

NOINLINE void modify_tls_via_volatile(volatile int* addr) {
    /* Volatile access prevents optimization */
    *addr = *addr + 1;
}

NOINLINE int compute_tls_checksum() {
    /* Compute checksum to prevent elimination */
    int sum = 0;
    
    /* Take addresses to force TLS usage */
    sum += tls_weak_var;
    sum += tls_hidden_var;
    sum += tls_protected_var;
    sum += tls_common_var;
    sum += tls_external_var;
    sum += tls_dllimport_var;
    sum += TLSNamespace::tls_namespace_var;
    sum += TLSNamespace::tls_static_namespace_var;
    sum += tls_used_var;
    
    return sum;
}

/* ===== FUNCTION WITH BLOCK-SCOPE TLS ===== */
NOINLINE void function_with_local_tls() {
    /* Static TLS inside function - different DECL_CONTEXT */
    static __thread int local_static_tls = 1234;
    
    /* Regular TLS in block scope */
    __thread int local_tls = 5678;
    
    /* Use both to prevent elimination */
    volatile int* volatile_ptr = &local_tls;
    *volatile_ptr += local_static_tls;
    local_static_tls = *volatile_ptr;
    
    /* Take address to force emulation */
    use_tls_address(&local_static_tls);
}

/* ===== CLASS WITH TLS USAGE (C++ SPECIFIC) ===== */
class TLSUser {
public:
    NOINLINE void use_namespace_tls() {
        /* Access namespace TLS from class method */
        TLSNamespace::tls_namespace_var++;
        tls_used_var--;
    }
    
    NOINLINE static void static_use_tls() {
        /* Static method using TLS */
        volatile int tmp = tls_hidden_var;
        tls_hidden_var = tmp + 1;
    }
};

/* ===== MAIN FUNCTION WITH COMPLEX CONTROL FLOW ===== */
int main() {
    int checksum = 0;
    volatile int selector = 0;
    
    /* Initialize common TLS variable */
    tls_common_var = 150;
    
    /* Call constructor-like behavior explicitly */
    init_tls_in_constructor();
    
    /* Use volatile loop to access different TLS variables */
    for (volatile int i = 0; i < 5; i++) {
        selector = i % 9;
        
        switch (selector) {
            case 0:
                use_tls_address(&tls_weak_var);
                break;
            case 1:
                modify_tls_via_volatile(&tls_hidden_var);
                break;
            case 2:
                use_tls_address(&tls_protected_var);
                break;
            case 3:
                tls_common_var++;
                break;
            case 4:
                modify_tls_via_volatile(&tls_external_var);
                break;
            case 5:
                use_tls_address(&tls_dllimport_var);
                break;
            case 6:
                TLSNamespace::tls_namespace_var += 2;
                break;
            case 7:
                tls_used_var--;
                break;
            case 8:
                function_with_local_tls();
                break;
        }
    }
    
    /* Use C++ class with TLS */
    TLSUser user;
    user.use_namespace_tls();
    TLSUser::static_use_tls();
    
    /* Compute final checksum */
    checksum = compute_tls_checksum();
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional volatile access pattern */
    volatile int* addrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_external_var,
        &tls_dllimport_var,
        &tls_used_var
    };
    
    for (volatile int i = 0; i < 7; i++) {
        volatile int idx = i % 7;
        volatile int* ptr = addrs[idx];
        *ptr += 1;
    }
    
    /* Final checksum */
    checksum = compute_tls_checksum();
    printf("Final TLS checksum: %d\n", checksum);
    
    return 0;
}

/* ===== EXTERN DECLARATIONS IN DIFFERENT SCOPE ===== */
/* These test DECL_EXTERNAL handling */
extern __thread int external_tls_unused;
extern __thread int external_tls_used;

/* Unused external - should still get attributes copied */
__thread int external_tls_unused;

/* Used external */
__thread int external_tls_used = 9000;

NOINLINE void use_external_tls() {
    external_tls_used++;
}
