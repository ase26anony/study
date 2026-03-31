/* tls_emulation_test.c - Test program for GCC emulated TLS attribute copying */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))

/* Helper for checksum calculation */
static inline uintptr_t mix_ptr(uintptr_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    return x;
}

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 100;

/* 3. Protected visibility TLS variable */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - requires -fno-common to test properly */
__thread int tls_common;

/* 5. External declaration (defined in another TU or later) */
extern __thread int tls_external;

/* 6. DLL import simulation (using dllimport attribute when available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* On non-Windows, use weak to simulate similar behavior */
    __thread int tls_dllimport __attribute__((weak)) = 300;
#endif

/* 7. Static TLS inside a function context (tests DECL_CONTEXT) */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 400;
    volatile int* volatile ptr = &tls_func_static;
    (void)ptr; /* Use pointer to prevent optimization */
}

/* 8. TLS with constructor interaction */
__thread int tls_with_constructor = 0;

/* 9. TLS variable that will be marked as used through address-taking */
__thread int tls_address_taken = 500;

/* ===== NOINLINE FUNCTIONS THAT TAKE ADDRESSES ===== */

NOINLINE static void use_tls_weak_hidden(volatile int** ptr) {
    *ptr = &tls_weak_hidden;
}

NOINLINE static void use_tls_public_default(volatile int** ptr) {
    *ptr = &tls_public_default;
}

NOINLINE static void use_tls_protected(volatile int** ptr) {
    *ptr = &tls_protected;
}

NOINLINE static void use_tls_address_taken(volatile int** ptr) {
    *ptr = &tls_address_taken;
}

/* Function that forces TREE_USED marking */
NOINLINE static void mark_tls_used(void) {
    /* Access TLS variables in ways that force TREE_USED */
    if (tls_public_default > 0) {
        tls_public_default++;
    }
    
    /* Take address in conditional */
    volatile int* ptr = (tls_protected > 0) ? &tls_protected : &tls_weak_hidden;
    (void)ptr;
}

/* ===== CONSTRUCTOR FUNCTION ===== */

/* Constructor that interacts with TLS (tests DECL_PRESERVE_P) */
static void __attribute__((constructor)) tls_constructor(void) {
    tls_with_constructor = 0xABCD;
    
    /* Also access other TLS variables */
    tls_common = 999;
    
    /* Force address-taking in constructor */
    volatile int* volatile ptr = &tls_with_constructor;
    (void)ptr;
}

/* ===== VOLATILE ACCESS PATTERNS ===== */

NOINLINE static uintptr_t volatile_tls_access(volatile int selector) {
    volatile int* tls_ptr = NULL;
    volatile int counter = selector;
    uintptr_t checksum = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < counter; i++) {
        switch (i % 4) {
            case 0:
                use_tls_weak_hidden((volatile int**)&tls_ptr);
                break;
            case 1:
                use_tls_public_default((volatile int**)&tls_ptr);
                break;
            case 2:
                use_tls_protected((volatile int**)&tls_ptr);
                break;
            case 3:
                use_tls_address_taken((volatile int**)&tls_ptr);
                break;
        }
        
        if (tls_ptr) {
            /* Mix pointer value into checksum */
            checksum = mix_ptr(checksum ^ (uintptr_t)tls_ptr);
            
            /* Volatile write/read to prevent elimination */
            volatile int val = *tls_ptr;
            *tls_ptr = val + 1;
        }
    }
    
    return checksum;
}

/* ===== MAIN FUNCTION WITH COMPLEX CONTROL FLOW ===== */

int main(void) {
    uintptr_t checksum = 0;
    volatile int selector = 8; /* Force multiple loop iterations */
    
    /* 1. Call functions that take addresses of TLS variables */
    volatile int* ptr1, *ptr2, *ptr3, *ptr4;
    
    use_tls_weak_hidden((volatile int**)&ptr1);
    use_tls_public_default((volatile int**)&ptr2);
    use_tls_protected((volatile int**)&ptr3);
    use_tls_address_taken((volatile int**)&ptr4);
    
    /* Mix addresses into checksum */
    checksum ^= mix_ptr((uintptr_t)ptr1);
    checksum ^= mix_ptr((uintptr_t)ptr2);
    checksum ^= mix_ptr((uintptr_t)ptr3);
    checksum ^= mix_ptr((uintptr_t)ptr4);
    
    /* 2. Force TREE_USED marking */
    mark_tls_used();
    
    /* 3. Call function with static TLS */
    func_with_static_tls();
    
    /* 4. Volatile access pattern with loop */
    checksum ^= volatile_tls_access(selector);
    
    /* 5. Direct TLS variable usage with side effects */
    tls_public_default += tls_protected;
    tls_address_taken = tls_weak_hidden * 2;
    
    /* 6. Compute final checksum from TLS values */
    checksum ^= mix_ptr(tls_weak_hidden);
    checksum ^= mix_ptr(tls_public_default);
    checksum ^= mix_ptr(tls_protected);
    checksum ^= mix_ptr(tls_common);
    checksum ^= mix_ptr(tls_with_constructor);
    checksum ^= mix_ptr(tls_address_taken);
    
    /* 7. Print checksum (prevents dead code elimination) */
    printf("TLS checksum: 0x%lx\n", (unsigned long)checksum);
    
    /* 8. Runtime verification of emulated TLS */
    printf("TLS variable addresses:\n");
    printf("  tls_weak_hidden: %p\n", (void*)&tls_weak_hidden);
    printf("  tls_public_default: %p\n", (void*)&tls_public_default);
    printf("  tls_protected: %p\n", (void*)&tls_protected);
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* ===== EXTERNAL TLS DEFINITION (for testing extern) ===== */
__thread int tls_external = 600;

/* ===== C++ VERSION (if compiled as C++) ===== */
#ifdef __cplusplus
namespace tls_test {
    /* TLS in namespace with internal linkage */
    static __thread int tls_namespace_static = 700;
    
    /* Public TLS in namespace */
    __thread int tls_namespace_public = 800;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls() {
            /* Take address of namespace TLS */
            volatile int* ptr1 = &tls_namespace_static;
            volatile int* ptr2 = &tls_namespace_public;
            
            /* Use them to prevent optimization */
            *ptr1 += 1;
            *ptr2 += 2;
            
            /* Local class with TLS usage */
            class LocalClass {
            public:
                volatile int* get_tls_ptr() {
                    return &tls_namespace_public;
                }
            };
            
            LocalClass local;
            volatile int* ptr3 = local.get_tls_ptr();
            (void)ptr3;
        }
    };
}

/* C++ main wrapper */
extern "C" int run_cpp_tls_test(void) {
    tls_test::TLSUser user;
    user.use_namespace_tls();
    return tls_test::tls_namespace_static + tls_test::tls_namespace_public;
}
#endif
