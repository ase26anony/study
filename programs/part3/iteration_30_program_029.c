/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern int get_random_value(void);
extern void escape_pointer(void **p);

/* Global seed for unpredictable control flow */
static volatile int global_seed = 0;

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* Public TLS with external linkage and default visibility */
__thread int tls_public_default = 42;
extern __thread int tls_external_default;  /* Will be defined in another TU */

/* Weak TLS symbol with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 100;

/* Static TLS (non-public) with internal linkage */
static __thread int tls_static_internal = 255;

/* TLS with preserved attribute (address escapes) */
__thread volatile int tls_preserved __attribute__((used)) = 777;

/* Common TLS (tentative definition) */
__thread int tls_common;

/* DLL import simulation (Windows-style) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute if supported */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* TLS with protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 999;

/* TLS requiring dynamic initialization */
__thread int tls_dynamic_init = 0;

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64))) = 1234;

/* ========== FUNCTION USING TLS IN COMPLEX WAYS ========== */

/* Inline function that accesses TLS - may trigger declaration copying during inlining */
static inline int process_tls_inline(int idx) {
    /* Mix different TLS variables based on index */
    switch (idx % 4) {
        case 0: return tls_public_default++;
        case 1: return tls_weak_hidden * 2;
        case 2: return tls_static_internal ^ 0xFF;
        case 3: return tls_preserved / 2;
        default: return tls_common;
    }
}

/* Function that takes address of TLS variables */
static void capture_tls_addresses(void **ptrs, int count) {
    /* Store addresses in array to force address-taking */
    if (count > 0) ptrs[0] = (void *)&tls_public_default;
    if (count > 1) ptrs[1] = (void *)&tls_weak_hidden;
    if (count > 2) ptrs[2] = (void *)&tls_static_internal;
    if (count > 3) ptrs[3] = (void *)&tls_preserved;
    if (count > 4) ptrs[4] = (void *)&tls_common;
    if (count > 5) ptrs[5] = (void *)&tls_protected;
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < count && i < 6; i++) {
        use_ptr(ptrs[i]);
    }
}

/* Function with loop that uses TLS variables */
static int compute_tls_checksum(int iterations) {
    int sum = 0;
    volatile int * volatile ptr;  /* Volatile pointer to volatile TLS */
    
    for (int i = 0; i < iterations; i++) {
        /* Access TLS in ways that might trigger emulation */
        sum += tls_public_default;
        sum -= tls_weak_hidden;
        sum ^= tls_static_internal;
        
        /* Use inline function */
        sum += process_tls_inline(i);
        
        /* Volatile access to prevent optimization */
        ptr = &tls_preserved;
        sum += *ptr;
        
        /* Modify TLS based on loop index */
        if (i % 3 == 0) {
            tls_common = i;
        }
        
        /* Complex expression with TLS address */
        sum += (int)((long)&tls_protected % 256);
    }
    
    return sum;
}

/* Function that forces dynamic TLS initialization */
static void init_dynamic_tls(void) {
    /* Use runtime value for initialization */
    tls_dynamic_init = get_random_value();
    
    /* Initialize aligned TLS with computed value */
    tls_aligned = tls_dynamic_init * 2;
}

/* ========== MAIN FUNCTION WITH UNPREDICTABLE FLOW ========== */

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    global_seed = seed;
    
    /* Array to store TLS addresses */
    void *tls_pointers[10];
    
    /* Initialize dynamic TLS if seed is odd */
    if (seed & 1) {
        init_dynamic_tls();
    }
    
    /* Capture addresses of TLS variables */
    capture_tls_addresses(tls_pointers, 6);
    
    /* Escape pointers to prevent optimization */
    escape_pointer(tls_pointers);
    escape_pointer(&tls_pointers[3]);
    
    /* Modify TLS variables based on seed */
    tls_public_default += seed;
    tls_weak_hidden -= seed % 100;
    tls_static_internal ^= seed;
    tls_preserved = seed * 3;
    tls_common = seed % 256;
    tls_protected = seed * seed;
    
    /* Complex access pattern */
    int iterations = 10 + (seed % 20);
    int checksum = compute_tls_checksum(iterations);
    
    /* Additional unpredictable TLS access */
    volatile int *access_ptr;
    switch (seed % 5) {
        case 0: access_ptr = &tls_public_default; break;
        case 1: access_ptr = &tls_weak_hidden; break;
        case 2: access_ptr = &tls_static_internal; break;
        case 3: access_ptr = &tls_preserved; break;
        case 4: access_ptr = &tls_common; break;
        default: access_ptr = &tls_protected;
    }
    
    /* Force multiple volatile accesses */
    for (int i = 0; i < 3; i++) {
        checksum += *access_ptr;
        *access_ptr += i;
    }
    
    /* Final checksum computation using all TLS variables */
    checksum += tls_public_default;
    checksum += tls_weak_hidden;
    checksum += tls_static_internal;
    checksum += tls_preserved;
    checksum += tls_common;
    checksum += tls_protected;
    checksum += tls_dynamic_init;
    checksum += tls_aligned;
    
    /* Print result to prevent dead code elimination */
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    return checksum != 0 ? 0 : 1;
}

/* ========== SECOND "COMPILATION UNIT" SIMULATION ========== */
/* In a real multi-file test, this would be in a separate source file */

/* External TLS definition (simulating another TU) */
__thread int tls_external_default = 888;

/* Weak TLS definition */
__thread int tls_weak_hidden = 100;  /* Matches the weak declaration */

/* DLL import stub (would be in actual DLL) */
#ifdef _WIN32
__declspec(dllexport) __thread int tls_dllimport = 5555;
#endif

/* Opaque function stubs for linking */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    (void)p;
}

int get_random_value(void) {
    return 42;  /* Deterministic for testing */
}

void escape_pointer(void **p) {
    /* Prevent optimization */
    static void *sink;
    sink = *p;
    (void)sink;
}
