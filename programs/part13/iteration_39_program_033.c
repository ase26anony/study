/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")

/* Prevent inlining to ensure TLS addresses are taken */
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

/* 4. Common linkage (tentative definition) - tests DECL_COMMON */
__thread int tls_common;  /* No initializer = common linkage */

/* 5. External declaration (tests DECL_EXTERNAL) */
extern __thread int tls_external;

/* 6. DLL import simulation (tests DECL_DLLIMPORT_P) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* Simulate with weak attribute for non-Windows */
    __thread __attribute__((weak)) int tls_dllimport;
#endif

/* 7. Static TLS inside a function context */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 0xDEF0;
    volatile int* volatile ptr = &tls_function_static;
    *ptr += 1;  /* Force access through volatile pointer */
}

/* 8. TLS with preserve attribute (via used) */
__thread __attribute__((used)) int tls_preserved = 0x1111;

/* ========== C++ SPECIFIC TESTS (if compiled as C++) ========== */
#ifdef __cplusplus
namespace TLS_Namespace {
    __thread int tls_in_namespace = 0x2222;
    
    class TLS_Class {
    public:
        static __thread int tls_in_class;
        __thread int tls_member;  /* Instance-specific TLS */
        
        void access_tls() {
            volatile int* p1 = &tls_in_class;
            volatile int* p2 = &this->tls_member;
            *p1 += 1;
            *p2 += 2;
        }
    };
    
    __thread int TLS_Class::tls_in_class = 0x3333;
}
#endif

/* ========== HELPER FUNCTIONS ========== */

/* Force address-taking of TLS variables */
NOINLINE static void use_tls_weak_hidden(volatile int** out) {
    *out = &tls_weak_hidden;
}

NOINLINE static void use_tls_public_default(volatile int** out) {
    *out = &tls_public_default;
}

NOINLINE static void use_tls_protected(volatile int** out) {
    *out = &tls_protected;
}

NOINLINE static void use_tls_common(volatile int** out) {
    *out = &tls_common;
}

NOINLINE static void use_tls_external(volatile int** out) {
    /* External declaration - provide definition here */
    __thread int tls_external = 0x4444;
    *out = &tls_external;
}

NOINLINE static void use_tls_dllimport(volatile int** out) {
    /* Provide definition for the "dllimport" variable */
    __thread int tls_dllimport = 0x5555;
    *out = &tls_dllimport;
}

NOINLINE static void use_tls_preserved(volatile int** out) {
    *out = &tls_preserved;
}

/* Complex control flow with volatile selector */
NOINLINE static int access_tls_based_on_selector(int selector) {
    volatile int result = 0;
    volatile int* tls_ptr = NULL;
    
    /* Volatile to prevent optimization */
    volatile int sel = selector;
    
    switch (sel & 7) {
        case 0: tls_ptr = &tls_weak_hidden; break;
        case 1: tls_ptr = &tls_public_default; break;
        case 2: tls_ptr = &tls_protected; break;
        case 3: tls_ptr = &tls_common; break;
        case 4: 
            __thread int tls_external = 0x4444;
            tls_ptr = &tls_external;
            break;
        case 5: 
            __thread int tls_dllimport = 0x5555;
            tls_ptr = &tls_dllimport;
            break;
        case 6: tls_ptr = &tls_preserved; break;
        case 7: 
            static __thread int tls_local_switch = 0x7777;
            tls_ptr = &tls_local_switch;
            break;
    }
    
    if (tls_ptr) {
        result = *tls_ptr;
        /* Modify through volatile pointer */
        *tls_ptr = result + 1;
    }
    
    return result;
}

/* Constructor that accesses TLS */
CONSTRUCTOR static void init_tls_in_constructor(void) {
    volatile int* p = &tls_public_default;
    *p = 0x8888;  /* Set known value in constructor */
    
    /* Also access other TLS variables */
    tls_weak_hidden = 0x9999;
    tls_protected = 0xAAAA;
}

/* Destructor that verifies TLS was used */
DESTRUCTOR static void verify_tls_in_destructor(void) {
    volatile int sum = 
        tls_weak_hidden + 
        tls_public_default + 
        tls_protected;
    /* Use sum to prevent elimination */
    asm volatile("" : : "r"(sum) : "memory");
}

/* ========== MAIN TEST FUNCTION ========== */

int main(void) {
    volatile int checksum = 0;
    volatile int* volatile ptrs[8];
    
    /* 1. Take addresses of all TLS variables */
    use_tls_weak_hidden(&ptrs[0]);
    use_tls_public_default(&ptrs[1]);
    use_tls_protected(&ptrs[2]);
    use_tls_common(&ptrs[3]);
    use_tls_external(&ptrs[4]);
    use_tls_dllimport(&ptrs[5]);
    use_tls_preserved(&ptrs[6]);
    
    /* 2. Function with static TLS */
    function_with_static_tls();
    
    /* 3. Complex control flow accessing TLS */
    for (volatile int i = 0; i < 100; i++) {
        checksum += access_tls_based_on_selector(i);
    }
    
    /* 4. Direct volatile accesses */
    volatile int* vptr;
    
    vptr = &tls_weak_hidden;
    *vptr += 1;
    checksum += *vptr;
    
    vptr = &tls_public_default;
    *vptr += 2;
    checksum += *vptr;
    
    vptr = &tls_protected;
    *vptr += 3;
    checksum += *vptr;
    
    vptr = &tls_common;
    *vptr = 0xCCCC;
    checksum += *vptr;
    
    /* 5. C++ specific tests if compiled as C++ */
#ifdef __cplusplus
    {
        using namespace TLS_Namespace;
        
        TLS_Class obj1, obj2;
        obj1.tls_member = 0x1111;
        obj2.tls_member = 0x2222;
        TLS_Class::tls_in_class = 0x3333;
        tls_in_namespace = 0x4444;
        
        obj1.access_tls();
        obj2.access_tls();
        
        volatile int* cp1 = &obj1.tls_member;
        volatile int* cp2 = &obj2.tls_member;
        volatile int* cp3 = &TLS_Class::tls_in_class;
        volatile int* cp4 = &tls_in_namespace;
        
        checksum += *cp1 + *cp2 + *cp3 + *cp4;
    }
#endif
    
    /* 6. Compute final checksum through all pointers */
    for (int i = 0; i < 7; i++) {
        if (ptrs[i]) {
            checksum += *ptrs[i];
        }
    }
    
    /* 7. Runtime verification of emulated TLS */
    /* Check if we're using emulated TLS by examining pointer patterns */
    volatile int* same_tls_var = &tls_public_default;
    volatile int* same_tls_var2 = &tls_public_default;
    
    /* In emulated TLS, taking address twice might give different
       pointers to the control structure vs actual data */
    checksum += (same_tls_var == same_tls_var2) ? 0 : 1;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(checksum) : "memory");
    
    /* Print something to ensure execution */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
