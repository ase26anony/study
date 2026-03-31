/* test_tls_emulation.c - Comprehensive TLS emulation test */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);
extern void side_effect(void);

/* Global seed for unpredictable control flow */
static volatile int global_seed = 0;

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage and default visibility */
__thread int tls_public_default = 42;
extern __thread int tls_external_default;  /* Will be defined in another TU */

/* Weak TLS symbol with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 100;

/* Static TLS (internal linkage) with complex initialization */
static __thread int tls_static_complex = 0;
static __thread volatile int tls_static_volatile = 255;

/* TLS with preserved attribute (address escapes) */
__thread int tls_preserved __attribute__((used)) = 777;

/* TLS with DLL import attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with a custom attribute if supported */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int tls_common;

/* TLS with protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 888;

/* TLS with internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 999;

/* TLS requiring dynamic initialization */
__thread int tls_dynamic_init = 0;

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64))) = 1234;

/* TLS array to test address calculations */
__thread int tls_array[10];

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Mix different TLS variables in inline context */
    int result = tls_public_default + tls_static_complex;
    if (idx > 0) {
        result += tls_array[idx % 10];
    }
    /* Take address of TLS variable inside inline function */
    volatile int *addr = &tls_static_volatile;
    *addr += 1;
    return result;
}

/* Function that takes address of TLS and passes to opaque function */
static void escape_tls_addresses(void) {
    void *addresses[] = {
        (void*)&tls_public_default,
        (void*)&tls_weak_hidden,
        (void*)&tls_static_complex,
        (void*)&tls_static_volatile,
        (void*)&tls_preserved,
        (void*)&tls_dllimport,
        (void*)&tls_common,
        (void*)&tls_protected,
        (void*)&tls_internal,
        (void*)&tls_dynamic_init,
        (void*)&tls_aligned,
        (void*)tls_array
    };
    
    for (int i = 0; i < (int)(sizeof(addresses)/sizeof(addresses[0])); i++) {
        use_ptr(addresses[i]);
    }
}

/* Complex function mixing TLS with non-TLS operations */
static int process_tls_variables(int seed) {
    volatile int counter = 0;
    int result = 0;
    
    /* Dynamic initialization based on seed */
    tls_dynamic_init = seed * 2;
    
    /* Use inline function with TLS access */
    result += inline_tls_access(seed % 10);
    
    /* Loop with TLS array access */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = seed + i;
        counter += tls_array[i];  /* volatile access */
    }
    
    /* Conditional TLS access */
    if (seed & 1) {
        tls_public_default += tls_static_complex;
        tls_static_complex = inline_tls_access(seed % 5);
    } else {
        tls_weak_hidden -= tls_protected;
        tls_protected = inline_tls_access(seed % 3);
    }
    
    /* Switch statement with TLS */
    switch (seed % 4) {
        case 0:
            tls_common = tls_internal;
            break;
        case 1:
            tls_internal = tls_aligned;
            break;
        case 2:
            tls_aligned = tls_dynamic_init;
            break;
        case 3:
            tls_dynamic_init = tls_common;
            break;
    }
    
    /* Take address and modify through pointer */
    int * volatile ptr = &tls_static_volatile;
    *ptr = seed;
    
    /* Call opaque function with TLS address */
    use_ptr(&tls_preserved);
    
    return result + counter;
}

/* Function that returns address of TLS - forces preservation */
static volatile int* get_tls_address(int idx) {
    side_effect();
    switch (idx % 6) {
        case 0: return &tls_public_default;
        case 1: return &tls_weak_hidden;
        case 2: return &tls_static_volatile;
        case 3: return &tls_protected;
        case 4: return &tls_internal;
        case 5: return (volatile int*)&tls_aligned;
        default: return &tls_common;
    }
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    global_seed = seed;
    
    /* Initialize TLS array with pattern */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = i * i + seed;
    }
    
    /* Force TLS emulation through address escape */
    escape_tls_addresses();
    
    /* Complex processing with TLS */
    int intermediate = process_tls_variables(seed);
    
    /* Additional unpredictable TLS access pattern */
    volatile int *tls_ptrs[8];
    for (int i = 0; i < 8; i++) {
        tls_ptrs[i] = get_tls_address(seed + i);
        *tls_ptrs[i] += i;
    }
    
    /* Compute checksum of all TLS variables to prevent elimination */
    int checksum = 0;
    checksum += tls_public_default;
    checksum += tls_weak_hidden;
    checksum += tls_static_complex;
    checksum += tls_static_volatile;
    checksum += tls_preserved;
    checksum += tls_dllimport;
    checksum += tls_common;
    checksum += tls_protected;
    checksum += tls_internal;
    checksum += tls_dynamic_init;
    checksum += tls_aligned;
    
    for (int i = 0; i < 10; i++) {
        checksum += tls_array[i];
    }
    
    /* Use checksum to prevent dead code elimination */
    use_int(checksum + intermediate);
    
    /* Return value based on TLS state */
    return (checksum & 0xFF) == (seed & 0xFF) ? 0 : 1;
}

/* ================= EXTERNAL TLS DEFINITION ================= */
/* This simulates a separate compilation unit */
__thread int tls_external_default = 9999;

/* ================= STUB FUNCTIONS FOR LINKING ================= */
/* In a real test environment, these would be empty or return dummy values */
void use_ptr(void *p) {
    static volatile int sink = 0;
    sink += (int)(long)p;
}

void use_int(int x) {
    static volatile int sink = 0;
    sink += x;
}

int get_random(void) {
    static int counter = 0;
    return counter++ * 1103515245 + 12345;
}

void side_effect(void) {
    static volatile int counter = 0;
    counter++;
}
