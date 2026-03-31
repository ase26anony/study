/* tls_emulation_test.c - Test program for GCC emulated TLS attribute copying */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
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
    /* On non-Windows, use weak external to simulate similar behavior */
    extern __thread int tls_dllimport_var __attribute__((weak));
#endif

/* 7. Static TLS inside a function context (tests DECL_CONTEXT) */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 999;
    tls_function_static++;
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_used_var __attribute__((used)) = 333;

/* 9. Public TLS variable with explicit visibility */
__thread int tls_public_var __attribute__((visibility("default"))) = 444;

/* ========== C++ SPECIFIC TESTS (compile as C++) ========== */
#ifdef __cplusplus
namespace TLSNameSpace {
    __thread int tls_namespace_var = 555;
    
    class TLSClass {
    public:
        static __thread int tls_class_var;
        __thread int tls_member_var;
        
        void method() {
            tls_member_var = tls_class_var + tls_namespace_var;
        }
    };
    
    __thread int TLSClass::tls_class_var = 666;
}
#endif

/* ========== HELPER FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE static void use_tls_address_weak(volatile int** ptr) {
    *ptr = &tls_weak_var;
    (**ptr)++;  /* Force memory access through pointer */
}

NOINLINE static void use_tls_address_hidden(volatile int** ptr) {
    *ptr = &tls_hidden_var;
    (**ptr) += 2;
}

NOINLINE static void use_tls_address_protected(volatile int** ptr) {
    *ptr = &tls_protected_var;
    (**ptr) += 3;
}

NOINLINE static void use_tls_address_common(volatile int** ptr) {
    *ptr = &tls_common_var;
    (**ptr) += 4;
}

NOINLINE static void use_tls_address_external(volatile int** ptr) {
    /* External might be undefined, so check first */
    if (&tls_external_var != NULL) {
        *ptr = &tls_external_var;
        (**ptr) += 5;
    }
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

CONSTRUCTOR static void tls_constructor(void) {
    /* Access and modify TLS in constructor - tests DECL_PRESERVE_P */
    tls_used_var = 0xABCD;
    tls_hidden_var = 0x1234;
    
    /* Force address taking in constructor */
    volatile int* volatile ptr = &tls_protected_var;
    *ptr = 777;
}

DESTRUCTOR static void tls_destructor(void) {
    /* Verify TLS is still accessible in destructor */
    volatile int check = tls_used_var;
    (void)check; /* Suppress unused warning */
}

/* ========== COMPLEX CONTROL FLOW WITH TLS ========== */

NOINLINE static uint32_t compute_tls_checksum(volatile int selector) {
    uint32_t sum = 0;
    volatile int* ptrs[5];
    volatile int* volatile ptr;
    
    /* Initialize pointers based on selector */
    switch (selector % 5) {
        case 0: ptr = &tls_weak_var; break;
        case 1: ptr = &tls_hidden_var; break;
        case 2: ptr = &tls_protected_var; break;
        case 3: ptr = &tls_common_var; break;
        case 4: ptr = &tls_public_var; break;
        default: ptr = &tls_weak_var;
    }
    
    /* Force multiple accesses through volatile pointer */
    for (volatile int i = 0; i < 3; i++) {
        sum += *ptr + i;
        (*ptr)++;
    }
    
    /* Mix in function static TLS */
    function_with_static_tls();
    
    return sum;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* This must be in same compilation unit to avoid undefined reference */
__thread int tls_external_var = 888;

#ifdef _WIN32
    __declspec(dllexport) __thread int tls_dllimport_var = 999;
#else
    __thread int tls_dllimport_var = 999;
#endif

/* ========== MAIN EXECUTION FLOW ========== */

int main(void) {
    volatile int loop_counter;
    uint32_t final_checksum = 0;
    
    /* 1. Take addresses of all TLS variables */
    volatile int* addr_weak = &tls_weak_var;
    volatile int* addr_hidden = &tls_hidden_var;
    volatile int* addr_protected = &tls_protected_var;
    volatile int* addr_common = &tls_common_var;
    volatile int* addr_external = &tls_external_var;
    volatile int* addr_dllimport = &tls_dllimport_var;
    volatile int* addr_used = &tls_used_var;
    volatile int* addr_public = &tls_public_var;
    
    /* Force compiler to keep addresses alive */
    (void)addr_weak;
    (void)addr_hidden;
    (void)addr_protected;
    (void)addr_common;
    (void)addr_external;
    (void)addr_dllimport;
    (void)addr_used;
    (void)addr_public;
    
    /* 2. Call noinline helpers with address parameters */
    use_tls_address_weak((volatile int**)&addr_weak);
    use_tls_address_hidden((volatile int**)&addr_hidden);
    use_tls_address_protected((volatile int**)&addr_protected);
    use_tls_address_common((volatile int**)&addr_common);
    use_tls_address_external((volatile int**)&addr_external);
    
    /* 3. Complex control flow with volatile selector */
    for (loop_counter = 0; loop_counter < 10; loop_counter++) {
        volatile int selector = loop_counter;
        final_checksum += compute_tls_checksum(selector);
        
        /* Alternate between different TLS variables */
        if (selector & 1) {
            tls_weak_var += selector;
        } else {
            tls_hidden_var -= selector;
        }
    }
    
    /* 4. C++ specific tests if compiled as C++ */
    #ifdef __cplusplus
    {
        TLSNameSpace::tls_namespace_var = 1000;
        TLSNameSpace::TLSClass::tls_class_var = 2000;
        
        TLSNameSpace::TLSClass obj;
        obj.tls_member_var = 3000;
        obj.method();
        
        final_checksum += TLSNameSpace::tls_namespace_var;
        final_checksum += TLSNameSpace::TLSClass::tls_class_var;
        final_checksum += obj.tls_member_var;
    }
    #endif
    
    /* 5. Compute final checksum using all TLS variables */
    final_checksum += tls_weak_var;
    final_checksum += tls_hidden_var;
    final_checksum += tls_protected_var;
    final_checksum += tls_common_var;
    final_checksum += tls_external_var;
    final_checksum += tls_dllimport_var;
    final_checksum += tls_used_var;
    final_checksum += tls_public_var;
    
    /* 6. Print result (prevents dead code elimination) */
    printf("TLS checksum: 0x%08X\n", final_checksum);
    
    /* 7. Verify emulated TLS structure by checking addresses */
    printf("TLS variable addresses:\n");
    printf("  weak:       %p\n", (void*)&tls_weak_var);
    printf("  hidden:     %p\n", (void*)&tls_hidden_var);
    printf("  protected:  %p\n", (void*)&tls_protected_var);
    printf("  common:     %p\n", (void*)&tls_common_var);
    printf("  external:   %p\n", (void*)&tls_external_var);
    
    return (final_checksum != 0) ? 0 : 1;
}
