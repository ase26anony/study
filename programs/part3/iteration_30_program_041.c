/* test_tls_emulation.c - Comprehensive TLS emulation test */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);
extern int __attribute__((noinline)) opaque_func(int x);

/* Global seed for unpredictable control flow */
static volatile int global_seed = 0;

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* Public TLS with external linkage and default visibility */
__thread int tls_public = 42;
extern __thread int tls_extern;  /* Defined in another compilation unit */

/* Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int tls_internal __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int tls_common;

/* Preserved TLS (address escapes) */
__thread volatile int tls_preserved __asm__("custom_tls_name") = 999;

/* Static TLS with internal linkage */
static __thread int tls_static = 123;

/* TLS with dynamic initialization */
extern int compute_init(void);
__thread int tls_dynamic = compute_init();

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64))) = 777;

/* TLS array */
__thread int tls_array[10];

/* ========== FUNCTIONS THAT OPERATE ON TLS ========== */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline __attribute__((always_inline)) 
int inline_tls_access(int idx) {
    /* Mix static and non-static TLS access */
    tls_static += tls_public;
    tls_array[idx % 10] = tls_hidden;
    return tls_static + tls_array[idx % 10];
}

/* Function that takes address of TLS variables */
static void take_tls_addresses(void) {
    void *addresses[] = {
        &tls_public,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_internal,
        &tls_protected,
        &tls_dllimport,
        &tls_common,
        &tls_preserved,
        &tls_static,
        &tls_dynamic,
        &tls_aligned,
        tls_array
    };
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        use_ptr(addresses[i]);
    }
}

/* Complex TLS usage in loops */
static void tls_loop_operations(int iterations) {
    volatile int *volatile ptr_array[5];
    
    /* Store TLS addresses in volatile pointers */
    ptr_array[0] = &tls_public;
    ptr_array[1] = &tls_hidden;
    ptr_array[2] = &tls_static;
    ptr_array[3] = &tls_dynamic;
    ptr_array[4] = tls_array;
    
    for (int i = 0; i < iterations; i++) {
        /* Unpredictable access pattern */
        int idx = (i * global_seed) % 5;
        
        /* Modify TLS through volatile pointer */
        *ptr_array[idx] += i;
        
        /* Use inline function */
        int val = inline_tls_access(i);
        
        /* Mix with non-TLS operations */
        tls_common = val % 256;
        
        /* Conditional TLS access */
        if (i & 1) {
            tls_weak += tls_internal;
        } else {
            tls_protected -= tls_hidden;
        }
        
        /* Array operations */
        tls_array[i % 10] = *ptr_array[idx % 5];
    }
}

/* Function that creates complex expression with TLS addresses */
static void *get_tls_address_expr(int selector) {
    /* Complex address computation that might force proxy creation */
    switch (selector & 7) {
        case 0: return &tls_public + selector;
        case 1: return &tls_hidden + (selector >> 1);
        case 2: return tls_array + (selector % 10);
        case 3: return (void*)((long)&tls_static ^ selector);
        case 4: return &tls_dynamic;
        case 5: return &tls_weak;
        case 6: return &tls_protected;
        default: return &tls_common;
    }
}

/* ========== MAIN FUNCTION ========== */

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    global_seed = (argc > 1) ? (argv[1][0] * 31 + argv[1][1]) : 12345;
    
    /* Initialize some TLS array elements */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = i * global_seed;
    }
    
    /* Take addresses of all TLS variables */
    take_tls_addresses();
    
    /* Perform loop operations with TLS */
    tls_loop_operations(100 + (global_seed % 50));
    
    /* Complex address expressions */
    for (int i = 0; i < 20; i++) {
        void *addr = get_tls_address_expr(i + global_seed);
        use_ptr(addr);
    }
    
    /* Mix TLS with external function calls */
    tls_public = opaque_func(tls_hidden);
    tls_hidden = opaque_func(tls_public);
    
    /* Compute checksum of all TLS values to prevent elimination */
    int checksum = 0;
    checksum += tls_public;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_dllimport;
    checksum += tls_common;
    checksum += tls_preserved;
    checksum += tls_static;
    checksum += tls_dynamic;
    checksum += tls_aligned;
    
    for (int i = 0; i < 10; i++) {
        checksum += tls_array[i];
    }
    
    /* Use checksum to prevent dead code elimination */
    use_int(checksum);
    
    return checksum & 255;
}

/* ========== EXTERNAL TLS DEFINITION (simulating another file) ========== */
/* This would normally be in a separate file, but included here for completeness */
__thread int tls_extern = 9999;

/* ========== DUMMY FUNCTION DEFINITIONS FOR LINKING ========== */
/* In a real test environment, these would be empty or return dummy values */

void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = 0;
    last_ptr = p;
}

void use_int(int x) {
    /* Prevent optimization */
    static volatile int last_int = 0;
    last_int = x;
}

int get_random(void) {
    return global_seed * 1103515245 + 12345;
}

int __attribute__((noinline)) opaque_func(int x) {
    return x ^ 0x55AA55AA;
}

int compute_init(void) {
    return get_random() % 1000;
}
