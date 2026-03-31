/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int val);
extern int get_random(void);
extern void escape(void *p);

/* Global volatile pointer array to force TLS address taking */
static volatile void *volatile ptr_array[20];

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage and default visibility */
__thread int public_tls = 42;
extern __thread int external_public_tls;  /* Will be defined in another TU */

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
/* Use dllimport-like attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;  /* No initializer = common symbol */

/* Static TLS (internal linkage) */
static __thread int static_tls = 500;

/* Volatile TLS to prevent optimization */
__thread volatile int volatile_tls = 600;

/* Dynamically initialized TLS */
extern int compute_init(void);
__thread int dynamic_tls = compute_init();

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* TLS used in asm (forces DECL_PRESERVE_P) */
__thread int asm_tls __asm__("custom_asm_tls_symbol") = 800;

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 0;
    inline_tls += idx;
    return inline_tls;
}

/* Function that takes address of TLS and escapes it */
static void escape_tls_addresses(void) {
    ptr_array[0] = &public_tls;
    ptr_array[1] = &weak_tls;
    ptr_array[2] = &hidden_tls;
    ptr_array[3] = &protected_tls;
    ptr_array[4] = &internal_tls;
    ptr_array[5] = &common_tls;
    ptr_array[6] = &static_tls;
    ptr_array[7] = &volatile_tls;
    ptr_array[8] = &dynamic_tls;
    ptr_array[9] = &aligned_tls;
    ptr_array[10] = &asm_tls;
    
    /* Pass addresses to opaque function */
    for (int i = 0; i < 11; i++) {
        use_ptr((void*)ptr_array[i]);
    }
}

/* Complex function mixing TLS and non-TLS operations */
static int process_tls_variables(int seed) {
    int result = seed;
    
    /* Access TLS in unpredictable pattern based on seed */
    if (seed & 1) {
        public_tls += seed;
        result ^= public_tls;
    }
    
    if (seed & 2) {
        weak_tls -= seed;
        result += weak_tls;
    }
    
    if (seed & 4) {
        hidden_tls *= (seed % 10) + 1;
        result |= hidden_tls;
    }
    
    /* Use inline function with TLS */
    result += inline_tls_access(seed);
    
    /* Loop with TLS access - may trigger optimizations that copy declarations */
    for (int i = 0; i < (seed % 8); i++) {
        static_tls += i;
        volatile_tls = static_tls;
        result += volatile_tls;
    }
    
    /* Conditional TLS initialization */
    if (seed > 100) {
        common_tls = seed * 2;
    } else {
        common_tls = seed / 2;
    }
    result += common_tls;
    
    /* Access aligned TLS */
    aligned_tls = result;
    result = aligned_tls;
    
    /* Use asm TLS */
    asm_tls = result ^ 0x55AA55AA;
    result = asm_tls;
    
    return result;
}

/* Function that creates a local TLS proxy scenario */
static void* create_tls_proxy(void) {
    /* Taking address of TLS and doing complex operations */
    int *local_ptr = &public_tls;
    *local_ptr += get_random();
    
    /* Cast through void* to obscure type */
    void *opaque_ptr = local_ptr;
    
    /* Store in volatile array to prevent optimization */
    static volatile void *storage;
    storage = opaque_ptr;
    
    return (void*)storage;
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Get seed from command line for unpredictable flow */
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Initialize some TLS variables based on seed */
    if (seed != 0) {
        public_tls = seed;
        weak_tls = seed * 2;
        hidden_tls = seed * 3;
    }
    
    /* Force address taking and escaping of TLS variables */
    escape_tls_addresses();
    
    /* Complex processing that may trigger TLS declaration copying */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        int val = process_tls_variables(seed + i);
        checksum = (checksum * 31 + val) & 0x7FFFFFFF;
        
        /* Create proxy scenarios periodically */
        if (i % 3 == 0) {
            void *proxy = create_tls_proxy();
            use_ptr(proxy);
        }
    }
    
    /* Additional TLS mixing with external declarations */
    {
        /* Simulate external TLS access */
        extern __thread int external_tls_var;
        external_tls_var = checksum;
        checksum ^= external_tls_var;
        
        /* Access DLL import TLS */
        dllimport_tls = checksum;
        checksum += dllimport_tls;
    }
    
    /* Final checksum computation using all TLS variables */
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    checksum += aligned_tls;
    checksum += asm_tls;
    
    /* Use checksum to prevent dead code elimination */
    use_int(checksum);
    
    /* Print result (prevents optimization) */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* ================= EXTERNAL TLS DEFINITIONS ================= */

/* Define external TLS variables (simulating another compilation unit) */
__thread int external_public_tls = 999;
__thread int external_tls_var = 1111;

/* ================= DUMMY FUNCTION DEFINITIONS ================= */

/* These would normally be in a separate file, but included here for completeness */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr;
    last_ptr = p;
}

void use_int(int val) {
    /* Prevent optimization */
    static volatile int last_val;
    last_val = val;
}

int get_random(void) {
    /* Simple pseudo-random */
    static int state = 12345;
    state = state * 1103515245 + 12345;
    return (state >> 16) & 32767;
}

int compute_init(void) {
    return get_random() % 1000;
}
