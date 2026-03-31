/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even on platforms with native support */
#pragma GCC target("tls")

/* Disable inlining to ensure TLS variable addresses are taken */
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

/* 6. DLL import style (simulated with dllimport attribute if supported) */
#ifdef __MINGW32__
__declspec(dllimport) __thread int tls_dllimport_var;
#else
/* On non-Windows, use weak to simulate similar behavior */
__thread int tls_dllimport_var __attribute__((weak));
#endif

/* 7. Static TLS inside a function (tests DECL_CONTEXT) */
static void function_with_static_tls(void) {
    static __thread int static_tls_inside_func = 999;
    volatile int* volatile ptr = &static_tls_inside_func;
    *ptr += 1; /* Force access through volatile pointer */
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_preserved_var __attribute__((used)) = 333;

/* ========== C++ SPECIFIC TESTS (if compiled as C++) ========== */
#ifdef __cplusplus
namespace TLS_Namespace {
    /* 9. TLS in namespace */
    __thread int tls_in_namespace = 555;
    
    /* 10. Weak TLS in namespace */
    __thread int tls_namespace_weak __attribute__((weak)) = 777;
}

class TLS_Class {
public:
    /* 11. Static member TLS (C++ specific) */
    static __thread int static_member_tls;
    
    void access_tls() {
        volatile int* p = &static_member_tls;
        *p = 1234;
    }
};

__thread int TLS_Class::static_member_tls = 0;
#endif

/* ========== HELPER FUNCTIONS ========== */

/* Force address-taking of TLS variables */
NOINLINE static void use_tls_weak(int* out) {
    volatile int* volatile ptr = &tls_weak_var;
    *out = *ptr;
    *ptr += 1; /* Modify through volatile pointer */
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
    /* Initialize common variable if not already */
    if (tls_common_var == 0) {
        tls_common_var = 300;
    }
    volatile int* volatile ptr = &tls_common_var;
    *out = *ptr;
    *ptr += 4;
}

NOINLINE static void use_tls_external(int* out) {
    /* External might be defined elsewhere, but we'll define it here
       for completeness in this test */
    static int tls_external_var_definition = 400;
    volatile int* volatile ptr = (volatile int*)&tls_external_var;
    *out = *ptr ? *ptr : tls_external_var_definition;
}

NOINLINE static void use_tls_preserved(int* out) {
    volatile int* volatile ptr = &tls_preserved_var;
    *out = *ptr;
    *ptr += 5;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTION ========== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* Access and modify TLS in constructor - tests DECL_PRESERVE_P */
    tls_hidden_var = 999;
    tls_protected_var = 888;
    
#ifdef __cplusplus
    TLS_Namespace::tls_in_namespace = 1111;
#endif
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Final TLS access */
    volatile int dummy = tls_weak_var + tls_hidden_var;
    (void)dummy; /* Suppress unused warning */
}

/* ========== MAIN TEST FUNCTION ========== */

int main(void) {
    volatile int selector = 0;
    int results[6] = {0};
    int checksum = 0;
    
    /* Force TREE_USED on all TLS variables by taking addresses */
    volatile int* addrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_external_var,
        &tls_preserved_var,
#ifdef __cplusplus
        &TLS_Namespace::tls_in_namespace,
        &TLS_Class::static_member_tls,
#endif
        &tls_dllimport_var
    };
    
    /* Access static TLS inside function */
    function_with_static_tls();
    
    /* Loop with volatile selector to prevent optimization */
    for (selector = 0; selector < 6; selector++) {
        switch (selector) {
            case 0: use_tls_weak(&results[0]); break;
            case 1: use_tls_hidden(&results[1]); break;
            case 2: use_tls_protected(&results[2]); break;
            case 3: use_tls_common(&results[3]); break;
            case 4: use_tls_external(&results[4]); break;
            case 5: use_tls_preserved(&results[5]); break;
        }
    }
    
#ifdef __cplusplus
    /* C++ specific tests */
    TLS_Class obj;
    obj.access_tls();
    
    volatile int* ns_ptr = &TLS_Namespace::tls_in_namespace;
    *ns_ptr += 10;
#endif
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 6; i++) {
        checksum += results[i];
    }
    
    /* Mix in address values (LSB only to avoid overflow issues) */
    for (size_t i = 0; i < sizeof(addrs)/sizeof(addrs[0]); i++) {
        checksum += ((long)addrs[i] & 0xFF);
    }
    
    /* Print checksum so compiler can't eliminate everything */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* ========== DEFINITION FOR EXTERNAL TLS ========== */
/* Define it here to avoid linker errors, but keep the external declaration */
__thread int tls_external_var = 500;

/* ========== ADDITIONAL TLS IN DIFFERENT CONTEXT ========== */
/* File-scope static TLS */
static __thread int static_file_tls = 600;

/* Another function using it */
NOINLINE static void use_static_file_tls(void) {
    volatile int* volatile ptr = &static_file_tls;
    *ptr += 7;
}
