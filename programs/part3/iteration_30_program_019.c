/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);
extern void side_effect(void);

/* External TLS declarations (simulating another compilation unit) */
extern __thread int ext_tls_public;
extern __thread int ext_tls_weak __attribute__((weak));
extern __thread int ext_tls_hidden __attribute__((visibility("hidden")));

/* Global TLS variables with diverse attributes */

/* Public TLS with dynamic initialization */
__thread int tls_public = 123;
__thread int tls_public_dyn = 0;

/* Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 456;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 789;

/* Internal visibility TLS */
__thread int tls_internal __attribute__((visibility("internal")));

/* Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 321;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* GCC attribute approximation */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int tls_common;

/* Static TLS (file scope) */
static __thread int tls_static = 654;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 999;

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64)));

/* TLS that might need preservation (address escapes) */
__thread int tls_preserve;

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int modify_tls_inline(int idx, int val) {
    /* Access different TLS variables based on index */
    switch(idx % 5) {
        case 0: tls_public += val; return tls_public;
        case 1: tls_hidden -= val; return tls_hidden;
        case 2: tls_static *= val; return tls_static;
        case 3: tls_volatile = val; return tls_volatile;
        case 4: tls_weak ^= val; return tls_weak;
    }
    return 0;
}

/* Function that takes address of TLS - forces addressability */
static void take_tls_addresses(void) {
    void *tls_ptrs[] = {
        &tls_public,
        &tls_public_dyn,
        &tls_weak,
        &tls_hidden,
        &tls_internal,
        &tls_protected,
        &tls_dllimport,
        &tls_common,
        &tls_static,
        &tls_volatile,
        &tls_aligned,
        &tls_preserve,
        &ext_tls_public,
        &ext_tls_weak,
        &ext_tls_hidden
    };
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < (int)(sizeof(tls_ptrs)/sizeof(tls_ptrs[0])); i++) {
        use_ptr(tls_ptrs[i]);
    }
}

/* Complex TLS usage in loops */
static void process_tls_loop(int iterations, int seed) {
    volatile int * volatile ptr_array[10];
    
    /* Store TLS addresses in volatile array */
    ptr_array[0] = &tls_public;
    ptr_array[1] = &tls_hidden;
    ptr_array[2] = &tls_static;
    ptr_array[3] = &tls_volatile;
    ptr_array[4] = &tls_weak;
    
    for (int i = 0; i < iterations; i++) {
        int idx = (i + seed) % 5;
        
        /* Access TLS through volatile pointer */
        *ptr_array[idx] += i;
        
        /* Use inline function with TLS access */
        int result = modify_tls_inline(i, seed);
        
        /* Mix TLS with non-TLS operations */
        if (i % 3 == 0) {
            tls_common = result * 2;
        }
        
        /* Conditional TLS access */
        if (seed % 2) {
            tls_aligned = *ptr_array[idx] | 0xFF;
        }
        
        /* Call side effect to prevent reordering */
        side_effect();
    }
}

/* Function with local TLS */
static void local_tls_test(void) {
    /* Local TLS variable */
    static __thread int local_tls = 42;
    
    /* Take address of local TLS */
    volatile int *local_ptr = &local_tls;
    
    /* Modify through pointer */
    *local_ptr += get_random();
    
    /* Pass to opaque function */
    use_ptr(local_ptr);
    
    /* Inline function access */
    modify_tls_inline(local_tls % 5, 10);
}

/* Initialize dynamic TLS */
static void init_dynamic_tls(void) {
    /* Simulate dynamic initialization */
    tls_public_dyn = get_random();
    tls_internal = tls_public_dyn * 2;
    tls_aligned = tls_public_dyn & 0xFFFF;
}

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Initialize TLS variables */
    init_dynamic_tls();
    
    /* Set up TLS that needs preservation - take address and escape it */
    tls_preserve = seed;
    void *preserve_ptr = &tls_preserve;
    use_ptr(preserve_ptr);
    
    /* Take addresses of all TLS variables */
    take_tls_addresses();
    
    /* Process TLS in complex loop */
    process_tls_loop(100, seed);
    
    /* Test local TLS */
    local_tls_test();
    
    /* Compute checksum of TLS values to prevent elimination */
    int checksum = 0;
    checksum += tls_public;
    checksum += tls_public_dyn;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_dllimport;
    checksum += tls_common;
    checksum += tls_static;
    checksum += tls_volatile;
    checksum += tls_aligned;
    checksum += tls_preserve;
    
    /* Use checksum to prevent dead code elimination */
    use_int(checksum);
    
    /* Print result so program has observable behavior */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* External TLS definitions (would be in separate file in real project) */
__thread int ext_tls_public = 1111;
__thread int ext_tls_weak = 2222;
__thread int ext_tls_hidden = 3333;

/* Dummy implementations of opaque functions for standalone compilation */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr;
    last_ptr = p;
}

void use_int(int x) {
    static volatile int last_val;
    last_val = x;
}

int get_random(void) {
    static int counter = 0;
    return ++counter;
}

void side_effect(void) {
    static volatile int marker;
    marker++;
}
