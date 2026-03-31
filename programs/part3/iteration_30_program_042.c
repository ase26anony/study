/* test_emutls_attributes.c */
/* Compile with: -O2 -femulated-tls -fvisibility=hidden -fPIC */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);
extern void external_function(void);

/* TLS variables with various attributes */

/* Public TLS with default visibility */
__thread int public_tls = 42;
__thread int public_uninit_tls;

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 400;

/* Static TLS (not public) */
static __thread int static_tls = 500;

/* TLS with DLL import attribute (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Simulate with attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* TLS variable whose address escapes (for DECL_PRESERVE_P) */
__thread int escape_tls = 600;

/* TLS with dynamic initialization */
extern int compute_value(void);
__thread int dynamic_tls = compute_value();

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* External TLS declarations (simulating other compilation units) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* Function that takes address of TLS - may cause declaration cloning */
static void take_tls_address(void) {
    volatile void *addrs[] = {
        &public_tls,
        &weak_tls,
        &hidden_tls,
        &internal_tls,
        &protected_tls,
        &static_tls,
        &escape_tls,
        &dynamic_tls,
        &aligned_tls,
        &external_tls,
        &external_weak_tls
    };
    
    for (int i = 0; i < sizeof(addrs)/sizeof(addrs[0]); i++) {
        use_ptr((void*)addrs[i]);
    }
}

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    switch (idx % 7) {
        case 0: return public_tls++;
        case 1: return weak_tls--;
        case 2: hidden_tls ^= 0xFF;
                return hidden_tls;
        case 3: return internal_tls + static_tls;
        case 4: protected_tls *= 2;
                return protected_tls;
        case 5: return escape_tls | aligned_tls;
        case 6: return dynamic_tls & 0x0F;
        default: return 0;
    }
}

/* Complex function using TLS in loops */
static int process_tls_values(int iterations) {
    int sum = 0;
    volatile int *volatile ptr; /* volatile pointer to volatile prevent optimizations */
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        int val = inline_tls_access(i);
        
        /* Take address in complex way */
        if (i & 1) {
            ptr = &public_tls;
        } else if (i & 2) {
            ptr = &hidden_tls;
        } else {
            ptr = &static_tls;
        }
        
        /* Modify through pointer */
        *ptr += val;
        
        /* Use opaque function */
        use_int(*ptr);
        
        sum += val;
        
        /* Force compiler to consider all TLS variables */
        switch (i % 11) {
            case 0: public_uninit_tls = i; break;
            case 1: weak_tls = i * 2; break;
            case 2: hidden_tls = i * 3; break;
            case 3: internal_tls = i * 4; break;
            case 4: protected_tls = i * 5; break;
            case 5: static_tls = i * 6; break;
            case 6: escape_tls = i * 7; break;
            case 7: dynamic_tls = i * 8; break;
            case 8: aligned_tls = i * 9; break;
            case 9: public_tls = i * 10; break;
            case 10: /* Use external TLS if available */
                     if (&external_tls != NULL) {
                         external_tls = i;
                     }
                     break;
        }
    }
    
    return sum;
}

/* Function that causes TLS address to escape globally */
static void *global_escape_ptr;
static void escape_tls_address(void) {
    global_escape_ptr = &escape_tls;
    /* This should set DECL_PRESERVE_P */
    asm volatile("" : : "r"(&escape_tls) : "memory");
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    int seed = 0;
    if (argc > 1) {
        seed = argv[1][0]; /* Use first char of first argument as seed */
    }
    
    /* Initialize some TLS variables unpredictably */
    public_uninit_tls = seed;
    if (seed & 1) {
        weak_tls = seed * 3;
    }
    
    /* Take addresses of all TLS variables */
    take_tls_address();
    
    /* Escape one TLS variable's address */
    escape_tls_address();
    
    /* Process TLS values with runtime-determined iterations */
    int iterations = (seed % 100) + 10;
    int sum = process_tls_values(iterations);
    
    /* Use TLS variables in conditionals */
    volatile int cond = 0;
    if (public_tls > 1000) cond = 1;
    if (hidden_tls < 0) cond = 2;
    if ((weak_tls & 0xFF) == 0) cond = 3;
    
    /* Compute checksum of all TLS values to prevent elimination */
    int checksum = 
        public_tls + 
        public_uninit_tls + 
        weak_tls + 
        hidden_tls + 
        internal_tls + 
        protected_tls + 
        static_tls + 
        escape_tls + 
        dynamic_tls + 
        aligned_tls + 
        sum + 
        cond;
    
    /* Use checksum so it can't be optimized away */
    use_int(checksum);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* External TLS definitions (simulating another compilation unit) */
__thread int external_tls = 999;
__thread int external_weak_tls __attribute__((weak)) = 888;

/* Dummy definitions for opaque functions (for linking) */
void use_ptr(void *p) {
    static volatile void *last_ptr;
    last_ptr = p;
}

void use_int(int x) {
    static volatile int last_val;
    last_val = x;
}

int compute_value(void) {
    return 1234;
}
