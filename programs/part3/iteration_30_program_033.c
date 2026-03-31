/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random_value(void);
extern void external_function(void);

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

/* Common TLS (tentative definition) */
__thread int common_tls;

/* Static TLS (internal linkage) */
static __thread int static_tls = 500;

/* Volatile TLS to prevent optimization */
__thread volatile int volatile_tls = 600;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* TLS that might need preservation (address escapes) */
__thread int preserved_tls = 800;

/* ================= EXTERNAL TLS DECLARATIONS ================= */
/* Simulating TLS variables defined in another compilation unit */
extern __thread int external_public_tls;
extern __thread int external_weak_tls __attribute__((weak));
extern __thread int external_hidden_tls __attribute__((visibility("hidden")));

/* ================= FUNCTION WITH INLINE ACCESS TO TLS ================= */
/* This may trigger declaration copying during inlining */
static inline int inline_tls_access(int multiplier) {
    /* Access multiple TLS variables */
    int result = static_tls + public_tls;
    result *= multiplier;
    
    /* Take address of TLS variable inside inline function */
    int *ptr = &static_tls;
    use_ptr(ptr);
    
    return result;
}

/* Function that takes address of TLS - forces emulation structures */
static void take_tls_addresses(void) {
    void *addresses[20];
    volatile int idx = 0;
    
    addresses[idx++] = &public_tls;
    addresses[idx++] = &weak_tls;
    addresses[idx++] = &hidden_tls;
    addresses[idx++] = &protected_tls;
    addresses[idx++] = &internal_tls;
    addresses[idx++] = &imported_tls;
    addresses[idx++] = &common_tls;
    addresses[idx++] = &static_tls;
    addresses[idx++] = &volatile_tls;
    addresses[idx++] = &aligned_tls;
    addresses[idx++] = &preserved_tls;
    
    /* Pass addresses to opaque function */
    for (int i = 0; i < idx; i++) {
        use_ptr(addresses[i]);
    }
}

/* Function with complex TLS usage pattern */
static int compute_with_tls(int seed) {
    int result = 0;
    
    /* Dynamic initialization simulation */
    static __thread int dynamic_tls = 0;
    if (dynamic_tls == 0) {
        dynamic_tls = seed * 2;
    }
    
    /* Mix TLS and non-TLS values */
    result += public_tls * seed;
    result += weak_tls / (seed + 1);
    result += hidden_tls - seed;
    
    /* Use TLS in loop */
    for (int i = 0; i < (seed % 5); i++) {
        result += static_tls + i;
        volatile_tls = result;  /* Volatile write */
    }
    
    /* Conditional TLS access */
    if (seed % 2) {
        result += protected_tls;
        use_ptr(&protected_tls);
    } else {
        result += internal_tls;
        use_ptr(&internal_tls);
    }
    
    /* Call inline function with TLS access */
    result += inline_tls_access(seed % 3 + 1);
    
    /* Access external TLS */
    result += external_public_tls;
    result += external_weak_tls;
    result += external_hidden_tls;
    
    return result;
}

/* Function that escapes TLS addresses */
static void escape_tls_addresses(void) {
    /* Force addresses to escape */
    static void *escaped_ptrs[10];
    static int ptr_idx = 0;
    
    escaped_ptrs[ptr_idx++ % 10] = &preserved_tls;
    escaped_ptrs[ptr_idx++ % 10] = &public_tls;
    escaped_ptrs[ptr_idx++ % 10] = &common_tls;
    
    /* Use assembly to prevent optimization (if supported) */
    __asm__ volatile ("" : : "r"(&preserved_tls) : "memory");
}

/* ================= MAIN FUNCTION ================= */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed += *p;
        }
    } else {
        seed = 12345;
    }
    
    /* Initialize some TLS variables with runtime values */
    public_tls_uninit = seed;
    common_tls = seed * 2;
    
    /* Force TLS emulation by taking addresses */
    take_tls_addresses();
    
    /* Complex computation using TLS */
    int checksum = compute_with_tls(seed);
    
    /* More TLS operations in loops */
    for (int i = 0; i < (seed % 10 + 1); i++) {
        checksum += aligned_tls + i;
        aligned_tls = checksum % 1000;
        
        /* Access via volatile pointer */
        volatile int *vol_ptr = &volatile_tls;
        *vol_ptr = i;
        checksum += *vol_ptr;
    }
    
    /* Escape addresses to force preservation */
    escape_tls_addresses();
    
    /* Mix with external function call */
    external_function();
    
    /* Final checksum computation using all TLS variables */
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += aligned_tls;
    checksum += preserved_tls;
    
    /* Prevent dead code elimination */
    use_int(checksum);
    
    /* Print result (prevents optimization) */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* ================= SECONDARY COMPILATION UNIT SIMULATION ================= */
/* In a real multi-file test, this would be in a separate source file */

/* External TLS definitions */
__thread int external_public_tls = 999;
__thread int external_weak_tls __attribute__((weak)) = 888;
__thread int external_hidden_tls __attribute__((visibility("hidden"))) = 777;

/* Opaque function stubs for linking */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    static volatile void *last_ptr;
    last_ptr = p;
}

void use_int(int x) {
    /* Empty but prevents optimization */
    static volatile int last_val;
    last_val = x;
}

void external_function(void) {
    /* Access TLS from "external" context */
    external_public_tls++;
    external_hidden_tls--;
}
