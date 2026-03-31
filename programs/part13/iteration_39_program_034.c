/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if native is available */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS variable usage */
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

/* 5. External declaration (defined in another TU would be) */
extern __thread int tls_external_var;

/* 6. DLL import simulation (Windows-style) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#else
/* Simulate with visibility and weak */
__thread int tls_dllimport_var __attribute__((weak, visibility("default")));
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 999;
    volatile int* p = &tls_func_static;
    *p += 1;
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_preserved_var __attribute__((used)) = 333;

/* ===== C++ SPECIFIC TESTS (compile as C++ for these) ===== */
#ifdef __cplusplus
namespace TLSNameSpace {
    __thread int tls_in_namespace = 555;
    
    class TLSClass {
    public:
        static __thread int tls_in_class;
        __thread int tls_member;  /* Instance-specific TLS */
        
        void modify_tls() {
            volatile int* p = &tls_in_class;
            *p += 10;
        }
    };
    
    __thread int TLSClass::tls_in_class = 777;
}

/* Local class using TLS */
void test_cpp_tls() {
    class LocalClass {
    public:
        static __thread int local_tls;
        void modify() {
            volatile int* p = &local_tls;
            *p += 5;
        }
    };
    
    __thread int LocalClass::local_tls = 888;
    LocalClass lc;
    lc.modify();
}
#endif

/* ===== HELPER FUNCTIONS TO FORCE TLS USAGE ===== */

NOINLINE void use_tls_weak(int* out) {
    volatile int* p = &tls_weak_var;
    *out = *p;
    *p += 1;
}

NOINLINE void use_tls_hidden(int* out) {
    volatile int* p = &tls_hidden_var;
    *out = *p;
    *p += 2;
}

NOINLINE void use_tls_protected(int* out) {
    volatile int* p = &tls_protected_var;
    *out = *p;
    *p += 3;
}

NOINLINE void use_tls_common(int* out) {
    /* Initialize common variable */
    tls_common_var = 300;
    volatile int* p = &tls_common_var;
    *out = *p;
    *p += 4;
}

NOINLINE void use_tls_external(int* out) {
    /* Define the external variable locally for this test */
    __thread int tls_external_var = 400;
    volatile int* p = &tls_external_var;
    *out = *p;
    *p += 5;
}

NOINLINE void use_tls_dllimport(int* out) {
    volatile int* p = &tls_dllimport_var;
    /* Initialize if not already */
    static int initialized = 0;
    if (!initialized) {
        *p = 600;
        initialized = 1;
    }
    *out = *p;
    *p += 6;
}

NOINLINE void use_tls_preserved(int* out) {
    volatile int* p = &tls_preserved_var;
    *out = *p;
    *p += 7;
}

/* Constructor that accesses TLS */
CONSTRUCTOR static void init_tls_in_constructor(void) {
    volatile int* p = &tls_preserved_var;
    *p = 999;  /* Different initial value */
    
    /* Also touch hidden TLS */
    p = &tls_hidden_var;
    *p = 111;
}

/* Destructor that verifies TLS */
DESTRUCTOR static void verify_tls_in_destructor(void) {
    /* Just access to ensure TLS is alive */
    volatile int dummy = tls_preserved_var;
    (void)dummy;
}

/* ===== MAIN TEST FUNCTION ===== */

int main(void) {
    int results[8] = {0};
    volatile int selector = 0;
    
    /* Force function context TLS usage */
    func_with_static_tls();
    
    /* Loop with volatile selector to prevent optimization */
    for (selector = 0; selector < 7; selector++) {
        switch (selector) {
            case 0: use_tls_weak(&results[0]); break;
            case 1: use_tls_hidden(&results[1]); break;
            case 2: use_tls_protected(&results[2]); break;
            case 3: use_tls_common(&results[3]); break;
            case 4: use_tls_external(&results[4]); break;
            case 5: use_tls_dllimport(&results[5]); break;
            case 6: use_tls_preserved(&results[6]); break;
        }
    }
    
    /* C++ specific tests if compiled as C++ */
#ifdef __cplusplus
    TLSNameSpace::tls_in_namespace = 123;
    volatile int* p = &TLSNameSpace::tls_in_namespace;
    results[7] = *p;
    
    TLSNameSpace::TLSClass obj;
    obj.modify_tls();
    results[7] += TLSNameSpace::TLSClass::tls_in_class;
    
    test_cpp_tls();
#endif
    
    /* Compute checksum to prevent elimination */
    uint32_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum = (checksum << 3) ^ results[i];
    }
    
    /* Also take addresses in main to ensure TREE_USED is set */
    int* addrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_preserved_var,
    };
    
    /* Volatile access to addresses */
    volatile uintptr_t addr_sum = 0;
    for (int i = 0; i < 5; i++) {
        addr_sum += (uintptr_t)addrs[i];
    }
    
    printf("TLS test checksum: 0x%08x\n", checksum);
    printf("Address sum: 0x%lx\n", (unsigned long)addr_sum);
    
    /* Runtime check for emulated TLS */
    printf("Testing emulated TLS...\n");
    
    return 0;
}
