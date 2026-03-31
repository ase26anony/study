/* test_tls_emulation.c - Comprehensive TLS emulation test */

/* Opaque functions to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random_seed(void);
extern void external_function(void);

/* TLS variables with varied attributes */

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
__thread int imported_tls __attribute__((dllimport));
#endif

/* Static TLS (file scope) */
static __thread int static_tls = 500;

/* TLS with dynamic initialization */
extern int compute_value(void);
__thread int dynamic_tls = 0; /* Will be initialized at runtime */

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 600;

/* TLS that might become common symbol */
__thread int common_tls; /* Tentative definition */

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 700;

/* External TLS declarations (simulating other compilation units) */
extern __thread int external_tls;
extern __thread int external_tls_with_attr __attribute__((visibility("default")));

/* Function that uses TLS address - may trigger declaration copying when inlined */
static inline __attribute__((always_inline)) 
void manipulate_tls(int *output) 
{
    /* Mix TLS accesses with non-TLS operations */
    static_tls += public_tls;
    hidden_tls -= volatile_tls;
    
    /* Take address of TLS variables */
    void *ptrs[] = {
        (void*)&public_tls,
        (void*)&static_tls,
        (void*)&hidden_tls,
        (void*)&volatile_tls
    };
    
    /* Use opaque function to prevent optimization */
    for (int i = 0; i < 4; i++) {
        use_ptr(ptrs[i]);
    }
    
    *output = static_tls + hidden_tls;
}

/* Another function that might trigger TLS declaration adjustment */
static void complex_tls_operation(int seed) 
{
    /* Array of pointers to TLS variables with different attributes */
    volatile void *tls_pointers[10];
    
    tls_pointers[0] = (void*)&public_tls;
    tls_pointers[1] = (void*)&weak_tls;
    tls_pointers[2] = (void*)&hidden_tls;
    tls_pointers[3] = (void*)&protected_tls;
    tls_pointers[4] = (void*)&internal_tls;
    tls_pointers[5] = (void*)&static_tls;
    tls_pointers[6] = (void*)&aligned_tls;
    tls_pointers[7] = (void*)&common_tls;
    tls_pointers[8] = (void*)&volatile_tls;
    tls_pointers[9] = (void*)&dynamic_tls;
    
    /* Use seed to access different TLS variables unpredictably */
    for (int i = 0; i < 10; i++) {
        if (seed & (1 << i)) {
            use_ptr((void*)tls_pointers[i]);
        }
    }
    
    /* Modify TLS variables based on seed */
    if (seed & 1) public_tls += seed;
    if (seed & 2) weak_tls -= seed;
    if (seed & 4) hidden_tls ^= seed;
    if (seed & 8) static_tls *= (seed % 10) + 1;
    
    /* Force address escape for DECL_PRESERVE_P */
    asm volatile("" : : "r"(&public_tls), "r"(&hidden_tls) : "memory");
}

/* Function that returns address of TLS - may force proxy creation */
int* get_tls_address(int index) 
{
    switch (index % 5) {
        case 0: return &public_tls;
        case 1: return &weak_tls;
        case 2: return &hidden_tls;
        case 3: return &static_tls;
        case 4: return &volatile_tls;
        default: return &public_tls;
    }
}

int main(int argc, char *argv[]) 
{
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    } else {
        seed = get_random_seed();
    }
    
    /* Initialize dynamic TLS (simulating runtime initialization) */
    dynamic_tls = seed;
    common_tls = seed * 2;
    
    /* Call external function that might affect TLS */
    external_function();
    
    /* Complex TLS operations */
    complex_tls_operation(seed);
    
    /* Use inline function that accesses TLS */
    int inline_result;
    manipulate_tls(&inline_result);
    
    /* Loop with TLS accesses - may trigger optimization passes */
    int sum = 0;
    volatile int * volatile_ptr = &volatile_tls;
    
    for (int i = 0; i < 100; i++) {
        /* Mix TLS and non-TLS accesses */
        public_tls += i;
        static_tls -= i % 7;
        hidden_tls ^= i;
        
        /* Use volatile TLS access */
        sum += *volatile_ptr;
        
        /* Take address in loop */
        if (i % 13 == 0) {
            use_ptr(get_tls_address(i));
        }
        
        /* Conditional TLS modification */
        if (i % 3 == 0) {
            weak_tls += public_tls;
        }
    }
    
    /* Compute checksum of all TLS variables to prevent elimination */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += static_tls;
    checksum += dynamic_tls;
    checksum += aligned_tls;
    checksum += common_tls;
    checksum += volatile_tls;
    checksum += inline_result;
    checksum += sum % 1000;
    
    /* Use checksum so it can't be optimized away */
    use_int(checksum);
    
    /* Print result to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* Additional TLS definitions in same file */
__thread int external_tls = 800;
__thread int external_tls_with_attr = 900;

/* Stub implementations for opaque functions (for linking) */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    static volatile int sink;
    sink = (int)(long)p;
}

void use_int(int x) {
    printf("Used: %d\n", x);
}

int get_random_seed(void) {
    return 12345;
}

void external_function(void) {
    /* Access TLS to force external linkage handling */
    public_tls += 1;
}
