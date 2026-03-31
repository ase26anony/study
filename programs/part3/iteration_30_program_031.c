/* test_tls_emulation.c - Comprehensive TLS emulation test */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p);
extern int get_random_value(void);
extern void side_effect(int x);

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage, default visibility */
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

/* Common TLS symbol (tentative definition) */
__thread int common_tls;  /* No initializer - should become common */

/* Static TLS (file-local) with complex initialization */
static __thread int static_tls = 0;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 999;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 777;

/* TLS that might need preservation (address escapes) */
__thread int preserve_tls = 555;

/* ================= FUNCTION-LEVEL TLS VARIABLES ================= */

static void process_tls_values(int seed) {
    /* TLS inside a function */
    static __thread int func_static_tls = 0;
    
    /* Mix TLS with non-TLS operations */
    func_static_tls += seed;
    public_tls ^= seed;
    hidden_tls += public_tls;
    
    /* Take address of function-local TLS - forces emulation structure */
    int *func_tls_ptr = &func_static_tls;
    use_ptr(func_tls_ptr);
    
    /* Complex expression with TLS address */
    int * volatile ptr_array[3];
    ptr_array[0] = &func_static_tls;
    ptr_array[1] = &public_tls;
    ptr_array[2] = &hidden_tls;
    
    for (int i = 0; i < 3; i++) {
        use_ptr2(ptr_array[i]);
    }
}

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline __attribute__((always_inline)) 
void inline_tls_access(int *output) {
    /* Access multiple TLS variables */
    *output = public_tls + hidden_tls + protected_tls;
    
    /* Take address of TLS inside inline function */
    int *tls_ptr = &public_tls;
    use_ptr(tls_ptr);
    
    /* Force preservation by using in asm-like context */
    __asm__ volatile ("" : : "r"(tls_ptr) : "memory");
}

/* Function that returns address of TLS - causes address to escape */
int* __attribute__((noinline)) get_preserve_tls_addr(void) {
    /* This should set DECL_PRESERVE_P */
    return &preserve_tls;
}

/* ================= MAIN FUNCTION WITH COMPLEX TLS USAGE ================= */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv to make control flow unpredictable */
    if (argc > 1) {
        seed = argv[1][0];  /* Use first char of first argument as seed */
    }
    
    /* Initialize some TLS with runtime values */
    static_tls = seed * 2;
    volatile_tls = seed * 3;
    
    /* Dynamic initialization of TLS */
    __thread int dynamic_init_tls = get_random_value();
    
    /* Array of pointers to TLS variables - forces address computation */
    volatile void *tls_pointers[10];
    
    tls_pointers[0] = &public_tls;
    tls_pointers[1] = &weak_tls;
    tls_pointers[2] = &hidden_tls;
    tls_pointers[3] = &protected_tls;
    tls_pointers[4] = &internal_tls;
    tls_pointers[5] = &dllimport_tls;
    tls_pointers[6] = &common_tls;
    tls_pointers[7] = &static_tls;
    tls_pointers[8] = &volatile_tls;
    tls_pointers[9] = &aligned_tls;
    
    /* Pass all TLS addresses to opaque functions */
    for (int i = 0; i < 10; i++) {
        use_ptr((void*)tls_pointers[i]);
    }
    
    /* Process TLS values based on seed */
    process_tls_values(seed);
    
    /* Use inline function with TLS access */
    int inline_result;
    inline_tls_access(&inline_result);
    
    /* Complex loop with TLS mixing */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Mix TLS access with loop variable */
        public_tls += i % 7;
        hidden_tls ^= i;
        
        /* Conditional TLS access */
        if (i & 1) {
            protected_tls += public_tls;
        } else {
            internal_tls += hidden_tls;
        }
        
        /* Use TLS address in computation */
        int *ptr = (i & 2) ? &public_tls : &hidden_tls;
        sum += *ptr + i;
        
        /* Prevent optimization */
        side_effect(sum);
    }
    
    /* Get address of preserve_tls - should mark it for preservation */
    int *preserve_ptr = get_preserve_tls_addr();
    use_ptr(preserve_ptr);
    
    /* Simulate external TLS variable access */
    external_public_tls = seed * 5;
    use_ptr(&external_public_tls);
    
    /* Final checksum computation using all TLS variables */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += dllimport_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += aligned_tls;
    checksum += preserve_tls;
    checksum += dynamic_init_tls;
    checksum += inline_result;
    
    /* Use checksum to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* ================= "EXTERNAL" TLS DEFINITIONS ================= */

/* Simulate definitions from another compilation unit */
__thread int external_public_tls = 1234;

/* Weak external TLS */
extern __thread int weak_external_tls __attribute__((weak));

/* Common external TLS */
extern __thread int common_external_tls;
