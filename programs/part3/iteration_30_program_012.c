/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and force
 * the compiler to copy declaration attributes between TLS variables,
 * specifically targeting the uncovered lines in tree-emutls.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int i);
extern int get_random_value(void);

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage and default visibility */
__thread int public_tls = 42;
__thread int public_tls_uninit;

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 300;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Use dllimport-like attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Static TLS (not public) with complex initializer */
static __thread int static_tls = 0;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* TLS with address likely to escape */
__thread int escaping_tls = 700;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* TLS used in asm (forces DECL_PRESERVE_P) */
register __thread int asm_tls asm("r12") __asm__("asm_tls") = 800;

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 0;
    inline_tls += idx;
    return inline_tls;
}

/* Function that takes address of TLS and passes it to opaque function */
static void take_tls_addresses(void) {
    void *addresses[] = {
        (void*)&public_tls,
        (void*)&weak_tls,
        (void*)&hidden_tls,
        (void*)&protected_tls,
        (void*)&internal_tls,
        (void*)&dllimport_tls,
        (void*)&static_tls,
        (void*)&aligned_tls,
        (void*)&volatile_tls,
        (void*)&escaping_tls,
        (void*)&external_tls,
        (void*)&external_weak_tls,
        (void*)&asm_tls
    };
    
    for (int i = 0; i < (int)(sizeof(addresses)/sizeof(addresses[0])); i++) {
        use_ptr(addresses[i]);
    }
}

/* Complex TLS usage pattern that might trigger declaration cloning */
static int complex_tls_operations(int seed) {
    int result = 0;
    
    /* Dynamic initialization simulation */
    static __thread int dynamic_tls = 0;
    if (dynamic_tls == 0) {
        dynamic_tls = get_random_value() + seed;
    }
    
    /* Mix TLS and non-TLS in loops */
    for (int i = 0; i < seed % 10 + 1; i++) {
        public_tls += i;
        static_tls -= i;
        result += public_tls * static_tls;
        
        /* Use inline function with TLS */
        result += inline_tls_access(i);
    }
    
    /* Conditional TLS access */
    volatile int *volatile_ptr = (volatile int*)&volatile_tls;
    if (seed % 2) {
        *volatile_ptr += hidden_tls;
    } else {
        *volatile_ptr -= protected_tls;
    }
    
    /* Array indexing with TLS addresses */
    __thread int *tls_ptrs[] = {&public_tls, &static_tls, &escaping_tls};
    int idx = seed % 3;
    result += *tls_ptrs[idx];
    
    return result;
}

/* Function that forces TLS address escaping */
static void escape_tls_addresses(void) {
    /* Store TLS addresses in global-like locations */
    static void *escaped_addresses[5];
    
    escaped_addresses[0] = (void*)&public_tls;
    escaped_addresses[1] = (void*)&weak_tls;
    escaped_addresses[2] = (void*)&hidden_tls;
    escaped_addresses[3] = (void*)&escaping_tls;
    escaped_addresses[4] = (void*)&external_tls;
    
    /* Pass array to opaque function */
    use_ptr(escaped_addresses);
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    srand(seed);
    
    /* Initialize some TLS variables with runtime values */
    public_tls_uninit = seed;
    static_tls = seed * 2;
    aligned_tls = seed * 3;
    
    /* Force TLS address taking */
    take_tls_addresses();
    
    /* Complex operations that might trigger declaration copying */
    int checksum = complex_tls_operations(seed);
    
    /* More address escaping */
    escape_tls_addresses();
    
    /* Use TLS in switch statement */
    switch (seed % 4) {
        case 0:
            internal_tls += public_tls;
            break;
        case 1:
            protected_tls += hidden_tls;
            break;
        case 2:
            weak_tls += static_tls;
            break;
        case 3:
            escaping_tls += aligned_tls;
            break;
    }
    
    /* Compute final checksum covering all TLS variables */
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += static_tls;
    checksum += aligned_tls;
    checksum += volatile_tls;
    checksum += escaping_tls;
    checksum += asm_tls;
    checksum += inline_tls_access(0);  /* Access inline TLS */
    
    /* Use the checksum to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    /* Final use of all TLS addresses */
    void *final_addresses[] = {
        &public_tls, &weak_tls, &hidden_tls, &protected_tls,
        &internal_tls, &dllimport_tls, &static_tls, &aligned_tls,
        &volatile_tls, &escaping_tls, &external_tls, &external_weak_tls,
        &asm_tls
    };
    
    for (size_t i = 0; i < sizeof(final_addresses)/sizeof(final_addresses[0]); i++) {
        use_ptr(final_addresses[i]);
    }
    
    return checksum % 256;
}

/* ================= EXTERNAL TLS DEFINITIONS ================= */

/* Define the external TLS variables (simulating another source file) */
__thread int external_tls = 999;
__thread int external_weak_tls __attribute__((weak)) = 888;

/* ================= OPAQUE FUNCTION STUBS ================= */

/* These would be empty in a real test environment */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_int(int i) {
    /* Prevent optimization */
    static volatile int last_int = 0;
    last_int = i;
}

int get_random_value(void) {
    return rand();
}
