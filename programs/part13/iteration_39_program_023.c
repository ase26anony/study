/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */
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

/* 5. External declaration (defined in same file but declared extern first) */
extern __thread int tls_external;
__thread int tls_external = 300;

/* 6. DLL import simulation (using dllimport attribute if supported) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* On non-Windows, use visibility to simulate similar behavior */
    __thread int tls_dllimport __attribute__((visibility("default")));
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 400;
    volatile int* volatile ptr = &tls_func_static;
    (void)ptr; /* Use pointer to prevent optimization */
}

/* 8. TLS with constructor interaction */
__thread int tls_with_constructor __attribute__((visibility("default")));

/* ===== HELPER FUNCTIONS FOR ADDRESS TAKING ===== */

NOINLINE static void use_tls_weak_hidden(volatile int** out) {
    *out = &tls_weak_hidden;
    tls_weak_hidden += 1; /* Modify to ensure it's not read-only */
}

NOINLINE static void use_tls_public_default(volatile int** out) {
    *out = &tls_public_default;
    tls_public_default ^= 0x55AA; /* Bit manipulation */
}

NOINLINE static void use_tls_protected(volatile int** out) {
    *out = &tls_protected;
    tls_protected *= 2;
}

NOINLINE static void use_tls_common(volatile int** out) {
    *out = &tls_common;
    tls_common = (tls_common + 1) % 256;
}

NOINLINE static void use_tls_external(volatile int** out) {
    *out = &tls_external;
    tls_external = ~tls_external;
}

NOINLINE static void use_tls_dllimport(volatile int** out) {
    *out = &tls_dllimport;
    tls_dllimport += 1000;
}

NOINLINE static void use_multiple_tls(volatile int** out1, volatile int** out2) {
    *out1 = &tls_weak_hidden;
    *out2 = &tls_public_default;
    tls_weak_hidden += tls_public_default;
    tls_public_default -= tls_weak_hidden;
}

/* ===== CONSTRUCTOR INTERACTION ===== */

/* Constructor that accesses TLS before main */
__attribute__((constructor)) static void init_tls_constructor(void) {
    tls_with_constructor = 0xDEADBEEF;
    tls_common = 123; /* Initialize common TLS in constructor */
    
    /* Take address to force TREE_USED */
    volatile int* volatile ptr = &tls_with_constructor;
    (void)ptr;
}

/* ===== COMPLEX CONTROL FLOW WITH VOLATILE ===== */

NOINLINE static uint32_t compute_tls_checksum(void) {
    volatile int selector = 0;
    uint32_t checksum = 0;
    
    /* Volatile loop to prevent optimization */
    for (volatile int i = 0; i < 5; i++) {
        selector = i;
        volatile int* tls_ptr = NULL;
        
        /* Conditional TLS access based on volatile selector */
        switch (selector) {
            case 0:
                use_tls_weak_hidden(&tls_ptr);
                break;
            case 1:
                use_tls_public_default(&tls_ptr);
                break;
            case 2:
                use_tls_protected(&tls_ptr);
                break;
            case 3:
                use_tls_common(&tls_ptr);
                break;
            case 4:
                use_tls_external(&tls_ptr);
                break;
        }
        
        if (tls_ptr) {
            checksum += (uint32_t)*tls_ptr;
            checksum = (checksum << 3) | (checksum >> 29); /* Rotate */
        }
    }
    
    /* Mix in DLL import TLS */
    volatile int* dll_tls_ptr = NULL;
    use_tls_dllimport(&dll_tls_ptr);
    if (dll_tls_ptr) {
        checksum ^= (uint32_t)*dll_tls_ptr;
    }
    
    /* Mix in TLS with constructor */
    checksum += tls_with_constructor;
    
    return checksum;
}

/* ===== C++ STYLE NAMESPACE TEST (in C with struct simulation) ===== */

#ifdef __cplusplus
namespace TLS_Namespace {
    __thread int tls_in_namespace __attribute__((visibility("hidden"))) = 999;
    
    class TLS_User {
    public:
        NOINLINE void modify_tls() {
            tls_in_namespace *= 3;
        }
        
        NOINLINE int get_tls_address() {
            return (int)(intptr_t)&tls_in_namespace;
        }
    };
}
#else
/* C version using struct to simulate namespace/class */
struct TLS_Namespace {
    __thread int tls_in_namespace __attribute__((visibility("hidden")));
};

static struct TLS_Namespace tls_ns = { .tls_in_namespace = 999 };

struct TLS_User {
    int dummy;
};

NOINLINE static void TLS_User_modify_tls(struct TLS_User* user) {
    (void)user;
    tls_ns.tls_in_namespace *= 3;
}

NOINLINE static int TLS_User_get_tls_address(struct TLS_User* user) {
    (void)user;
    return (int)(intptr_t)&tls_ns.tls_in_namespace;
}
#endif

/* ===== MAIN FUNCTION ===== */

int main(void) {
    uint32_t checksum = 0;
    
    /* 1. Initialize all TLS variables through usage */
    volatile int* ptr1, *ptr2;
    use_multiple_tls(&ptr1, &ptr2);
    
    /* 2. Call function with static TLS */
    func_with_static_tls();
    
    /* 3. Complex control flow with volatile access patterns */
    checksum = compute_tls_checksum();
    
    /* 4. C++/C namespace-style TLS usage */
#ifdef __cplusplus
    TLS_Namespace::TLS_User user;
    user.modify_tls();
    checksum ^= (uint32_t)user.get_tls_address();
    checksum += TLS_Namespace::tls_in_namespace;
#else
    struct TLS_User user = {0};
    TLS_User_modify_tls(&user);
    checksum ^= (uint32_t)TLS_User_get_tls_address(&user);
    checksum += tls_ns.tls_in_namespace;
#endif
    
    /* 5. Final volatile store to all TLS variables to ensure they're marked used */
    volatile int* volatile force_use[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_protected,
        &tls_common,
        &tls_external,
        &tls_dllimport,
        &tls_with_constructor
    };
    
    /* Use array to prevent optimization */
    for (volatile int i = 0; i < (int)(sizeof(force_use)/sizeof(force_use[0])); i++) {
        if (force_use[i]) {
            *force_use[i] += i;
        }
    }
    
    /* 6. Print checksum (prevents dead code elimination) */
    printf("TLS checksum: 0x%08X\n", checksum);
    
    /* 7. Verify emulated TLS by checking if addresses are non-NULL */
    printf("TLS addresses:\n");
    printf("  weak_hidden: %p\n", (void*)&tls_weak_hidden);
    printf("  public_default: %p\n", (void*)&tls_public_default);
    printf("  protected: %p\n", (void*)&tls_protected);
    printf("  with_constructor: 0x%X\n", tls_with_constructor);
    
    return (checksum != 0) ? 0 : 1;
}

/* ===== ADDITIONAL EXTERNAL DECLARATION TEST ===== */

/* In a real multi-file test, this would be in a separate file: */
/*
// tls_external_file.c
__thread int tls_defined_elsewhere __attribute__((weak, visibility("hidden")));
*/

/* Force a reference to potentially undefined weak TLS */
extern __thread int tls_undefined_weak __attribute__((weak));

/* Function that references undefined weak TLS */
NOINLINE static void reference_undefined_weak(void) {
    volatile int* volatile ptr = &tls_undefined_weak;
    (void)ptr;
}
