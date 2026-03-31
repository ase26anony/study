/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")
#ifdef __clang__
#error "This test requires GCC for emulated TLS attribute testing"
#endif

/* Prevent inlining to ensure TLS variables are marked as used */
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

/* 4. Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 5. Static TLS within file scope (internal linkage) */
static __thread int tls_static_internal = 300;

/* 6. DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dll_imported;
#else
/* Simulate with weak external */
extern __thread int tls_dll_imported __attribute__((weak));
#endif

/* 7. External declaration (should be defined elsewhere) */
extern __thread int tls_external;

/* 8. TLS with both weak and common attributes */
__thread int tls_weak_common __attribute__((weak, common));

/* ========== FUNCTION DECLARATIONS ========== */
NOINLINE void use_tls_address(int *addr);
NOINLINE void modify_tls_volatile(volatile int *addr);
NOINLINE int compute_tls_checksum(void);
NOINLINE void access_all_tls_variables(void);
static void block_scope_tls_test(void);

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

/* Constructor that uses TLS - tests DECL_PRESERVE_P propagation */
CONSTRUCTOR static void tls_constructor(void) {
    volatile static int constructor_called = 0;
    if (!constructor_called) {
        constructor_called = 1;
        /* Access and modify TLS variables */
        tls_public_default = 999;
        tls_static_internal = 888;
        
        /* Force address-taking in constructor */
        int *addr = &tls_public_default;
        *addr += 1;
    }
}

/* Destructor that verifies TLS is still accessible */
DESTRUCTOR static void tls_destructor(void) {
    /* Just access to ensure TLS is preserved */
    volatile int dummy = tls_public_default + tls_static_internal;
    (void)dummy;
}

/* ========== HELPER FUNCTIONS ========== */

NOINLINE void use_tls_address(int *addr) {
    /* Force TREE_USED marking */
    static volatile int sink;
    sink = *addr;
    *addr += 1;
}

NOINLINE void modify_tls_volatile(volatile int *addr) {
    /* Volatile access prevents optimization */
    for (volatile int i = 0; i < 3; i++) {
        *addr += i;
    }
}

NOINLINE int compute_tls_checksum(void) {
    /* Compute a simple checksum of all TLS variables */
    int sum = 0;
    
    /* Access each TLS variable, forcing address computation */
    sum += tls_weak_hidden;
    sum += tls_public_default;
    sum += tls_common;
    sum += tls_protected;
    sum += tls_static_internal;
    
    /* External/weak variables might be zero */
    sum += tls_dll_imported;
    sum += tls_external;
    sum += tls_weak_common;
    
    return sum;
}

NOINLINE void access_all_tls_variables(void) {
    /* Take addresses of all TLS variables */
    int *addrs[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_common,
        &tls_protected,
        &tls_static_internal,
        &tls_dll_imported,
        &tls_external,
        &tls_weak_common,
        NULL
    };
    
    for (int **p = addrs; *p; p++) {
        use_tls_address(*p);
    }
}

static void block_scope_tls_test(void) {
    /* TLS in block scope - tests DECL_CONTEXT */
    static __thread int tls_block_static = 50;
    __thread int tls_block_auto;
    
    tls_block_auto = tls_block_static + 10;
    modify_tls_volatile(&tls_block_static);
    modify_tls_volatile(&tls_block_auto);
    
    /* Mix with global TLS */
    tls_block_static += tls_public_default;
}

/* ========== C++ SPECIFIC TESTS (if compiled as C++) ========== */
#ifdef __cplusplus
namespace tls_test {
    /* TLS in namespace */
    __thread int tls_namespace __attribute__((visibility("hidden"))) = 400;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls() {
            /* Access namespace TLS from member function */
            tls_namespace++;
            modify_tls_volatile(&tls_namespace);
        }
        
        /* Instance-specific TLS usage pattern */
        NOINLINE int compute() {
            static __thread int tls_member = 0;
            tls_member++;
            return tls_member + tls_namespace;
        }
    };
}
#endif

/* ========== MAIN FUNCTION ========== */

int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* Initialize some TLS variables */
    tls_common = 500;
    tls_weak_common = 600;
    
    /* Call constructor-like behavior explicitly too */
    tls_constructor();
    
    /* Test block scope TLS */
    block_scope_tls_test();
    
    /* Conditional access based on volatile selector */
    for (volatile int i = 0; i < 10; i++) {
        selector = i % 8;
        
        switch (selector) {
            case 0: modify_tls_volatile(&tls_weak_hidden); break;
            case 1: modify_tls_volatile(&tls_public_default); break;
            case 2: modify_tls_volatile(&tls_common); break;
            case 3: modify_tls_volatile(&tls_protected); break;
            case 4: modify_tls_volatile(&tls_static_internal); break;
            case 5: modify_tls_volatile(&tls_dll_imported); break;
            case 6: modify_tls_volatile(&tls_external); break;
            case 7: modify_tls_volatile(&tls_weak_common); break;
        }
        
        /* Occasionally access all variables */
        if (i % 3 == 0) {
            access_all_tls_variables();
        }
    }
    
#ifdef __cplusplus
    /* C++ specific tests */
    tls_test::TLSUser user;
    user.use_namespace_tls();
    checksum += user.compute();
#endif
    
    /* Final checksum computation */
    checksum += compute_tls_checksum();
    
    /* Use checksum to prevent dead code elimination */
    volatile int result = checksum;
    
    /* Print minimal output for verification */
    if (result != 0) {
        /* Force use of all TLS variable addresses one more time */
        access_all_tls_variables();
    }
    
    return 0;
}

/* ========== EXTERNAL TLS DEFINITION ========== */
/* Define the external TLS variable */
__thread int tls_external = 700;

/* Weak TLS definition */
__thread int tls_dll_imported __attribute__((weak)) = 800;
