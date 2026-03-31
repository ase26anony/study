/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p1, void *p2);
extern int get_random_value(void);
extern void escape_pointer(void **ptr);

/* Global volatile pointer array to prevent dead code elimination */
volatile void *volatile_ptr_array[20];

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* Public TLS with external linkage and default visibility */
__thread int public_tls = 42;
extern __thread int external_public_tls;  /* Will be defined in another "unit" */

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
/* Simulate with attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;  /* No initializer -> common symbol */

/* Static TLS (file-local) with complex initializer */
static __thread int static_tls = 0;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 999;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 777;

/* TLS used in asm (forces DECL_PRESERVE_P) */
__thread int preserved_tls __asm__("custom_tls_name") = 555;

/* ========== FUNCTION DECLARATIONS ========== */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch (idx % 4) {
        case 0: return public_tls++;
        case 1: return hidden_tls--;
        case 2: return protected_tls ^= 1;
        case 3: return volatile_tls;
        default: return 0;
    }
}

/* Function that takes address of TLS variables */
static void take_tls_addresses(void) {
    void *addrs[10];
    
    addrs[0] = (void *)&public_tls;
    addrs[1] = (void *)&weak_tls;
    addrs[2] = (void *)&hidden_tls;
    addrs[3] = (void *)&protected_tls;
    addrs[4] = (void *)&internal_tls;
    addrs[5] = (void *)&common_tls;
    addrs[6] = (void *)&static_tls;
    addrs[7] = (void *)&volatile_tls;
    addrs[8] = (void *)&aligned_tls;
    addrs[9] = (void *)&preserved_tls;
    
    /* Pass addresses to opaque function */
    for (int i = 0; i < 10; i++) {
        use_ptr(addrs[i]);
    }
    
    /* Escape pointers to prevent optimization */
    escape_pointer(&addrs[0]);
}

/* Complex TLS usage in loops */
static void tls_loop_operations(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        public_tls += i;
        hidden_tls -= i % 3;
        protected_tls ^= (i << 2);
        
        /* Use inline function */
        sum += inline_tls_access(i);
        
        /* Conditional TLS access */
        if (i % 5 == 0) {
            volatile_tls = i;
            use_ptr((void *)&volatile_tls);
        }
        
        /* Address computation with TLS */
        if (i % 7 == 0) {
            int *ptr = &common_tls;
            *ptr = i * 2;
            use_ptr2((void *)ptr, (void *)&public_tls);
        }
    }
    
    /* Store result in TLS */
    static_tls = sum % 1000;
}

/* Initialize TLS with runtime values */
static void init_tls_with_runtime(int seed) {
    public_tls = seed;
    hidden_tls = seed * 2;
    protected_tls = seed ^ 0xABCD;
    internal_tls = seed % 100;
    common_tls = seed + 1000;
    static_tls = seed - 500;
    volatile_tls = seed | 0xFF00;
    aligned_tls = seed * seed;
    preserved_tls = seed & 0xFFFF;
}

/* ========== MAIN FUNCTION ========== */

int main(int argc, char *argv[]) {
    int seed = 12345;  /* Default seed */
    
    /* Use argv for runtime control flow */
    if (argc > 1) {
        seed = 0;
        for (int i = 0; argv[1][i]; i++) {
            seed = seed * 31 + argv[1][i];
        }
    }
    
    /* Initialize TLS with unpredictable values */
    init_tls_with_runtime(seed);
    
    /* Take addresses of all TLS variables */
    take_tls_addresses();
    
    /* Perform complex operations */
    tls_loop_operations(seed % 100 + 50);
    
    /* Additional TLS stress: nested loops with TLS access */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Access TLS in inner loop */
            public_tls += (i * j) % 7;
            hidden_tls -= j;
            
            /* Store TLS addresses in volatile array */
            if ((i + j) % 3 == 0) {
                volatile_ptr_array[i] = (void *)&public_tls;
                volatile_ptr_array[j + 10] = (void *)&hidden_tls;
            }
        }
    }
    
    /* Compute checksum of all TLS values to prevent elimination */
    int checksum = 0;
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
    
    /* Mix in address values */
    checksum += (int)((long)&public_tls & 0xFFFF);
    checksum += (int)((long)&hidden_tls & 0xFFFF);
    
    /* Print checksum to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* ========== "EXTERNAL" COMPILATION UNIT SIMULATION ========== */

/* Simulate TLS variable defined in another compilation unit */
__thread int external_public_tls = 9999;

/* Additional TLS with DLL export simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls = 8888;
#else
__thread int exported_tls __attribute__((visibility("default"))) = 8888;
#endif

/* Weak external TLS */
extern __thread int weak_external_tls __attribute__((weak));

/* Function that uses external TLS */
void use_external_tls(void) {
    external_public_tls++;
    if (&weak_external_tls) {
        weak_external_tls = external_public_tls;
    }
    exported_tls = external_public_tls * 2;
}
