/* test_emutls_attributes.c */
/* Compile with: -O2 -femulated-tls -fvisibility=hidden -fPIC */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p);
extern int get_random_value(void);
extern void side_effect(void);

/* Global opaque function pointer to prevent inlining analysis */
void (* volatile func_ptr)(void *) = use_ptr;

/* TLS variables with diverse attributes */

/* Public TLS with external linkage */
__thread int public_tls = 42;
__thread int public_tls2 __attribute__((visibility("default"))) = 100;

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 200;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 300;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 400;

/* Static TLS (internal linkage) */
static __thread int static_tls = 500;

/* TLS with DLL import attribute (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with attribute if supported */
__thread int imported_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;

/* External TLS declaration (simulating definition in another file) */
extern __thread int external_tls;

/* TLS with complex initializer */
__thread int dynamic_init_tls = 0;

/* TLS used in asm (forces preservation) */
__thread int asm_tls __asm__("asm_named_tls") = 600;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 700;

/* Alignment-specified TLS */
__thread int aligned_tls __attribute__((aligned(64))) = 800;

/* Function that takes address of TLS and uses it */
static inline __attribute__((always_inline)) 
void inline_tls_access(int *out) {
    /* This inline function accesses TLS, potentially causing 
       declaration duplication during inlining */
    static_tls++;
    *out = public_tls + static_tls;
    
    /* Take address of TLS with hidden visibility */
    use_ptr2(&hidden_tls);
}

/* Another function that manipulates TLS addresses */
void process_tls_pointers(void **ptrs, int count) {
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) {
            /* Force compiler to consider TLS address escape */
            func_ptr(ptrs[i]);
        }
    }
}

int main(int argc, char **argv) {
    int seed = 0;
    if (argc > 1) {
        seed = argv[1][0];  /* Use command line input for variability */
    }
    
    /* Initialize dynamic TLS with runtime value */
    dynamic_init_tls = seed * 10;
    
    /* Initialize common TLS */
    common_tls = seed + 1000;
    
    /* Array to store TLS addresses (volatile to prevent optimization) */
    void * volatile tls_addresses[20];
    int addr_index = 0;
    
    /* Take addresses of all TLS variables with different attributes */
    tls_addresses[addr_index++] = (void *)&public_tls;
    tls_addresses[addr_index++] = (void *)&public_tls2;
    tls_addresses[addr_index++] = (void *)&weak_tls;
    tls_addresses[addr_index++] = (void *)&hidden_tls;
    tls_addresses[addr_index++] = (void *)&protected_tls;
    tls_addresses[addr_index++] = (void *)&static_tls;
    tls_addresses[addr_index++] = (void *)&imported_tls;
    tls_addresses[addr_index++] = (void *)&common_tls;
    tls_addresses[addr_index++] = (void *)&dynamic_init_tls;
    tls_addresses[addr_index++] = (void *)&asm_tls;
    tls_addresses[addr_index++] = (void *)&volatile_tls;
    tls_addresses[addr_index++] = (void *)&aligned_tls;
    
    /* Use inline function that accesses TLS */
    int inline_result;
    for (int i = 0; i < 5; i++) {
        inline_tls_access(&inline_result);
        public_tls += inline_result;
    }
    
    /* Complex expression with TLS address-taking */
    int *tls_ptr_array[5];
    tls_ptr_array[0] = &public_tls;
    tls_ptr_array[1] = &weak_tls;
    tls_ptr_array[2] = &hidden_tls;
    tls_ptr_array[3] = &static_tls;
    tls_ptr_array[4] = &common_tls;
    
    /* Use runtime value to select which TLS to access */
    int selector = seed % 5;
    volatile_tls = *tls_ptr_array[selector];
    
    /* Process all TLS addresses through opaque function */
    process_tls_pointers((void **)tls_addresses, addr_index);
    
    /* Use TLS in loop with runtime-dependent iterations */
    int sum = 0;
    int iterations = (seed % 10) + 5;
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        public_tls += i;
        static_tls -= i % 3;
        hidden_tls = public_tls * static_tls;
        
        /* Take address in loop */
        if (i % 2 == 0) {
            use_ptr(&hidden_tls);
        }
        
        sum += public_tls + static_tls + hidden_tls;
    }
    
    /* Additional TLS manipulation based on seed */
    switch (seed % 4) {
        case 0:
            weak_tls = public_tls * 2;
            break;
        case 1:
            protected_tls = static_tls + hidden_tls;
            break;
        case 2:
            aligned_tls = weak_tls ^ protected_tls;
            break;
        case 3:
            asm_tls = volatile_tls | common_tls;
            break;
    }
    
    /* Compute checksum of all TLS values (prevents dead code elimination) */
    int checksum = 0;
    checksum += public_tls;
    checksum += public_tls2;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += static_tls;
    checksum += common_tls;
    checksum += dynamic_init_tls;
    checksum += asm_tls;
    checksum += volatile_tls;
    checksum += aligned_tls;
    
    /* Use checksum to prevent optimization */
    if (checksum != 0) {
        /* Simulate output to prevent removal */
        side_effect();
    }
    
    return checksum & 0xFF;  /* Return non-constant value */
}

/* Simulate external TLS definition in another compilation unit */
__thread int external_tls = 9999;

/* Dummy definitions for opaque functions (for linking) */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    (void)p;
}

void use_ptr2(void *p) {
    /* Empty but prevents optimization */
    (void)p;
}

void side_effect(void) {
    /* Simulate observable side effect */
    static int counter = 0;
    counter++;
}
