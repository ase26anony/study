/* test_tls_emulation.c - Comprehensive TLS emulation test */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p);
extern int get_random_value(void);
extern void side_effect(int x);

/* Global TLS variables with varied attributes */

/* Public TLS with external linkage */
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

/* Static TLS (file scope only) */
static __thread int static_tls = 500;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 600;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 700;

/* TLS with complex initializer */
extern int compute_init(void);
__thread int complex_init_tls = 0; /* Will be initialized dynamically */

/* External TLS declarations (simulating other compilation units) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* Function that uses TLS and will be inlined */
static inline __attribute__((always_inline)) 
int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch (idx % 4) {
        case 0: return public_tls;
        case 1: return hidden_tls;
        case 2: return static_tls;
        case 3: return aligned_tls;
        default: return 0;
    }
}

/* Another function that takes address of TLS */
static void take_tls_addresses(void) {
    void *addresses[10];
    volatile void *volatile_addr;
    
    /* Take addresses of various TLS variables */
    addresses[0] = &public_tls;
    addresses[1] = &hidden_tls;
    addresses[2] = &protected_tls;
    addresses[3] = &internal_tls;
    addresses[4] = &static_tls;
    addresses[5] = &aligned_tls;
    addresses[6] = (void *)&volatile_tls;
    addresses[7] = &complex_init_tls;
    
    /* Force compiler to keep these addresses */
    for (int i = 0; i < 8; i++) {
        use_ptr(addresses[i]);
        volatile_addr = addresses[i];
        side_effect((int)(long)volatile_addr);
    }
}

/* Function with loop that uses TLS */
static int process_tls_values(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        sum += public_tls;
        sum -= hidden_tls;
        sum += inline_tls_access(i);
        
        /* Modify TLS variables */
        public_tls += i;
        hidden_tls -= i % 3;
        
        /* Use weak TLS if available */
        if (&weak_tls != NULL) {
            sum += weak_tls;
            weak_tls += i;
        }
        
        /* External TLS access */
        sum += external_tls;
    }
    
    return sum;
}

/* Function that creates TLS pointer aliasing */
static void create_tls_aliasing(void) {
    int *aliases[5];
    int **alias_ptr;
    
    aliases[0] = &public_tls;
    aliases[1] = &hidden_tls;
    aliases[2] = &static_tls;
    
    /* Create pointer chains to confuse optimizer */
    alias_ptr = &aliases[0];
    use_ptr2(alias_ptr);
    
    /* Volatile pointer to TLS */
    volatile int *volatile_tls_ptr = &public_tls;
    *volatile_tls_ptr = get_random_value();
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    int seed = 0;
    int checksum = 0;
    volatile int *volatile_ptrs[20];
    int volatile_ptr_count = 0;
    
    /* Get seed from command line for unpredictable flow */
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Dynamic initialization of TLS */
    complex_init_tls = seed;
    
    /* Initialize external TLS (simulating definition) */
    external_tls = seed % 100;
    
    /* Take addresses and store in volatile array */
    volatile_ptrs[volatile_ptr_count++] = &public_tls;
    volatile_ptrs[volatile_ptr_count++] = (volatile int *)&hidden_tls;
    volatile_ptrs[volatile_ptr_count++] = &protected_tls;
    volatile_ptrs[volatile_ptr_count++] = &internal_tls;
    volatile_ptrs[volatile_ptr_count++] = &static_tls;
    volatile_ptrs[volatile_ptr_count++] = &aligned_tls;
    volatile_ptrs[volatile_ptr_count++] = &volatile_tls;
    volatile_ptrs[volatile_ptr_count++] = &complex_init_tls;
    
    /* Call functions that manipulate TLS */
    take_tls_addresses();
    create_tls_aliasing();
    
    /* Process TLS values with seed-dependent iterations */
    int iterations = 10 + (seed % 20);
    checksum = process_tls_values(iterations);
    
    /* Additional unpredictable TLS accesses */
    if (seed & 1) {
        checksum += inline_tls_access(seed);
    }
    if (seed & 2) {
        checksum += *volatile_ptrs[seed % volatile_ptr_count];
    }
    if (seed & 4) {
        /* Access through volatile pointer */
        for (int i = 0; i < volatile_ptr_count; i++) {
            checksum += *volatile_ptrs[i];
        }
    }
    
    /* Use TLS in asm statement to force DECL_PRESERVE_P */
    __asm__ volatile (
        "# TLS reference %0" 
        : "+r" (checksum)
        : "r" (&public_tls)
        : "memory"
    );
    
    /* Final checksum computation using all TLS variables */
    checksum += public_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += static_tls;
    checksum += aligned_tls;
    checksum += volatile_tls;
    checksum += complex_init_tls;
    
    if (&weak_tls != NULL) {
        checksum += weak_tls;
    }
    
    checksum += external_tls;
    
    /* Prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* Definitions for external TLS variables */
__thread int external_tls = 999;
__thread int external_weak_tls __attribute__((weak)) = 1111;

/* Dummy function definitions for linking */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    static volatile int sink;
    sink = (int)(long)p;
}

void use_ptr2(void *p) {
    /* Another empty function */
    (void)p;
}

int get_random_value(void) {
    return 42; /* Not random, but compiler doesn't know */
}

void side_effect(int x) {
    /* Side effect to prevent optimization */
    volatile static int counter = 0;
    counter += x;
}
