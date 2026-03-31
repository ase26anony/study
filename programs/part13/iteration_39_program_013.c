/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 100;

/* 3. Common linkage TLS (tentative definition) */
__thread int tls_common __attribute__((common));

/* 4. External TLS declaration (defined later) */
extern __thread int tls_external;

/* 5. Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 6. DLL import simulation (using weak as proxy) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((weak)) = 300;
#endif

/* 7. Static TLS inside function (tests DECL_CONTEXT) */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 400;
    volatile int* volatile ptr = &tls_func_static;
    *ptr += 1; /* Force usage through volatile */
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_preserved __attribute__((used)) = 500;

/* ========== C++ SPECIFIC TESTS (compile as C++) ========== */
#ifdef __cplusplus
namespace TLSNameSpace {
    /* 9. TLS in namespace with internal linkage */
    static __thread int tls_namespace_internal = 600;
    
    /* 10. Public TLS in namespace */
    __thread int tls_namespace_public = 700;
    
    class TLSClass {
    public:
        /* 11. Static TLS member */
        static __thread int tls_member;
        
        void modify_tls() {
            volatile int* p = &tls_member;
            *p = 800;
        }
    };
    
    __thread int TLSClass::tls_member = 0;
}
#endif

/* ========== HELPER FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE static void use_tls_weak_hidden(volatile int** out) {
    *out = &tls_weak_hidden;
    tls_weak_hidden++; /* Force TREE_USED */
}

NOINLINE static void use_tls_public_default(int increment) {
    volatile int* p = &tls_public_default;
    *p += increment;
}

NOINLINE static void use_tls_common_via_ptr(void) {
    /* Take address in complex way */
    int* ptr = &tls_common;
    volatile int** vptr = (volatile int**)&ptr;
    **vptr = 123;
}

NOINLINE static void use_tls_external_ref(void) {
    /* External reference forces DECL_EXTERNAL handling */
    extern __thread int tls_external;
    volatile int* p = &tls_external;
    (void)p;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

CONSTRUCTOR static void init_tls_values(void) {
    /* Tests DECL_PRESERVE_P propagation */
    tls_public_default = 999;
    tls_protected = 888;
    
#ifdef __cplusplus
    TLSNameSpace::tls_namespace_public = 777;
#endif
}

DESTRUCTOR static void cleanup_tls(void) {
    /* Ensure TLS variables are preserved through destructors */
    volatile int dummy = tls_public_default;
    (void)dummy;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* Defined separately to test DECL_EXTERNAL -> DECL_COMMON transition */
__thread int tls_external = 250;

/* ========== MAIN TEST ROUTINE ========== */

int main(void) {
    volatile int selector = 0;
    uintptr_t checksum = 0;
    
    /* Force usage of function with static TLS */
    func_with_static_tls();
    
    /* Take addresses in non-inlineable functions */
    volatile int* ptr1;
    use_tls_weak_hidden(&ptr1);
    
    use_tls_public_default(5);
    use_tls_common_via_ptr();
    use_tls_external_ref();
    
    /* Conditional access based on volatile selector */
    for (selector = 0; selector < 10; selector++) {
        volatile int* current = NULL;
        
        switch (selector % 4) {
            case 0:
                current = &tls_weak_hidden;
                break;
            case 1:
                current = &tls_public_default;
                break;
            case 2:
                current = &tls_protected;
                break;
            case 3:
                current = &tls_dllimport;
                break;
        }
        
        if (current) {
            *current += (int)selector;
        }
    }
    
#ifdef __cplusplus
    /* C++ specific tests */
    TLSNameSpace::TLSClass obj;
    obj.modify_tls();
    
    volatile int* ns_ptr = &TLSNameSpace::tls_namespace_internal;
    *ns_ptr = 901;
#endif
    
    /* Compute checksum of all TLS addresses and values */
    checksum = (uintptr_t)&tls_weak_hidden;
    checksum ^= (uintptr_t)&tls_public_default * 31;
    checksum ^= (uintptr_t)&tls_common * 31 * 31;
    checksum ^= (uintptr_t)&tls_external * 31 * 31 * 31;
    checksum ^= (uintptr_t)&tls_protected * 31 * 31 * 31 * 31;
    checksum ^= (uintptr_t)&tls_dllimport * 31 * 31 * 31 * 31 * 31;
    checksum ^= (uintptr_t)&tls_preserved * 31 * 31 * 31 * 31 * 31 * 31;
    
#ifdef __cplusplus
    checksum ^= (uintptr_t)&TLSNameSpace::tls_namespace_internal;
    checksum ^= (uintptr_t)&TLSNameSpace::tls_namespace_public;
    checksum ^= (uintptr_t)&TLSNameSpace::TLSClass::tls_member;
#endif
    
    /* Mix in values */
    checksum += tls_weak_hidden;
    checksum += tls_public_default;
    checksum += tls_common;
    checksum += tls_external;
    checksum += tls_protected;
    checksum += tls_dllimport;
    checksum += tls_preserved;
    
#ifdef __cplusplus
    checksum += TLSNameSpace::tls_namespace_internal;
    checksum += TLSNameSpace::tls_namespace_public;
    checksum += TLSNameSpace::TLSClass::tls_member;
#endif
    
    printf("TLS checksum: 0x%lx\n", (unsigned long)checksum);
    
    /* Runtime verification of emulated TLS */
    printf("TLS variable addresses:\n");
    printf("  tls_weak_hidden: %p\n", (void*)&tls_weak_hidden);
    printf("  tls_public_default: %p\n", (void*)&tls_public_default);
    printf("  tls_external: %p\n", (void*)&tls_external);
    
    return (int)(checksum & 0x7FFFFFFF);
}
