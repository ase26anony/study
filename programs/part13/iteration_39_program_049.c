/* tls_emulation_test.c - Test program for GCC emulated TLS attribute propagation */

/* Force emulated TLS even on platforms with native support */
#pragma GCC target("tls")

/* Disable inlining to ensure TLS variable addresses are taken */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* Visibility attributes */
#define HIDDEN __attribute__((visibility("hidden")))
#define PROTECTED __attribute__((visibility("protected")))
#define INTERNAL __attribute__((visibility("internal")))

/* Linkage attributes */
#define WEAK __attribute__((weak))
#define DLLEXPORT __attribute__((dllexport))
#define DLLIMPORT __attribute__((dllimport))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Regular TLS with default visibility and external linkage */
__thread int tls_default = 42;
extern __thread int tls_extern;  /* DECL_EXTERNAL test */

/* 2. Weak TLS symbol */
__thread int tls_weak WEAK = 100;

/* 3. Hidden visibility TLS */
__thread int tls_hidden HIDDEN = 200;

/* 4. Protected visibility TLS */
__thread int tls_protected PROTECTED = 300;

/* 5. Common linkage (tentative definition) - tests DECL_COMMON */
__thread int tls_common;  /* No initializer for common */

/* 6. Static TLS within function context */
static void function_with_static_tls(void) {
    static __thread int tls_static_func = 500;
    volatile int* volatile ptr = &tls_static_func;
    *ptr += 1;  /* Force usage */
}

/* 7. DLL import simulation (even if not on Windows) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* Simulate with weak external */
    extern __thread int tls_dllimport WEAK;
#endif

/* 8. Public TLS used in multiple translation units */
__thread int tls_public TREE_PUBLIC = 999;

/* ========== NOINLINE FUNCTIONS FOR ADDRESS-TAKING ========== */

NOINLINE static void use_tls_default(int* out) {
    volatile int* volatile ptr = &tls_default;
    *out += *ptr;
    tls_default++;  /* Modify to ensure it's not optimized away */
}

NOINLINE static void use_tls_weak(int* out) {
    volatile int* volatile ptr = &tls_weak;
    *out += *ptr;
    tls_weak += 2;
}

NOINLINE static void use_tls_hidden(int* out) {
    volatile int* volatile ptr = &tls_hidden;
    *out += *ptr;
    tls_hidden += 3;
}

NOINLINE static void use_tls_protected(int* out) {
    volatile int* volatile ptr = &tls_protected;
    *out += *ptr;
    tls_protected += 4;
}

NOINLINE static void use_tls_common(int* out) {
    /* Force initialization of common TLS */
    tls_common = 600;
    volatile int* volatile ptr = &tls_common;
    *out += *ptr;
    tls_common += 5;
}

NOINLINE static void* get_tls_address(int selector) {
    /* Return address based on selector - forces multiple TLS vars to be considered */
    switch (selector) {
        case 0: return (void*)&tls_default;
        case 1: return (void*)&tls_weak;
        case 2: return (void*)&tls_hidden;
        case 3: return (void*)&tls_protected;
        case 4: return (void*)&tls_common;
        default: return (void*)&tls_public;
    }
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should trigger DECL_PRESERVE_P propagation */
    tls_default = 0xABCD;
    tls_hidden = 0x1234;
    
    /* Take address in constructor */
    volatile int* volatile ptr = &tls_protected;
    *ptr = 0x5678;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Access TLS in destructor */
    volatile int* volatile ptr = &tls_default;
    *ptr = 0;
}

/* ========== COMPLEX CONTROL FLOW WITH VOLATILE ========== */

NOINLINE static int compute_tls_checksum(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 5; i++) {
        selector = i % 6;
        
        /* Conditional TLS access based on volatile */
        if (selector == 0) {
            volatile int* volatile ptr = &tls_default;
            checksum += *ptr;
            *ptr += 1;
        } else if (selector == 1) {
            checksum += tls_weak;
            tls_weak += selector;
        } else if (selector == 2) {
            checksum += tls_hidden;
            tls_hidden += selector;
        } else if (selector == 3) {
            checksum += tls_protected;
            tls_protected += selector;
        } else if (selector == 4) {
            checksum += tls_common;
            tls_common += selector;
        } else {
            checksum += tls_public;
            tls_public += selector;
        }
    }
    
    return checksum;
}

/* ========== EXTERNAL DECLARATIONS (DECL_EXTERNAL TEST) ========== */

/* Forward declaration of TLS defined in another module (simulated) */
extern __thread int tls_external_module;
extern __thread int tls_external_weak WEAK;

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int checksum = 0;
    
    /* 1. Force usage of all TLS variables through address-taking functions */
    use_tls_default(&checksum);
    use_tls_weak(&checksum);
    use_tls_hidden(&checksum);
    use_tls_protected(&checksum);
    use_tls_common(&checksum);
    
    /* 2. Call function with static TLS */
    function_with_static_tls();
    
    /* 3. Complex control flow with volatile access */
    checksum += compute_tls_checksum();
    
    /* 4. Get addresses through switch (forces multiple TLS vars) */
    for (volatile int i = 0; i < 3; i++) {
        void* addr = get_tls_address(i);
        checksum += (int)(long)addr;  /* Use address in computation */
    }
    
    /* 5. Reference external TLS declarations */
    checksum += (int)(long)&tls_external_module;
    checksum += (int)(long)&tls_external_weak;
    
    /* 6. Reference DLL import style TLS */
    checksum += (int)(long)&tls_dllimport;
    
    /* Final value depends on all TLS accesses - prevents elimination */
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}

/* ========== C++ SPECIFIC TESTS (if compiled as C++) ========== */
#ifdef __cplusplus

namespace TLS_Namespace {
    /* TLS in namespace context */
    __thread int tls_in_namespace = 777;
    
    class TLS_Class {
    public:
        static __thread int tls_in_class;
        __thread int tls_instance_member;  /* Instance-specific TLS */
        
        NOINLINE void use_tls() {
            volatile int* volatile ptr1 = &tls_in_class;
            volatile int* volatile ptr2 = &tls_instance_member;
            *ptr1 += *ptr2;
        }
    };
    
    __thread int TLS_Class::tls_in_class = 888;
}

/* Additional C++ test function */
NOINLINE void test_cpp_tls(void) {
    TLS_Namespace::tls_in_namespace = 999;
    TLS_Namespace::TLS_Class obj;
    obj.tls_instance_member = 1111;
    obj.use_tls();
}

#endif /* __cplusplus */
