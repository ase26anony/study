/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")

/* Prevent inlining to ensure TLS addresses are taken */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) 
int tls_weak_hidden = 42;

/* 2. Public TLS with default visibility (explicit) */
__thread __attribute__((visibility("default"))) 
int tls_public_default = 100;

/* 3. Protected visibility TLS */
__thread __attribute__((visibility("protected"))) 
int tls_protected;

/* 4. Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* 5. External declaration (defined in another TU) */
extern __thread int tls_external;

/* 6. Static TLS inside function context (tests DECL_CONTEXT) */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 999;
    (void)tls_function_static;
}

/* 7. DLL import simulation (Windows-specific attributes) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
    __declspec(dllexport) __thread int tls_dllexport = 789;
#else
    /* Simulate with visibility attributes on ELF */
    __thread __attribute__((visibility("default"))) int tls_dllimport;
    __thread __attribute__((visibility("default"))) int tls_dllexport = 789;
#endif

/* 8. Weak external reference */
extern __thread __attribute__((weak)) int tls_weak_external;

/* 9. Used attribute (force TREE_USED) */
__thread int tls_used_force __attribute__((used)) = 333;

/* ===== HELPER FUNCTIONS TO TAKE ADDRESSES ===== */

NOINLINE static void use_tls_weak_hidden(volatile int **ptr) {
    *ptr = &tls_weak_hidden;
}

NOINLINE static void use_tls_public_default(volatile int **ptr) {
    *ptr = &tls_public_default;
}

NOINLINE static void use_tls_protected(volatile int **ptr) {
    *ptr = &tls_protected;
}

NOINLINE static void use_tls_common(volatile int **ptr) {
    *ptr = &tls_common;
}

NOINLINE static void use_tls_dllexport(volatile int **ptr) {
    *ptr = &tls_dllexport;
}

NOINLINE static void use_tls_used_force(volatile int **ptr) {
    *ptr = &tls_used_force;
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTION ===== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should trigger DECL_PRESERVE_P propagation */
    tls_protected = 0xABCD;
    tls_common = 0x1234;
    
    /* Take address to ensure variable is marked used */
    volatile int *unused = &tls_protected;
    (void)unused;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Access TLS in destructor */
    tls_public_default = 0;
}

/* ===== VOLATILE ACCESS PATTERNS ===== */

NOINLINE static int volatile_tls_access(volatile int selector) {
    volatile int result = 0;
    volatile int *tls_ptr = NULL;
    
    /* Force multiple control flow paths with volatile selector */
    for (volatile int i = 0; i < 5; i++) {
        switch (selector + i) {
            case 0:
                tls_ptr = &tls_weak_hidden;
                break;
            case 1:
                tls_ptr = &tls_public_default;
                break;
            case 2:
                tls_ptr = &tls_protected;
                break;
            case 3:
                tls_ptr = &tls_common;
                break;
            case 4:
                tls_ptr = &tls_dllexport;
                break;
            default:
                tls_ptr = &tls_used_force;
        }
        
        if (tls_ptr) {
            result += *tls_ptr;
            /* Volatile write to prevent optimization */
            *(volatile int *)tls_ptr = result;
        }
    }
    
    return result;
}

/* ===== MAIN TEST FUNCTION ===== */

int main(void) {
    volatile int checksum = 0;
    volatile int *tls_ptrs[6];
    
    /* 1. Call helper functions that take TLS addresses */
    use_tls_weak_hidden(&tls_ptrs[0]);
    use_tls_public_default(&tls_ptrs[1]);
    use_tls_protected(&tls_ptrs[2]);
    use_tls_common(&tls_ptrs[3]);
    use_tls_dllexport(&tls_ptrs[4]);
    use_tls_used_force(&tls_ptrs[5]);
    
    /* 2. Initialize some TLS variables */
    tls_weak_hidden = 1;
    tls_public_default = 2;
    tls_dllexport = 3;
    
    /* 3. Volatile access pattern */
    checksum += volatile_tls_access(0);
    checksum += volatile_tls_access(2);
    
    /* 4. Compute checksum through pointers */
    for (int i = 0; i < 6; i++) {
        if (tls_ptrs[i]) {
            checksum += *tls_ptrs[i];
            /* Modify through volatile pointer */
            *(volatile int *)tls_ptrs[i] = checksum & 0xFF;
        }
    }
    
    /* 5. Call function with static TLS */
    function_with_static_tls();
    
    /* 6. Use all TLS variables in expressions (force TREE_USED) */
    if (tls_weak_hidden || tls_public_default || tls_protected ||
        tls_common || tls_dllexport || tls_used_force) {
        checksum++;
    }
    
    /* Final value depends on TLS emulation behavior */
    return checksum & 0xFF;
}

/* ===== C++ VERSION WITH NAMESPACES ===== */
#ifdef __cplusplus
namespace tls_test {
    thread_local int tls_namespace = 555;
    thread_local __attribute__((visibility("hidden"))) int tls_namespace_hidden = 666;
    
    class TLSUser {
    public:
        NOINLINE int use_tls() {
            /* Take address of namespace TLS */
            volatile int* p1 = &tls_namespace;
            volatile int* p2 = &tls_namespace_hidden;
            return *p1 + *p2;
        }
    };
}

/* C++ main that uses namespace TLS */
extern "C" int test_cpp_tls(void) {
    tls_test::TLSUser user;
    return user.use_tls();
}
#endif
