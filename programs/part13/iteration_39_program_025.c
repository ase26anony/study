/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")  /* Some GCC versions support this to force emulation */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure TLS variables are actually referenced */
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

/* 5. External declaration (defined in another TU or later) */
extern __thread int tls_external_var;

/* 6. DLL import simulation (using dllimport attribute when available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* On non-Windows, use visibility to simulate similar behavior */
    __thread int tls_dllimport_var __attribute__((visibility("default")));
#endif

/* 7. Static TLS inside a function (tests DECL_CONTEXT) */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 999;
    volatile int* volatile ptr = &tls_func_static;
    *ptr += 1;  /* Force access through volatile pointer */
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_used_var __attribute__((used)) = 333;

/* 9. Public TLS variable (explicitly marked) */
__thread int tls_public_var __attribute__((visibility("default"))) = 444;

/* 10. Weak + hidden combination */
__thread int tls_weak_hidden_var __attribute__((weak, visibility("hidden"))) = 555;

/* ========== HELPER FUNCTIONS TO FORCE TLS USAGE ========== */

NOINLINE static void use_tls_weak(volatile int* ptr) {
    *ptr += 1;
}

NOINLINE static void use_tls_hidden(volatile int* ptr) {
    *ptr *= 2;
}

NOINLINE static void use_tls_protected(volatile int* ptr) {
    *ptr -= 10;
}

NOINLINE static void use_tls_common(volatile int* ptr) {
    static int counter = 0;
    *ptr = counter++;
}

NOINLINE static void use_tls_external(volatile int* ptr) {
    *ptr ^= 0xABCD;
}

NOINLINE static void use_tls_dllimport(volatile int* ptr) {
    *ptr |= 0x8000;
}

NOINLINE static void use_tls_used(volatile int* ptr) {
    *ptr = (*ptr << 1) | 1;
}

NOINLINE static void use_tls_public(volatile int* ptr) {
    *ptr = ~(*ptr);
}

NOINLINE static void use_tls_weak_hidden(volatile int* ptr) {
    *ptr = (*ptr + 7) * 3;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

CONSTRUCTOR static void tls_constructor(void) {
    /* Access multiple TLS variables in constructor */
    volatile int* p1 = &tls_weak_var;
    volatile int* p2 = &tls_hidden_var;
    volatile int* p3 = &tls_protected_var;
    
    *p1 = 0xDEAD;
    *p2 = 0xBEEF;
    *p3 = 0xCAFE;
    
    /* Force DECL_PRESERVE_P to matter */
    func_with_static_tls();
}

DESTRUCTOR static void tls_destructor(void) {
    /* Final TLS access */
    volatile int* p = &tls_used_var;
    *p = 0xFFFF;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This must be in the same file to avoid linking issues, but the 
   extern declaration above tests DECL_EXTERNAL copying */
__thread int tls_external_var = 1234;

/* ========== C++ SPECIFIC TESTS (if compiled as C++) ========== */
#ifdef __cplusplus
namespace tls_namespace {
    __thread int tls_in_namespace = 888;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls() {
            volatile int* ptr = &tls_in_namespace;
            *ptr += 0x1000;
        }
        
        NOINLINE static void static_use_tls() {
            __thread static int tls_in_member = 777;
            volatile int* ptr = &tls_in_member;
            *ptr += 0x2000;
        }
    };
}
#endif

/* ========== MAIN FUNCTION WITH COMPLEX CONTROL FLOW ========== */

int main(void) {
    volatile int selector = 0;
    uint32_t checksum = 0;
    
    /* Initial forced references to all TLS variables */
    volatile int* volatile ptrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_external_var,
        &tls_dllimport_var,
        &tls_used_var,
        &tls_public_var,
        &tls_weak_hidden_var,
    };
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 10; i++) {
        selector = (selector * 1103515245 + 12345) & 0x7FFFFFFF;
        int idx = selector % (sizeof(ptrs)/sizeof(ptrs[0]));
        
        /* Conditional TLS access based on volatile computation */
        switch (idx) {
            case 0: use_tls_weak(ptrs[idx]); break;
            case 1: use_tls_hidden(ptrs[idx]); break;
            case 2: use_tls_protected(ptrs[idx]); break;
            case 3: use_tls_common(ptrs[idx]); break;
            case 4: use_tls_external(ptrs[idx]); break;
            case 5: use_tls_dllimport(ptrs[idx]); break;
            case 6: use_tls_used(ptrs[idx]); break;
            case 7: use_tls_public(ptrs[idx]); break;
            case 8: use_tls_weak_hidden(ptrs[idx]); break;
        }
        
        /* Function with static TLS */
        if (selector & 1) {
            func_with_static_tls();
        }
        
        #ifdef __cplusplus
        if (selector & 2) {
            tls_namespace::TLSUser user;
            user.use_namespace_tls();
            tls_namespace::TLSUser::static_use_tls();
        }
        #endif
    }
    
    /* Compute checksum to prevent elimination */
    for (size_t j = 0; j < sizeof(ptrs)/sizeof(ptrs[0]); j++) {
        checksum = (checksum << 5) - checksum + (uint32_t)*ptrs[j];
        checksum = (checksum << 3) ^ (checksum >> 29);
    }
    
    /* Also compute checksum of addresses (tests TLS control structures) */
    uintptr_t addr_sum = 0;
    for (size_t j = 0; j < sizeof(ptrs)/sizeof(ptrs[0]); j++) {
        addr_sum += (uintptr_t)ptrs[j];
    }
    
    printf("TLS checksum: 0x%08X\n", checksum);
    printf("TLS address sum: 0x%016lX\n", (unsigned long)addr_sum);
    
    /* Force one more access pattern */
    volatile int* last_ptr = &tls_common_var;
    *last_ptr = checksum & 0xFFFF;
    
    return (checksum & 0xFF);
}
