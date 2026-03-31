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
__thread int tls_public = 42;                         /* Public, initialized */
__thread int tls_common;                              /* Common symbol */
__thread volatile int tls_volatile = 100;             /* Volatile */
static __thread int tls_static = 7;                   /* Static linkage */
__thread int tls_weak __attribute__((weak)) = 99;     /* Weak symbol */

/* TLS with visibility attributes */
__thread int tls_default __attribute__((visibility("default"))) = 1;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;
__thread int tls_internal __attribute__((visibility("internal"))) = 3;
__thread int tls_protected __attribute__((visibility("protected"))) = 4;

/* TLS that might be preserved across optimizations */
__thread int tls_preserve __asm__("custom_tls_name") = 77;

/* Dynamic initializer (forces runtime initialization) */
extern int compute_init(void);
__thread int tls_dynamic = compute_init();

/* Alignment attribute to complicate layout */
__thread int tls_aligned __attribute__((aligned(64))) = 256;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute if supported */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Inline function that accesses TLS (may trigger declaration copying during inlining) */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 0;
    inline_tls += idx;
    
    /* Mix with external TLS */
    if (ext_tls_weak)
        inline_tls += ext_tls_weak;
    
    return inline_tls;
}

/* Function that takes address of TLS variables */
static void take_tls_addresses(void) {
    void *addresses[] = {
        &tls_public,
        &tls_common,
        &tls_volatile,
        &tls_static,
        &tls_weak,
        &tls_hidden,
        &tls_internal,
        &tls_protected,
        &tls_preserve,
        &tls_dynamic,
        &tls_aligned,
        &tls_dllimport,
        &ext_tls_public,
        &ext_tls_weak,
        &ext_tls_hidden
    };
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        use_ptr(addresses[i]);
    }
}

/* Complex expression with TLS that might force proxy creation */
static int tls_complex_expr(int x) {
    /* Taking address in complex context */
    int * volatile ptr = &tls_public;
    
    /* TLS in conditional with side effects */
    int result = (tls_volatile > 50) ? 
                 (*ptr + tls_static) : 
                 (tls_hidden - tls_internal);
    
    /* Mix with inline function */
    result += inline_tls_access(x);
    
    /* Chain of TLS accesses */
    tls_common = tls_protected + tls_default;
    tls_default = tls_common * 2;
    
    return result;
}

/* Loop that accesses TLS variables */
static void tls_loop_operations(int iterations) {
    volatile int * volatile ptr_array[5];
    
    /* Store TLS addresses in volatile array */
    ptr_array[0] = &tls_public;
    ptr_array[1] = &tls_volatile;
    ptr_array[2] = &tls_hidden;
    ptr_array[3] = &tls_internal;
    ptr_array[4] = &tls_protected;
    
    for (int i = 0; i < iterations; i++) {
        /* Unpredictable access pattern */
        int idx = i % 5;
        
        /* Modify TLS through volatile pointer */
        *ptr_array[idx] += i;
        
        /* Call inline function with TLS access */
        inline_tls_access(*ptr_array[idx]);
        
        /* Complex expression */
        if (i & 1) {
            tls_complex_expr(i);
        }
        
        /* Prevent loop optimization */
        side_effect();
    }
}

/* Function that returns address of TLS (forces TLS machinery) */
int *get_tls_address(int selector) {
    switch (selector % 6) {
        case 0: return &tls_public;
        case 1: return &tls_hidden;
        case 2: return &tls_internal;
        case 3: return &tls_protected;
        case 4: return &tls_weak;
        case 5: return &tls_static;
        default: return &tls_common;
    }
}

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Initialize external TLS (simulating definition) */
    ext_tls_public = seed;
    ext_tls_hidden = seed * 2;
    if (&ext_tls_weak) {  /* Check if weak symbol is present */
        ext_tls_weak = seed * 3;
    }
    
    /* Take addresses early to force TLS setup */
    take_tls_addresses();
    
    /* Perform loop operations with TLS */
    tls_loop_operations(seed % 100 + 10);
    
    /* Use TLS in conditionals with runtime values */
    for (int i = 0; i < 20; i++) {
        int *tls_ptr = get_tls_address(seed + i);
        *tls_ptr += i * i;
        
        /* Mix with volatile access */
        tls_volatile ^= *tls_ptr;
        
        /* Call opaque function */
        use_int(*tls_ptr);
    }
    
    /* Compute checksum of all TLS values (prevents dead code elimination) */
    int checksum = 0;
    checksum += tls_public;
    checksum += tls_common;
    checksum += tls_volatile;
    checksum += tls_static;
    checksum += tls_weak;
    checksum += tls_default;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_preserve;
    checksum += tls_dynamic;
    checksum += tls_aligned;
    checksum += tls_dllimport;
    checksum += ext_tls_public;
    checksum += ext_tls_hidden;
    checksum += ext_tls_weak;
    
    /* Add inline TLS */
    checksum += inline_tls_access(0);
    
    /* Print checksum to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Stub implementations for external functions (for actual compilation) */
#ifdef COMPILE_WITH_STUBS
void use_ptr(void *p) { (void)p; }
void use_int(int x) { (void)x; }
int get_random(void) { return 42; }
void side_effect(void) { static int counter = 0; counter++; }
int compute_init(void) { return get_random() * 2; }

/* Definitions for external TLS variables */
__thread int ext_tls_public = 0;
__thread int ext_tls_weak = 0;
__thread int ext_tls_hidden = 0;
#endif
