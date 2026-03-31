/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even on platforms with native support */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure TLS addresses are taken */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

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
__thread int tls_dllimport_var __attribute__((weak)) = 300;
#endif

/* 7. Static TLS inside a function context */
static void function_with_static_tls(void) {
    static __thread int tls_static_func = 500;
    (void)tls_static_func;
}

/* 8. TLS with used attribute to force preservation */
__thread int tls_used_var __attribute__((used)) = 600;

/* 9. TLS in block scope (via function) */
void declare_block_scope_tls(void) {
    __thread int tls_block_scope = 700;
    volatile int* volatile ptr = &tls_block_scope;
    *ptr += 1;
}

/* ========== C++ SPECIFIC TESTS (compile as C++) ========== */
#ifdef __cplusplus
namespace TLSNameSpace {
    __thread int tls_in_namespace = 800;
    
    class TLSClass {
    public:
        static __thread int tls_in_class;
        __thread int tls_member;
        
        void method() {
            volatile int* p = &tls_member;
            *p = 900;
        }
    };
    
    __thread int TLSClass::tls_in_class = 850;
}

/* Local class with TLS */
void test_cpp_tls(void) {
    class LocalClass {
    public:
        static __thread int local_tls;
        void modify() {
            local_tls = 950;
        }
    };
    
    __thread int LocalClass::local_tls = 0;
    LocalClass obj;
    obj.modify();
}
#endif

/* ========== HELPER FUNCTIONS ========== */

/* Force address taking and usage */
NOINLINE static void use_tls_weak(int* out) {
    volatile int* volatile ptr = &tls_weak_var;
    *out = *ptr;
    *ptr += 1;
}

NOINLINE static void use_tls_hidden(int* out) {
    volatile int* volatile ptr = &tls_hidden_var;
    *out = *ptr;
    *ptr += 2;
}

NOINLINE static void use_tls_protected(int* out) {
    volatile int* volatile ptr = &tls_protected_var;
    *out = *ptr;
    *ptr += 3;
}

NOINLINE static void use_tls_common(int* out) {
    volatile int* volatile ptr = &tls_common_var;
    *out = *ptr;
    *ptr += 4;
}

NOINLINE static void use_tls_external(int* out) {
    /* External might be defined elsewhere, but we'll define it here for test */
    static __thread int tls_external_var = 400;
    volatile int* volatile ptr = &tls_external_var;
    *out = *ptr;
    *ptr += 5;
}

NOINLINE static void use_tls_dllimport(int* out) {
    volatile int* volatile ptr = &tls_dllimport_var;
    *out = *ptr;
    *ptr += 6;
}

NOINLINE static void use_tls_used(int* out) {
    volatile int* volatile ptr = &tls_used_var;
    *out = *ptr;
    *ptr += 7;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTION ========== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should run before main, testing DECL_PRESERVE_P */
    tls_hidden_var = 1234;
    tls_protected_var = 5678;
    
    /* Take address to force usage */
    volatile int* volatile ptr1 = &tls_hidden_var;
    volatile int* volatile ptr2 = &tls_protected_var;
    (void)ptr1;
    (void)ptr2;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Access TLS in destructor */
    volatile int* volatile ptr = &tls_used_var;
    *ptr = 0;
}

/* ========== MAIN TEST LOGIC ========== */

int main(void) {
    int results[8] = {0};
    volatile int selector = 0;
    
    /* Define the external TLS variable */
    __thread int tls_external_var = 400;
    
    /* Initialize common TLS */
    tls_common_var = 250;
    
    /* Call function with static TLS */
    function_with_static_tls();
    
    /* Declare block scope TLS */
    declare_block_scope_tls();
    
#ifdef __cplusplus
    /* Use C++ TLS variables */
    TLSNameSpace::tls_in_namespace = 801;
    TLSNameSpace::TLSClass obj;
    obj.tls_member = 901;
    test_cpp_tls();
#endif
    
    /* Loop with volatile selector to prevent optimization */
    for (selector = 0; selector < 8; selector++) {
        volatile int* volatile sel_ptr = &selector;
        
        switch (*sel_ptr) {
            case 0: use_tls_weak(&results[0]); break;
            case 1: use_tls_hidden(&results[1]); break;
            case 2: use_tls_protected(&results[2]); break;
            case 3: use_tls_common(&results[3]); break;
            case 4: use_tls_external(&results[4]); break;
            case 5: use_tls_dllimport(&results[5]); break;
            case 6: use_tls_used(&results[6]); break;
            default: results[7] = 999; break;
        }
    }
    
    /* Compute checksum to prevent elimination */
    uintptr_t checksum = 0;
    checksum += (uintptr_t)&tls_weak_var;
    checksum += (uintptr_t)&tls_hidden_var;
    checksum += (uintptr_t)&tls_protected_var;
    checksum += (uintptr_t)&tls_common_var;
    checksum += (uintptr_t)&tls_external_var;
    checksum += (uintptr_t)&tls_dllimport_var;
    checksum += (uintptr_t)&tls_used_var;
    
    /* Use all results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    
    printf("TLS test completed. Checksum: %lu, Sum: %d\n", 
           (unsigned long)checksum, sum);
    
    /* Final volatile access to all TLS variables */
    volatile int* volatile final_access[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_external_var,
        &tls_dllimport_var,
        &tls_used_var
    };
    
    for (size_t i = 0; i < sizeof(final_access)/sizeof(final_access[0]); i++) {
        *final_access[i] += 1;
    }
    
    return 0;
}
