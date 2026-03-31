/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p);
extern int get_random_value(void);
extern void side_effect(void);

/* Global seed for unpredictable control flow */
static volatile int global_seed;

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage and default visibility */
__thread int tls_public_default = 42;
extern __thread int tls_extern_default;  /* Will be defined in another TU */

/* Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int tls_internal __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute if supported */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int tls_common;  /* No initializer = common symbol */

/* Static TLS with preservation requirement */
static __thread int tls_static_preserve = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 600;

/* TLS with dynamic initialization */
__thread int tls_dynamic = 0;

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64))) = 700;

/* TLS array */
__thread int tls_array[10];

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function that accesses TLS - may trigger declaration copying */
static inline int inline_tls_access(int idx) {
    /* Access multiple TLS variables to force emulation */
    int sum = tls_public_default + tls_hidden;
    
    /* Take address of TLS variable inside inline function */
    int *ptr = &tls_static_preserve;
    use_ptr(ptr);
    
    /* Array access with TLS */
    if (idx >= 0 && idx < 10) {
        sum += tls_array[idx];
    }
    
    return sum;
}

/* Function that takes address of TLS and uses it in complex way */
static void process_tls_addresses(void) {
    /* Array of pointers to TLS variables */
    volatile void *tls_pointers[20];
    int i = 0;
    
    /* Take addresses of all TLS variables */
    tls_pointers[i++] = (void*)&tls_public_default;
    tls_pointers[i++] = (void*)&tls_weak;
    tls_pointers[i++] = (void*)&tls_hidden;
    tls_pointers[i++] = (void*)&tls_internal;
    tls_pointers[i++] = (void*)&tls_protected;
    tls_pointers[i++] = (void*)&tls_dllimport;
    tls_pointers[i++] = (void*)&tls_common;
    tls_pointers[i++] = (void*)&tls_static_preserve;
    tls_pointers[i++] = (void*)&tls_volatile;
    tls_pointers[i++] = (void*)&tls_dynamic;
    tls_pointers[i++] = (void*)&tls_aligned;
    tls_pointers[i++] = (void*)tls_array;
    
    /* Pass all addresses to opaque function */
    for (int j = 0; j < i; j++) {
        use_ptr((void*)tls_pointers[j]);
    }
    
    /* Use TLS in asm statement to force DECL_PRESERVE_P */
    __asm__ volatile (
        "# TLS access in asm\n"
        : 
        : "r"(&tls_static_preserve)
        : "memory"
    );
}

/* Function that mixes TLS with runtime values */
static int compute_with_tls(int seed) {
    int result = 0;
    
    /* Unpredictable access pattern */
    switch (seed % 8) {
        case 0:
            result = tls_public_default * seed;
            break;
        case 1:
            result = tls_hidden + seed;
            break;
        case 2:
            result = tls_internal - seed;
            break;
        case 3:
            result = tls_protected ^ seed;
            break;
        case 4:
            /* Array access with bounds checking */
            result = tls_array[seed % 10];
            break;
        case 5:
            /* Call inline function */
            result = inline_tls_access(seed % 10);
            break;
        case 6:
            /* Use volatile TLS */
            result = tls_volatile;
            break;
        case 7:
            /* Common TLS */
            result = tls_common + seed;
            break;
    }
    
    return result;
}

/* Loop with TLS accesses that might trigger optimizations */
static void tls_loop_operations(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Modify TLS variables in loop */
        tls_public_default += i;
        tls_hidden -= i;
        tls_array[i % 10] = i;
        
        /* Use inline function */
        sum += inline_tls_access(i % 10);
        
        /* Take address and pass to opaque function periodically */
        if (i % 7 == 0) {
            use_ptr2(&tls_static_preserve);
        }
    }
    
    /* Store result in TLS */
    tls_common = sum;
}

/* Initialize dynamic TLS */
static void init_dynamic_tls(void) {
    /* Simulate dynamic initialization */
    tls_dynamic = get_random_value();
    
    /* Initialize array with pattern */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = i * i;
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
    
    /* Initialize TLS variables */
    init_dynamic_tls();
    
    /* Process TLS addresses - forces emulation structures */
    process_tls_addresses();
    
    /* Perform loop operations with TLS */
    tls_loop_operations(seed % 100 + 50);
    
    /* Compute checksum using all TLS variables */
    int checksum = 0;
    
    /* Access all TLS variables in non-volatile way */
    checksum += tls_public_default;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_dllimport;
    checksum += tls_common;
    checksum += tls_static_preserve;
    checksum += tls_volatile;
    checksum += tls_dynamic;
    checksum += tls_aligned;
    
    for (int i = 0; i < 10; i++) {
        checksum += tls_array[i];
    }
    
    /* Mix with compute function */
    checksum += compute_with_tls(seed);
    
    /* Print result to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* ================= EXTERNAL DEFINITIONS ================= */

/* Define the extern TLS variable */
__thread int tls_extern_default = 999;

/* Stub implementations for opaque functions (for actual compilation) */
void __attribute__((noinline)) use_ptr(void *p) {
    /* Prevent optimization */
    asm volatile("" : : "r"(p) : "memory");
}

void __attribute__((noinline)) use_ptr2(void *p) {
    /* Another opaque function */
    asm volatile("" : : "r"(p) : "memory");
}

int __attribute__((noinline)) get_random_value(void) {
    return 12345;  /* Deterministic for testing */
}
