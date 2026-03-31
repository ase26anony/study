/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern int get_random_value(void);
extern void side_effect(void);

/* Global seed for unpredictable control flow */
static volatile int global_seed;

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage and default visibility */
__thread int public_tls = 42;
extern __thread int external_public_tls;  /* Defined elsewhere */

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

/* Common TLS (tentative definition) - should trigger DECL_COMMON */
__thread int common_tls;  /* No initializer */

/* Static TLS (file scope, internal linkage) */
static __thread int static_tls = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* TLS with preserved attribute (used in asm) */
__thread int preserved_tls = 800;

/* TLS with dynamic initialization */
extern int compute_init(void);
__thread int dynamic_tls = 0;  /* Will be initialized in main */

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch(idx % 4) {
        case 0: return public_tls;
        case 1: return static_tls;
        case 2: return hidden_tls;
        case 3: return volatile_tls;
        default: return 0;
    }
}

/* Function that takes address of TLS - forces addressability */
static void take_tls_addresses(void) {
    void *addresses[] = {
        (void*)&public_tls,
        (void*)&weak_tls,
        (void*)&hidden_tls,
        (void*)&protected_tls,
        (void*)&internal_tls,
        (void*)&common_tls,
        (void*)&static_tls,
        (void*)&volatile_tls,
        (void*)&aligned_tls,
        (void*)&preserved_tls,
        (void*)&dynamic_tls
    };
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        use_ptr(addresses[i]);
    }
}

/* Function with complex TLS usage pattern */
static int complex_tls_usage(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        int val = inline_tls_access(i);
        
        /* Modify TLS variables based on control flow */
        if (i % 3 == 0) {
            public_tls += val;
        } else if (i % 3 == 1) {
            static_tls -= val;
        } else {
            hidden_tls ^= val;
        }
        
        /* Use volatile TLS */
        sum += volatile_tls;
        
        /* Take address in loop - may force emulation structures */
        if (i % 7 == 0) {
            void *ptr = &aligned_tls;
            use_ptr(ptr);
        }
    }
    
    return sum;
}

/* Function that escapes TLS pointers */
static void escape_tls_pointers(int seed) {
    /* Array of pointers to TLS variables */
    volatile void *tls_pointers[10];
    
    /* Store addresses based on seed */
    tls_pointers[0] = (seed & 1) ? (void*)&public_tls : (void*)&static_tls;
    tls_pointers[1] = (seed & 2) ? (void*)&hidden_tls : (void*)&protected_tls;
    tls_pointers[2] = (void*)&internal_tls;
    tls_pointers[3] = (void*)&common_tls;
    tls_pointers[4] = (void*)&volatile_tls;
    tls_pointers[5] = (void*)&aligned_tls;
    tls_pointers[6] = (void*)&preserved_tls;
    tls_pointers[7] = (void*)&dynamic_tls;
    
    /* Pass to opaque function */
    for (int i = 0; i < 8; i++) {
        use_ptr((void*)tls_pointers[i]);
    }
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    int seed = (argc > 1) ? (argv[1][0] * 31 + argv[1][1]) : 12345;
    global_seed = seed;
    
    /* Initialize dynamic TLS */
    dynamic_tls = seed;
    
    /* Initialize common TLS */
    common_tls = seed * 2;
    
    /* Mark preserved TLS as used in asm (simulated) */
    /* This should trigger DECL_PRESERVE_P */
    asm volatile("" : "+m" (preserved_tls));
    
    /* Complex TLS usage pattern */
    int result = complex_tls_usage(seed % 100 + 50);
    
    /* Take addresses of all TLS variables */
    take_tls_addresses();
    
    /* Escape TLS pointers with control flow */
    escape_tls_pointers(seed);
    
    /* Use TLS variables in conditionals */
    if (public_tls > static_tls) {
        hidden_tls = public_tls - static_tls;
    } else {
        protected_tls = static_tls - public_tls;
    }
    
    /* Loop with TLS-dependent termination */
    int counter = 0;
    while (counter < (seed % 20)) {
        internal_tls += counter;
        volatile_tls -= counter;
        counter++;
        
        /* Take address inside loop */
        if (counter % 5 == 0) {
            use_ptr(&common_tls);
        }
    }
    
    /* Compute checksum of all TLS values to prevent elimination */
    int checksum = 
        public_tls + 
        weak_tls + 
        hidden_tls + 
        protected_tls + 
        internal_tls + 
        common_tls + 
        static_tls + 
        volatile_tls + 
        aligned_tls + 
        preserved_tls + 
        dynamic_tls + 
        result;
    
    /* Use checksum to prevent dead code elimination */
    side_effect();
    
    return checksum % 256;
}

/* ================= EXTERNAL TLS DEFINITIONS ================= */

/* Define the external TLS variable (simulating another compilation unit) */
__thread int external_public_tls = 999;

/* Weak TLS definition (alternative) */
__thread int weak_tls_alt __attribute__((weak)) = 1111;

/* ================= DUMMY FUNCTION DEFINITIONS ================= */

/* These would be linked from another file in a real test */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr;
    last_ptr = p;
}

int get_random_value(void) {
    return global_seed * 1103515245 + 12345;
}

void side_effect(void) {
    /* Empty but prevents optimization */
    static volatile int counter;
    counter++;
}
