/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern int get_random_value(void);
extern void escape_pointer(void **p);

/* Global seed for unpredictable control flow */
static volatile int global_seed = 0;

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage and default visibility */
__thread int tls_public_default = 42;
extern __thread int tls_extern_default;  /* Will be defined in another TU */

/* Weak TLS symbol with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 100;

/* Static TLS (internal linkage) with complex initialization */
static __thread int tls_static_complex = 0;
static __thread int tls_static_initialized = get_random_value();

/* TLS with DLL import attribute (Windows-like) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute if supported */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* TLS with preserved attribute (address escapes) */
__thread volatile int tls_preserved __asm__("preserved_tls_var") = 255;

/* Common TLS (tentative definition) - may become DECL_COMMON */
__thread int tls_common;

/* TLS with protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 777;

/* TLS array with alignment requirement */
__thread int tls_aligned_array[16] __attribute__((aligned(64)));

/* ================= FUNCTION DECLARATIONS ================= */

static inline int inline_tls_access(int idx) {
    /* Inline function accessing TLS - may trigger declaration copying */
    static __thread int tls_inline_local = 99;
    tls_inline_local += idx;
    return tls_inline_local + tls_public_default;
}

static void modify_tls_via_pointer(void) {
    /* Take addresses of TLS variables to force addressability */
    void *tls_pointers[] = {
        &tls_public_default,
        &tls_weak_hidden,
        &tls_static_complex,
        &tls_dllimport,
        &tls_preserved,
        &tls_common,
        &tls_protected,
        tls_aligned_array
    };
    
    volatile void **volatile escape_array = (volatile void**)tls_pointers;
    
    /* Force compiler to consider all TLS addresses as escaping */
    for (int i = 0; i < sizeof(tls_pointers)/sizeof(tls_pointers[0]); i++) {
        use_ptr((void*)escape_array[i]);
    }
    
    /* Additional escape through external function */
    escape_pointer((void**)&tls_pointers[3]);
}

static int compute_tls_checksum(int seed) {
    int sum = seed;
    
    /* Access TLS variables in unpredictable order based on seed */
    sum += (seed & 1) ? tls_public_default : tls_weak_hidden;
    sum += (seed & 2) ? tls_static_complex : tls_protected;
    sum += (seed & 4) ? tls_common : tls_aligned_array[seed % 16];
    
    /* Use inline function that accesses TLS */
    sum += inline_tls_access(seed % 8);
    
    /* Complex expression with TLS address taken */
    int * volatile ptr = &tls_preserved;
    sum += *ptr * (seed % 5);
    
    /* Loop with TLS access that may trigger transformations */
    for (int i = 0; i < (seed % 8); i++) {
        static __thread int tls_loop_counter = 0;
        tls_loop_counter++;
        sum += tls_loop_counter;
        
        /* Mix with array access */
        tls_aligned_array[i] = tls_loop_counter * i;
    }
    
    return sum;
}

/* External TLS definition (simulating another compilation unit) */
__thread int tls_extern_default = 123;

/* ================= MAIN FUNCTION ================= */

int main(int argc, char *argv[]) {
    /* Unpredictable control flow based on input */
    int seed = (argc > 1) ? (argv[1][0] * 17) : 42;
    global_seed = seed;
    
    /* Initialize some TLS variables with runtime values */
    tls_static_complex = seed * 2;
    tls_common = seed * 3;
    
    /* Modify TLS via pointers (forces addressability) */
    modify_tls_via_pointer();
    
    /* Complex TLS usage pattern */
    volatile int temp = 0;
    for (int i = 0; i < 100; i++) {
        /* Conditional TLS access */
        if (i % (seed % 7 + 1)) {
            tls_public_default += inline_tls_access(i);
        } else {
            tls_weak_hidden -= i;
        }
        
        /* Array access with TLS index */
        int idx = (tls_public_default + i) % 16;
        tls_aligned_array[idx] = i * seed;
        
        /* Volatile access to prevent optimization */
        temp += tls_aligned_array[idx];
    }
    
    /* Compute final checksum using all TLS variables */
    int checksum = compute_tls_checksum(seed);
    
    /* Mix in preserved TLS (address was taken) */
    checksum ^= tls_preserved;
    
    /* Use checksum to prevent dead code elimination */
    if (checksum != 0) {
        /* Simulate output to prevent optimization */
        asm volatile("" : : "r"(checksum));
    }
    
    return checksum & 0xFF;
}

/* ================= STUB FUNCTIONS ================= */
/* These would be provided in a separate file for linking */

void use_ptr(void *p) {
    /* Prevent optimization of pointer use */
    static volatile void *last_ptr = 0;
    last_ptr = p;
}

int get_random_value(void) {
    return 42;  /* Deterministic for testing */
}

void escape_pointer(void **p) {
    /* Force pointer to escape */
    static void *escaped_ptr = 0;
    escaped_ptr = *p;
}
