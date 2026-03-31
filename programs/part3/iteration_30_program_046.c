/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern int get_random_value(void);
extern void escape_pointer(void **p);

/* TLS variables with varied attributes */

/* Public TLS with default visibility */
__thread int public_tls = 42;
__thread int public_tls_noinit;

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 300;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 400;

/* Static TLS (not public) */
static __thread int static_tls = 500;

/* TLS with DLL import attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Simulate with attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* TLS with preservation requirement (address escapes) */
__thread volatile int preserved_tls = 600;

/* TLS with dynamic initialization */
extern int compute_value(void);
__thread int dynamic_tls = 0; /* Will be initialized dynamically */

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls; /* No initializer at file scope */

/* TLS used in inline function */
static inline int get_static_tls_value(void) {
    /* This inline function accesses static TLS */
    return static_tls;
}

/* Function that takes address of TLS and might cause declaration copying */
static void manipulate_tls_pointers(int seed) {
    void *tls_pointers[20];
    volatile void *volatile_ptr;
    
    /* Store addresses of all TLS variables in array */
    tls_pointers[0] = (void *)&public_tls;
    tls_pointers[1] = (void *)&public_tls_noinit;
    tls_pointers[2] = (void *)&weak_tls;
    tls_pointers[3] = (void *)&hidden_tls;
    tls_pointers[4] = (void *)&protected_tls;
    tls_pointers[5] = (void *)&internal_tls;
    tls_pointers[6] = (void *)&static_tls;
    tls_pointers[7] = (void *)&dllimport_tls;
    tls_pointers[8] = (void *)&preserved_tls;
    tls_pointers[9] = (void *)&dynamic_tls;
    tls_pointers[10] = (void *)&aligned_tls;
    tls_pointers[11] = (void *)&external_tls;
    tls_pointers[12] = (void *)&external_weak_tls;
    tls_pointers[13] = (void *)&common_tls;
    
    /* Force compiler to keep all addresses */
    volatile_ptr = tls_pointers[seed % 14];
    
    /* Pass TLS addresses to opaque function */
    for (int i = 0; i < 14; i++) {
        use_ptr(tls_pointers[i]);
    }
    
    /* Escape pointer to prevent optimization */
    escape_pointer(&tls_pointers[0]);
}

/* Complex TLS usage in loops with runtime values */
static int process_tls_values(int iterations, int modifier) {
    int result = 0;
    
    /* Use TLS in loop with runtime-dependent access pattern */
    for (int i = 0; i < iterations; i++) {
        /* Switch between different TLS variables based on runtime values */
        switch ((i + modifier) % 8) {
            case 0:
                result += public_tls;
                public_tls += i;
                break;
            case 1:
                result += weak_tls;
                weak_tls ^= i;
                break;
            case 2:
                result += hidden_tls;
                hidden_tls -= i;
                break;
            case 3:
                result += static_tls;
                static_tls = get_static_tls_value() + 1;
                break;
            case 4:
                result += preserved_tls;
                preserved_tls *= (i + 1);
                break;
            case 5:
                result += aligned_tls;
                aligned_tls |= i;
                break;
            case 6:
                result += common_tls;
                common_tls = i;
                break;
            case 7:
                result += dynamic_tls;
                dynamic_tls = (dynamic_tls + i) % 100;
                break;
        }
        
        /* Mix with external TLS */
        if (i % 3 == 0) {
            result += external_tls;
        }
    }
    
    return result;
}

/* Function that creates local TLS-like context */
static void nested_tls_access(void) {
    /* Local static TLS-like variable */
    static __thread int local_static_tls = 999;
    
    /* Take address and use in complex expression */
    int *ptr = &local_static_tls;
    *ptr += get_random_value();
    
    /* Use in inline assembly to force preservation */
    __asm__ volatile ("" : "+r"(ptr) : : "memory");
    
    use_ptr(ptr);
}

/* Initialize dynamic TLS */
static void init_dynamic_tls(void) {
    dynamic_tls = get_random_value() % 1000;
}

int main(int argc, char *argv[]) {
    int seed = 0;
    int checksum = 0;
    
    /* Use argv for runtime control flow */
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed += *p;
        }
    }
    
    /* Initialize TLS variables */
    init_dynamic_tls();
    public_tls_noinit = seed;
    common_tls = seed * 2;
    
    /* Manipulate TLS pointers to trigger declaration copying */
    manipulate_tls_pointers(seed);
    
    /* Complex TLS processing with runtime-dependent iterations */
    int iterations = 100 + (seed % 50);
    checksum = process_tls_values(iterations, seed);
    
    /* Nested TLS access */
    nested_tls_access();
    
    /* Additional volatile accesses to prevent optimization */
    volatile int *volatile_tls_ptr = &preserved_tls;
    for (int i = 0; i < 10; i++) {
        *volatile_tls_ptr += i;
        __asm__ volatile ("" : : "r"(volatile_tls_ptr) : "memory");
    }
    
    /* Compute final checksum covering all TLS variables */
    checksum += public_tls;
    checksum += public_tls_noinit;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += static_tls;
    checksum += preserved_tls;
    checksum += dynamic_tls;
    checksum += aligned_tls;
    checksum += common_tls;
    
    /* Use inline function */
    checksum += get_static_tls_value();
    
    /* Print result to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* External TLS definitions (simulating another file) */
__thread int external_tls = 12345;
__thread int external_weak_tls __attribute__((weak)) = 54321;

/* Dummy implementations for opaque functions (for actual compilation) */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr;
    last_ptr = p;
}

int get_random_value(void) {
    return 42; /* Deterministic for testing */
}

void escape_pointer(void **p) {
    /* Prevent optimization */
    static volatile void *escaped_ptr;
    escaped_ptr = p;
}
