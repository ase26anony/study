/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")  /* Some GCC versions support this to force emulation */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) 
int tls_weak_hidden = 0x1234;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 0x5678;

/* 3. Protected visibility TLS variable */
__thread __attribute__((visibility("protected"))) 
int tls_protected = 0x9ABC;

/* 4. Common linkage (tentative definition) - requires -fno-common to test properly */
__thread int tls_common __attribute__((common));

/* 5. External declaration (defined in this file but declared extern first) */
extern __thread int tls_external;
__thread int tls_external = 0xDEF0;

/* 6. DLL import simulation (using dllimport attribute if available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* On non-Windows, use weak to simulate similar behavior */
    __thread __attribute__((weak)) int tls_dllimport;
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 0x1111;
    (void)tls_func_static; /* Reference to prevent optimization */
}

/* 8. TLS with preserve attribute (via used) */
__thread __attribute__((used)) int tls_preserved = 0x2222;

/* ========== C++ SPECIFIC TESTS (compile as C++) ========== */
#ifdef __cplusplus
namespace TLSNameSpace {
    __thread int tls_in_namespace = 0x3333;
    
    class TLSClass {
    public:
        static __thread int tls_in_class;
        __thread int tls_instance_member; /* Instance thread-local */
        
        void method() {
            /* Access TLS variables */
            volatile int* ptr = &tls_in_class;
            *ptr = 0x4444;
        }
    };
    
    __thread int TLSClass::tls_in_class = 0x5555;
}
#endif

/* ========== HELPER FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE static void use_tls_address_weak_hidden(volatile int** ptr) {
    *ptr = &tls_weak_hidden;
}

NOINLINE static void use_tls_address_public(volatile int** ptr) {
    *ptr = &tls_public_default;
}

NOINLINE static void use_tls_address_protected(volatile int** ptr) {
    *ptr = &tls_protected;
}

NOINLINE static void use_tls_address_external(volatile int** ptr) {
    *ptr = &tls_external;
}

/* Function that takes multiple TLS addresses */
NOINLINE static int compute_tls_sum(void) {
    volatile int sum = 0;
    volatile int* p1 = &tls_weak_hidden;
    volatile int* p2 = &tls_public_default;
    volatile int* p3 = &tls_protected;
    
    sum += *p1;
    sum += *p2;
    sum += *p3;
    
    return sum;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should trigger DECL_PRESERVE_P propagation */
    tls_public_default = 0x8888;
    tls_preserved = 0x9999;
    
    /* Take address in constructor */
    volatile int* volatile ptr = &tls_protected;
    *ptr = 0xAAAA;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Access TLS in destructor */
    volatile int val = tls_public_default;
    (void)val; /* Suppress unused warning */
}

/* ========== COMPLEX CONTROL FLOW ========== */

NOINLINE static void conditional_tls_access(volatile int selector) {
    volatile int* ptr = NULL;
    
    /* Complex conditional to prevent optimization */
    if (selector & 0x01) {
        ptr = &tls_weak_hidden;
    } else if (selector & 0x02) {
        ptr = &tls_public_default;
    } else if (selector & 0x04) {
        ptr = &tls_protected;
    } else if (selector & 0x08) {
        ptr = &tls_external;
    }
    
    if (ptr) {
        *ptr += selector;
    }
}

/* ========== MAIN TEST FUNCTION ========== */

int main(void) {
    volatile int checksum = 0;
    volatile int counter;
    
    /* 1. Force TREE_USED marking by taking addresses */
    volatile int* addr_weak_hidden;
    volatile int* addr_public;
    volatile int* addr_protected;
    volatile int* addr_external;
    
    use_tls_address_weak_hidden(&addr_weak_hidden);
    use_tls_address_public(&addr_public);
    use_tls_address_protected(&addr_protected);
    use_tls_address_external(&addr_external);
    
    /* 2. Access through volatile pointers */
    *addr_weak_hidden = 0x1111;
    *addr_public = 0x2222;
    *addr_protected = 0x3333;
    *addr_external = 0x4444;
    
    /* 3. Complex loop with volatile counter */
    for (counter = 0; counter < 10; counter++) {
        conditional_tls_access(counter);
        
        /* Alternate between different TLS variables */
        if (counter % 2 == 0) {
            *addr_weak_hidden += 1;
        } else {
            *addr_protected += 1;
        }
    }
    
    /* 4. Compute checksum to prevent elimination */
    checksum += *addr_weak_hidden;
    checksum += *addr_public;
    checksum += *addr_protected;
    checksum += *addr_external;
    checksum += compute_tls_sum();
    
    /* 5. Test static TLS inside function */
    func_with_static_tls();
    
    /* 6. C++ specific tests */
    #ifdef __cplusplus
    {
        using namespace TLSNameSpace;
        TLSClass obj;
        
        volatile int* ns_ptr = &tls_in_namespace;
        volatile int* class_ptr = &TLSClass::tls_in_class;
        
        *ns_ptr = 0x6666;
        *class_ptr = 0x7777;
        
        checksum += *ns_ptr;
        checksum += *class_ptr;
        
        obj.method();
    }
    #endif
    
    /* 7. Test common linkage TLS */
    tls_common = 0xCCCC;
    checksum += tls_common;
    
    /* 8. Test preserved TLS */
    checksum += tls_preserved;
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: 0x%08X\n", checksum);
    
    /* Return non-zero if any TLS variable has unexpected value */
    return (checksum == 0) ? 1 : 0;
}

/* ========== ADDITIONAL EXTERNAL REFERENCES ========== */

/* Force external reference to dllimport TLS */
void reference_dllimport_tls(void) {
    volatile int* ptr = &tls_dllimport;
    (void)ptr;
}
