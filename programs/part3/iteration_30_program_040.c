/* test-tls-emulation.c */
/* Compile with: -O2 -femulated-tls -fvisibility=hidden -fPIC */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);
extern int some_function(void);

/* Global function that will be inlined */
static inline int inline_tls_access(int idx) {
    /* Access TLS variables inside inline function */
    static __thread int inline_tls1 __attribute__((visibility("default"))) = 100;
    static __thread int inline_tls2 __attribute__((weak)) = 200;
    
    inline_tls1 += idx;
    inline_tls2 -= idx;
    return inline_tls1 + inline_tls2;
}

/* TLS variables with diverse attributes */

/* Public TLS with external linkage */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 123;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 456;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 789;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with attribute if supported */
__thread int imported_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;  /* No initializer */

/* Preserved TLS (address escapes) */
__thread int preserved_tls __attribute__((used)) = 999;

/* Static TLS (internal linkage) */
static __thread int static_tls = 111;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 222;

/* Dynamically initialized TLS */
__thread int dynamic_tls = 0;

/* External TLS declarations (simulating other compilation units) */
extern __thread int external_tls1;
extern __thread int external_tls2 __attribute__((weak));
extern __thread int external_tls3 __attribute__((visibility("protected")));

/* Complex TLS with alignment */
__thread int aligned_tls __attribute__((aligned(64))) = 333;

/* Function that takes address of TLS variables */
static void take_tls_addresses(void **ptrs) {
    ptrs[0] = (void *)&public_tls;
    ptrs[1] = (void *)&weak_tls;
    ptrs[2] = (void *)&hidden_tls;
    ptrs[3] = (void *)&internal_tls;
    ptrs[4] = (void *)&imported_tls;
    ptrs[5] = (void *)&common_tls;
    ptrs[6] = (void *)&preserved_tls;
    ptrs[7] = (void *)&static_tls;
    ptrs[8] = (void *)&volatile_tls;
    ptrs[9] = (void *)&dynamic_tls;
    ptrs[10] = (void *)&external_tls1;
    ptrs[11] = (void *)&aligned_tls;
}

/* Function that uses TLS in complex ways */
static int process_tls_variables(int seed) {
    int result = 0;
    volatile int *volatile_ptr;
    
    /* Force dynamic initialization */
    dynamic_tls = some_function();
    
    /* Mix TLS access with non-TLS operations */
    for (int i = 0; i < 10; i++) {
        /* Vary which TLS variable is accessed */
        switch ((seed + i) % 8) {
            case 0:
                public_tls += i;
                result += public_tls;
                break;
            case 1:
                weak_tls -= i;
                result += weak_tls;
                break;
            case 2:
                hidden_tls *= (i + 1);
                result += hidden_tls;
                break;
            case 3:
                internal_tls /= (i + 1);
                result += internal_tls;
                break;
            case 4:
                /* Use inline function that accesses TLS */
                result += inline_tls_access(i);
                break;
            case 5:
                /* Volatile access */
                volatile_ptr = &volatile_tls;
                *volatile_ptr += seed;
                result += *volatile_ptr;
                break;
            case 6:
                /* Common TLS access */
                common_tls = seed * i;
                result += common_tls;
                break;
            case 7:
                /* Preserved TLS access */
                preserved_tls ^= i;
                result += preserved_tls;
                break;
        }
    }
    
    /* Take addresses and pass to opaque function */
    void *tls_ptrs[12];
    take_tls_addresses(tls_ptrs);
    
    for (int i = 0; i < 12; i++) {
        use_ptr(tls_ptrs[i]);
    }
    
    /* Array of TLS pointers for complex access */
    static __thread int* tls_ptr_array[4];
    tls_ptr_array[0] = &public_tls;
    tls_ptr_array[1] = &weak_tls;
    tls_ptr_array[2] = &hidden_tls;
    tls_ptr_array[3] = &static_tls;
    
    /* Indirect access through TLS pointer array */
    for (int i = 0; i < 4; i++) {
        *tls_ptr_array[i] += result;
        use_ptr(tls_ptr_array[i]);
    }
    
    return result;
}

/* External TLS definitions (simulating other file) */
__thread int external_tls1 __attribute__((visibility("default"))) = 555;
__thread int external_tls2 __attribute__((weak)) = 666;
__thread int external_tls3 __attribute__((visibility("protected"))) = 777;

int main(int argc, char **argv) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = argv[1][0];  /* Use first char of first argument */
    }
    
    /* Initialize aligned TLS with runtime value */
    aligned_tls = seed * 2;
    
    /* Complex TLS processing */
    int checksum = process_tls_variables(seed);
    
    /* Additional TLS usage in main */
    static __thread int main_local_tls __attribute__((visibility("hidden"))) = 888;
    
    for (int i = 0; i < 5; i++) {
        main_local_tls += checksum + i;
        inline_tls_access(main_local_tls);
        
        /* Mix external TLS access */
        external_tls1 += i;
        external_tls2 -= i;
        external_tls3 *= (i + 1);
    }
    
    /* Final checksum computation using all TLS variables */
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += internal_tls;
    checksum += common_tls;
    checksum += preserved_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    checksum += external_tls1;
    checksum += external_tls2;
    checksum += external_tls3;
    checksum += aligned_tls;
    checksum += main_local_tls;
    
    /* Prevent dead code elimination */
    use_int(checksum);
    
    /* Print checksum to prevent removal */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero, non-constant value */
}

/* Stub implementations for opaque functions (for actual compilation) */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr;
    last_ptr = p;
}

void use_int(int x) {
    /* Prevent optimization */
    static volatile int last_val;
    last_val = x;
}

int some_function(void) {
    return 42;  /* Simple return value */
}
