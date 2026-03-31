/* tls_emutls_test.c - Test program for GCC emulated TLS attribute propagation */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emutls_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with default visibility */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* 2. Hidden visibility TLS variable */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 100;

/* 3. Protected visibility TLS variable */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - forces DECL_COMMON */
__thread int tls_common_var;  /* No initializer = common linkage */

/* 5. External declaration - forces DECL_EXTERNAL */
extern __thread int tls_external_var;

/* 6. Static TLS inside function context - tests DECL_CONTEXT */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 999;
    (void)tls_function_static;
}

/* 7. DLL import simulation (using visibility attributes) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* Simulate DLL import with visibility */
    __thread int tls_dllimport_var __attribute__((visibility("default")));
#endif

/* 8. Public TLS variable with explicit visibility */
__thread int tls_public_var __attribute__((visibility("default"))) = 300;

/* 9. Used attribute - ensure TREE_USED is set */
__thread volatile int tls_used_var = 500;

/* ===== HELPER FUNCTIONS TO FORCE TLS USAGE ===== */

NOINLINE static void use_tls_weak(int *out) {
    volatile int *volatile ptr = &tls_weak_var;
    *out += *ptr;
    tls_weak_var++;  /* Modify to ensure it's not optimized away */
}

NOINLINE static void use_tls_hidden(int *out) {
    volatile int *volatile ptr = &tls_hidden_var;
    *out += *ptr;
    tls_hidden_var ^= 0x55AA55AA;  /* Non-trivial operation */
}

NOINLINE static void use_tls_protected(int *out) {
    int val = tls_protected_var;
    *out += val;
    tls_protected_var = val * 2;
}

NOINLINE static void use_tls_common(int *out) {
    /* Force address-taking of common TLS variable */
    int *ptr = &tls_common_var;
    *out += *ptr;
    *ptr += 1;  /* Modify the common variable */
}

NOINLINE static void use_tls_external(int *out) {
    /* External TLS variable - compiler must generate emutls struct */
    extern __thread int tls_external_var;
    volatile int *volatile ptr = &tls_external_var;
    *out += *ptr;
}

NOINLINE static void use_tls_public(int *out) {
    volatile int *volatile ptr = &tls_public_var;
    *out += *ptr;
    tls_public_var = (*ptr) + 1;
}

NOINLINE static void use_tls_used(int *out) {
    /* Direct volatile access ensures TREE_USED is set */
    *out += tls_used_var;
    tls_used_var++;
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ===== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should force DECL_PRESERVE_P to be true for accessed TLS vars */
    tls_hidden_var = 0xDEADBEEF;
    tls_public_var = 0xCAFEBABE;
    
    /* Take address in constructor to ensure preservation */
    volatile int *volatile ptr = &tls_weak_var;
    *ptr = 1234;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Access TLS in destructor */
    tls_public_var = 0;
}

/* ===== COMPLEX CONTROL FLOW WITH VOLATILE SELECTOR ===== */

NOINLINE static int access_tls_based_on_selector(volatile int selector) {
    int result = 0;
    
    /* Volatile loop counter prevents optimization */
    for (volatile int i = 0; i < 3; i++) {
        switch (selector + i) {
            case 0:
                result += tls_weak_var;
                break;
            case 1:
                result += tls_hidden_var;
                break;
            case 2:
                result += tls_protected_var;
                break;
            case 3:
                result += tls_common_var;
                break;
            case 4:
                result += tls_public_var;
                break;
            case 5:
                result += tls_used_var;
                break;
            default:
                result += selector;
        }
    }
    
    return result;
}

/* ===== MAIN FUNCTION WITH COMPREHENSIVE TLS USAGE ===== */

int main(void) {
    int checksum = 0;
    volatile int selector = 0;
    
    /* 1. Initialize some TLS variables */
    tls_common_var = 777;
    
    /* 2. Call helper functions that take addresses of TLS variables */
    use_tls_weak(&checksum);
    use_tls_hidden(&checksum);
    use_tls_protected(&checksum);
    use_tls_common(&checksum);
    use_tls_public(&checksum);
    use_tls_used(&checksum);
    
    /* 3. Try to use external TLS variable */
    use_tls_external(&checksum);
    
    /* 4. Complex control flow with volatile selector */
    for (selector = 0; selector < 10; selector++) {
        checksum += access_tls_based_on_selector(selector);
    }
    
    /* 5. Function with static TLS to test DECL_CONTEXT */
    function_with_static_tls();
    
    /* 6. Compute final checksum using all TLS variables */
    checksum += tls_weak_var;
    checksum += tls_hidden_var;
    checksum += tls_protected_var;
    checksum += tls_common_var;
    checksum += tls_public_var;
    checksum += tls_used_var;
    
    /* 7. Take addresses in main to ensure they're marked used */
    volatile int *volatile addrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_public_var,
        &tls_used_var
    };
    
    for (int i = 0; i < (int)(sizeof(addrs)/sizeof(addrs[0])); i++) {
        checksum += *addrs[i];
    }
    
    printf("TLS checksum: %d\n", checksum);
    printf("TLS values: weak=%d, hidden=%d, protected=%d, common=%d, public=%d, used=%d\n",
           tls_weak_var, tls_hidden_var, tls_protected_var,
           tls_common_var, tls_public_var, tls_used_var);
    
    return checksum != 0 ? 0 : 1;
}

/* ===== EXTERNAL TLS VARIABLE DEFINITION ===== */
/* This must be in a separate compilation unit normally, but for testing
   we define it here with different visibility to test attribute merging */
__thread int tls_external_var __attribute__((visibility("protected"))) = 888;
