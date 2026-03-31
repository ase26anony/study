/* test_tls_emulation.c - Comprehensive TLS emulation test */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);
extern void escape(void *p);

/* TLS variables with diverse attributes */

/* Public TLS with external linkage */
__thread int public_tls = 42;
__thread int public_uninit_tls;

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

/* Static TLS (not public) */
static __thread int static_tls = 500;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 600;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 700;

/* TLS with complex initializer */
extern int compute_init(void);
__thread int complex_init_tls = 0; /* Will be initialized dynamically */

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls; /* No initializer at file scope */

/* Function that takes address of TLS - may cause declaration cloning */
static inline __attribute__((always_inline)) 
void manipulate_tls_inline(int *out) {
    /* Access multiple TLS variables in inline function */
    static __thread int local_static_tls = 0;
    local_static_tls++;
    
    /* Take address of TLS variable */
    int *addr = &local_static_tls;
    escape(addr);
    
    /* Mix with other TLS */
    *out = public_tls + local_static_tls + static_tls;
    
    /* Volatile access to prevent optimization */
    volatile int v = volatile_tls;
    (void)v;
}

/* Non-inline function that uses TLS extensively */
void __attribute__((noinline)) process_tls(int seed) {
    /* Array of TLS pointers for address-taking */
    void *tls_pointers[10];
    volatile void *volatile_escape;
    
    /* Initialize complex TLS dynamically */
    complex_init_tls = seed * 2;
    
    /* Take addresses of various TLS variables */
    tls_pointers[0] = (void*)&public_tls;
    tls_pointers[1] = (void*)&weak_tls;
    tls_pointers[2] = (void*)&hidden_tls;
    tls_pointers[3] = (void*)&protected_tls;
    tls_pointers[4] = (void*)&static_tls;
    tls_pointers[5] = (void*)&aligned_tls;
    tls_pointers[6] = (void*)&volatile_tls;
    tls_pointers[7] = (void*)&complex_init_tls;
    tls_pointers[8] = (void*)&common_tls;
    tls_pointers[9] = (void*)&external_tls;
    
    /* Force compiler to keep all address operations */
    for (int i = 0; i < 10; i++) {
        use_ptr(tls_pointers[i]);
        volatile_escape = tls_pointers[i];
    }
    
    /* Use inline function that accesses TLS */
    int inline_result;
    manipulate_tls_inline(&inline_result);
    use_int(inline_result);
    
    /* Complex expression with TLS addresses */
    int *ptr_array[5];
    ptr_array[0] = &public_tls;
    ptr_array[1] = &hidden_tls;
    ptr_array[2] = &static_tls;
    ptr_array[3] = &complex_init_tls;
    ptr_array[4] = &common_tls;
    
    /* Opaque operation on pointer array */
    escape(ptr_array);
    
    /* Loop with TLS access that might trigger optimizations */
    int sum = 0;
    for (int i = 0; i < seed % 8; i++) {
        public_tls += i;
        hidden_tls -= i;
        static_tls *= (i + 1);
        sum += aligned_tls;
        
        /* Access through volatile pointer */
        volatile int *vol_ptr = &volatile_tls;
        sum += *vol_ptr;
    }
    
    /* Conditional based on TLS values */
    if (public_tls > 100) {
        weak_tls = protected_tls + internal_tls;
    } else {
        hidden_tls = public_tls * 2;
    }
    
    /* Switch statement with TLS */
    switch (seed % 4) {
        case 0:
            external_tls = public_tls;
            break;
        case 1:
            external_tls = hidden_tls;
            break;
        case 2:
            external_tls = static_tls;
            break;
        case 3:
            external_tls = complex_init_tls;
            break;
    }
}

/* Another function that returns TLS address - might cause declaration cloning */
int* __attribute__((noinline)) get_tls_address(int selector) {
    static __thread int func_static_tls = 999;
    
    switch (selector % 6) {
        case 0: return &public_tls;
        case 1: return &weak_tls;
        case 2: return &hidden_tls;
        case 3: return &func_static_tls;
        case 4: return &common_tls;
        case 5: return &external_tls;
    }
    return &public_tls;
}

int main(int argc, char **argv) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Initialize common TLS */
    common_tls = seed + 1000;
    
    /* Initialize external TLS (simulating definition) */
    external_tls = seed + 2000;
    external_weak_tls = seed + 3000;
    
    /* Process TLS with seed-dependent behavior */
    process_tls(seed);
    
    /* Get and use TLS addresses */
    for (int i = 0; i < 10; i++) {
        int *tls_ptr = get_tls_address(seed + i);
        use_ptr(tls_ptr);
        
        /* Modify through pointer */
        *tls_ptr += i;
        
        /* Volatile access */
        volatile int *vptr = tls_ptr;
        use_int(*vptr);
    }
    
    /* Compute checksum of all TLS values to prevent elimination */
    int checksum = 0;
    
    /* Access all TLS variables in non-volatile way for checksum */
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += static_tls;
    checksum += aligned_tls;
    checksum += volatile_tls;
    checksum += complex_init_tls;
    checksum += common_tls;
    checksum += external_tls;
    checksum += external_weak_tls;
    
    /* Mix with imported TLS if available */
#ifdef _WIN32
    checksum += imported_tls;
#endif
    
    /* Use inline function one more time */
    int final_result;
    manipulate_tls_inline(&final_result);
    checksum += final_result;
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional: take address of TLS in printf argument */
    printf("Public TLS address: %p, value: %d\n", 
           (void*)&public_tls, public_tls);
    printf("Hidden TLS address: %p, value: %d\n", 
           (void*)&hidden_tls, hidden_tls);
    
    return checksum == 0 ? 0 : 1;
}

/* Dummy definitions for opaque functions (for linking) */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    static volatile void *sink;
    sink = p;
}

void use_int(int x) {
    static volatile int sink;
    sink = x;
}

int get_random(void) {
    return 42; /* Deterministic for testing */
}

void escape(void *p) {
    static volatile void *sink;
    sink = p;
}
