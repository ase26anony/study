/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int v);
extern int get_random(void);
extern void external_func(void);

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
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with attribute if supported */
__thread int imported_tls __attribute__((dllimport));
#endif

/* Static TLS (not public) with complex initializer */
static __thread int static_tls = 123;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 456;

/* TLS whose address escapes (forces DECL_PRESERVE_P) */
__thread int preserved_tls = 789;

/* Common TLS (tentative definition) */
__thread int common_tls;  /* No initializer */

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 999;

/* TLS with runtime initialization */
__thread int dynamic_tls;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_tls_with_attr __attribute__((visibility("default")));

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function that accesses TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch(idx % 4) {
        case 0: return public_tls + static_tls;
        case 1: return hidden_tls - aligned_tls;
        case 2: return preserved_tls * 2;
        case 3: return volatile_tls / 2;
        default: return 0;
    }
}

/* Function that takes address of TLS - forces address computation */
static void take_tls_addresses(void) {
    void *addresses[] = {
        (void*)&public_tls,
        (void*)&weak_tls,
        (void*)&hidden_tls,
        (void*)&protected_tls,
        (void*)&internal_tls,
        (void*)&static_tls,
        (void*)&aligned_tls,
        (void*)&preserved_tls,
        (void*)&common_tls,
        (void*)&volatile_tls,
        (void*)&dynamic_tls
    };
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        use_ptr(addresses[i]);
    }
}

/* Function with complex TLS usage pattern */
static int compute_with_tls(int seed) {
    int result = 0;
    
    /* Mix TLS and non-TLS computations */
    for (int i = 0; i < 10; i++) {
        /* Use inline function that accesses TLS */
        result += inline_tls_access(seed + i);
        
        /* Direct TLS accesses with varying attributes */
        if (i % 2 == 0) {
            result += public_tls;
            public_tls += seed;  /* Modify TLS */
        } else {
            result += hidden_tls;
            hidden_tls -= seed;  /* Modify TLS */
        }
        
        /* Access volatile TLS */
        result += volatile_tls;
        
        /* Take address of TLS in loop */
        use_ptr(&preserved_tls);
    }
    
    return result;
}

/* Function that forces TLS emulation through address escape */
static void escape_tls_address(void) {
    /* Store TLS addresses in non-TLS locations */
    static void *escaped_addresses[5];
    
    escaped_addresses[0] = &public_tls;
    escaped_addresses[1] = &weak_tls;
    escaped_addresses[2] = &hidden_tls;
    escaped_addresses[3] = &preserved_tls;
    escaped_addresses[4] = &volatile_tls;
    
    /* Use assembly to ensure address escapes */
    __asm__ volatile ("" : : "r"(&escaped_addresses) : "memory");
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed += *p;
        }
    }
    
    /* Initialize dynamic TLS with runtime value */
    dynamic_tls = seed * 2;
    
    /* Initialize common TLS */
    common_tls = seed + 1000;
    
    /* Force TLS address taking early */
    take_tls_addresses();
    
    /* Complex computation with TLS */
    int checksum = compute_with_tls(seed);
    
    /* Additional TLS manipulations */
    for (int i = 0; i < 5; i++) {
        /* Conditional TLS access based on runtime value */
        if ((seed + i) % 3 == 0) {
            protected_tls += inline_tls_access(i);
            use_int(protected_tls);
        } else if ((seed + i) % 3 == 1) {
            internal_tls -= inline_tls_access(i);
            use_int(internal_tls);
        } else {
            aligned_tls *= (i + 1);
            use_int(aligned_tls);
        }
        
        /* Access external TLS (simulating cross-module access) */
        if (external_tls != 0) {
            checksum += external_tls;
        }
    }
    
    /* Force address escape */
    escape_tls_address();
    
    /* Final checksum computation using all TLS variables */
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += static_tls;
    checksum += aligned_tls;
    checksum += preserved_tls;
    checksum += common_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    
    /* Simulate use of imported TLS */
    checksum += imported_tls;
    
    /* Print result to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* ================= EXTERNAL TLS DEFINITIONS ================= */

/* Define the external TLS variables (simulating another source file) */
__thread int external_tls = 5555;
__thread int external_tls_with_attr __attribute__((visibility("default"))) = 6666;

/* ================= DUMMY FUNCTION DEFINITIONS ================= */

/* These would normally be in a separate library, but defined here for completeness */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr;
    last_ptr = p;
}

void use_int(int v) {
    /* Prevent optimization */
    static volatile int last_val;
    last_val = v;
}

int get_random(void) {
    return 42; /* Deterministic for testing */
}
