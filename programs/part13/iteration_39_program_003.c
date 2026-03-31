/* tls_emulation_test.c - Test program for GCC emulated TLS attribute copying */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) int tls_weak_hidden = 0x1234;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 0x5678;

/* 3. Protected visibility TLS variable */
__thread __attribute__((visibility("protected"))) int tls_protected = 0x9ABC;

/* 4. Common linkage TLS (tentative definition) - requires -fno-common to test properly */
__thread int tls_common;

/* 5. External TLS declaration (defined in same file but declared extern first) */
extern __thread int tls_external_def;
__thread int tls_external_def = 0xDEF0;

/* 6. DLL import style attribute (simulated for testing) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    __thread __attribute__((dllimport)) int tls_dllimport;
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 0x2468;
    (void)tls_func_static;
}

/* 8. TLS with preserve attribute (via used) */
__thread __attribute__((used)) int tls_preserved = 0x1357;

/* ===== C++ SPECIFIC TESTS (compile as C++ for these) ===== */
#ifdef __cplusplus
namespace TLSNameSpace {
    __thread int tls_namespace = 0x8888;
    
    class TLSClass {
    public:
        static __thread int tls_class_member;
        __thread int tls_instance_member;
        
        void method() {
            /* Force usage of TLS in method context */
            volatile int* ptr = &tls_class_member;
            *ptr = 0x9999;
        }
    };
    
    __thread int TLSClass::tls_class_member = 0x7777;
}
#endif

/* ===== HELPER FUNCTIONS THAT TAKE TLS ADDRESSES ===== */

NOINLINE static void use_tls_weak_hidden(volatile int* counter) {
    volatile int* ptr = &tls_weak_hidden;
    *ptr += *counter;
    (*counter)++;
}

NOINLINE static void use_tls_public_default(volatile int* counter) {
    volatile int* ptr = &tls_public_default;
    *ptr ^= *counter;  /* XOR to create non-trivial pattern */
    (*counter)++;
}

NOINLINE static void use_tls_protected(volatile int* counter) {
    volatile int* ptr = &tls_protected;
    *ptr |= *counter;
    (*counter)++;
}

NOINLINE static void use_tls_common(volatile int* counter) {
    volatile int* ptr = &tls_common;
    *ptr = *counter * 2;
    (*counter)++;
}

NOINLINE static void use_tls_external(volatile int* counter) {
    volatile int* ptr = &tls_external_def;
    *ptr -= *counter;
    (*counter)++;
}

/* Function that mixes multiple TLS accesses */
NOINLINE static void use_multiple_tls(volatile int* counter) {
    volatile int* ptr1 = &tls_weak_hidden;
    volatile int* ptr2 = &tls_public_default;
    volatile int* ptr3 = &tls_protected;
    
    *ptr1 += *counter;
    *ptr2 -= *counter;
    *ptr3 ^= *counter;
    (*counter)++;
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ===== */

CONSTRUCTOR static void tls_constructor(void) {
    /* Initialize TLS in constructor - tests DECL_PRESERVE_P */
    tls_public_default = 0xCAFE;
    tls_protected = 0xBABE;
    
    /* Force address taking in constructor */
    volatile int* ptr = &tls_preserved;
    *ptr = 0xDEAD;
}

DESTRUCTOR static void tls_destructor(void) {
    /* Access TLS in destructor */
    volatile int dummy = tls_public_default + tls_protected;
    (void)dummy;
}

/* ===== MAIN EXECUTION FLOW ===== */

int main(void) {
    volatile int selector = 0;
    uint32_t checksum = 0;
    
    /* Force initial usage of all TLS variables */
    volatile int* volatile ptr_array[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_protected,
        &tls_common,
        &tls_external_def,
        &tls_preserved
    };
    
    /* Loop with volatile selector to prevent optimization */
    for (selector = 0; selector < 10; selector++) {
        switch (selector % 6) {
            case 0: use_tls_weak_hidden(&selector); break;
            case 1: use_tls_public_default(&selector); break;
            case 2: use_tls_protected(&selector); break;
            case 3: use_tls_common(&selector); break;
            case 4: use_tls_external(&selector); break;
            case 5: use_multiple_tls(&selector); break;
        }
        
        /* Volatile access pattern */
        volatile int idx = selector % 6;
        checksum += *ptr_array[idx];
        checksum = (checksum << 3) | (checksum >> 29); /* Simple rotation */
    }
    
    /* Call function with static TLS */
    func_with_static_tls();
    
    /* C++ specific tests if compiled as C++ */
    #ifdef __cplusplus
    {
        TLSNameSpace::tls_namespace = checksum & 0xFF;
        TLSNameSpace::TLSClass obj;
        obj.tls_instance_member = 0x1111;
        obj.method();
        
        /* Use class static TLS */
        volatile int* cptr = &TLSNameSpace::TLSClass::tls_class_member;
        checksum += *cptr;
    }
    #endif
    
    /* Final checksum computation using all TLS variables */
    checksum += tls_weak_hidden;
    checksum += tls_public_default;
    checksum += tls_protected;
    checksum += tls_common;
    checksum += tls_external_def;
    checksum += tls_preserved;
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: 0x%08X\n", checksum);
    
    /* Runtime verification of emulated TLS */
    #ifdef __EMUTLS__
    printf("Using emulated TLS\n");
    #endif
    
    return (checksum == 0) ? 1 : 0; /* Non-zero return if all TLS worked */
}

/* ===== ADDITIONAL EXTERNAL DECLARATIONS ===== */

/* External reference to force external attribute */
extern __thread int external_tls_ref;
__thread int external_tls_ref = 0xFACE;

/* Weak external reference */
extern __thread __attribute__((weak)) int weak_external_tls_ref;
