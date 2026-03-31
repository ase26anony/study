/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p1, void *p2);
extern int get_random_value(void);
extern void side_effect(void);

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
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Simulate with attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;

/* Static TLS (local linkage) */
static __thread int static_tls = 500;

/* Volatile TLS to prevent optimization */
__thread volatile int volatile_tls = 600;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 700;

/* TLS with preserved attribute (used in asm) */
__thread int preserved_tls = 800;

/* External declarations (simulating other compilation units) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* Complex initialized TLS */
extern int compute_init(void);
__thread int complex_init_tls = 0;

/* Function that takes address of TLS - may trigger declaration copying */
static inline __attribute__((always_inline)) 
void inline_tls_access(int *out) {
    /* Access multiple TLS variables in inline function */
    static __thread int inline_tls = 999;
    *out = public_tls + static_tls + inline_tls;
    
    /* Take address of TLS variable */
    void *addr = &inline_tls;
    use_ptr(addr);
}

/* Function that uses TLS in complex ways */
static void process_tls_variables(int seed, int *result) {
    volatile void *addr_array[20];
    int idx = 0;
    
    /* Store addresses of TLS variables in volatile array */
    addr_array[idx++] = (void*)&public_tls;
    addr_array[idx++] = (void*)&weak_tls;
    addr_array[idx++] = (void*)&hidden_tls;
    addr_array[idx++] = (void*)&protected_tls;
    addr_array[idx++] = (void*)&internal_tls;
    addr_array[idx++] = (void*)&dllimport_tls;
    addr_array[idx++] = (void*)&common_tls;
    addr_array[idx++] = (void*)&static_tls;
    addr_array[idx++] = (void*)&volatile_tls;
    addr_array[idx++] = (void*)&aligned_tls;
    addr_array[idx++] = (void*)&preserved_tls;
    
    /* Use opaque function calls with TLS addresses */
    for (int i = 0; i < idx; i++) {
        use_ptr((void*)addr_array[i]);
    }
    
    /* Mix TLS accesses with runtime values */
    int sum = 0;
    for (int i = 0; i < (seed % 10) + 1; i++) {
        sum += public_tls;
        sum += static_tls;
        
        /* Conditional TLS access */
        if (seed & (1 << i)) {
            sum += hidden_tls;
            volatile_tls = i;  /* Volatile write */
        }
    }
    
    /* Call inline function that accesses TLS */
    int inline_result;
    inline_tls_access(&inline_result);
    sum += inline_result;
    
    /* Take address of TLS and pass to function expecting two pointers */
    use_ptr2(&public_tls, &static_tls);
    
    *result = sum;
}

/* Function using TLS in asm statement to trigger DECL_PRESERVE_P */
static void asm_tls_access(void) {
    /* Mark as used in asm to preserve it */
    __asm__ volatile ("# %0" : : "r"(&preserved_tls));
    
    /* Actual asm that might reference TLS */
    int val;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(val)
        : "m"(preserved_tls)
        : "%eax"
    );
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Get seed from argv for unpredictable control flow */
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Initialize complex TLS with function call */
    complex_init_tls = seed;
    
    /* Dynamic initialization simulation */
    if (seed & 1) {
        public_tls = seed;
        weak_tls = seed * 2;
        hidden_tls = seed * 3;
    }
    
    /* Access external TLS variables */
    external_tls = seed + 100;
    external_weak_tls = seed + 200;
    
    /* Use TLS in loop with runtime-dependent iterations */
    int loop_count = (seed % 100) + 1;
    int tls_sum = 0;
    
    for (int i = 0; i < loop_count; i++) {
        /* Mix different TLS accesses */
        tls_sum += public_tls;
        tls_sum -= static_tls;
        
        /* Conditional TLS modification */
        if (i % 3 == 0) {
            volatile_tls = i;
            tls_sum += volatile_tls;
        }
        
        /* Address taking in loop */
        void *tls_addr = (i & 1) ? (void*)&public_tls : (void*)&static_tls;
        use_ptr(tls_addr);
    }
    
    /* Process TLS variables with complex function */
    int process_result;
    process_tls_variables(seed, &process_result);
    
    /* Use TLS in asm */
    asm_tls_access();
    
    /* Compute checksum of all TLS values */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += aligned_tls;
    checksum += preserved_tls;
    checksum += complex_init_tls;
    checksum += tls_sum;
    checksum += process_result;
    
    /* Prevent dead code elimination */
    if (checksum == 0x12345678) {
        side_effect();
    }
    
    /* Print checksum to prevent removal */
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    return checksum & 0xFF;
}

/* Additional file to simulate multi-compilation unit */

/* tls_extern.c - Simulate separate compilation unit */
#ifdef MULTI_FILE
__thread int external_tls = 1234;
__thread int external_weak_tls __attribute__((weak)) = 5678;

/* Opaque function implementations for linking */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    static int counter;
    counter += (int)(long)p;
}

void use_ptr2(void *p1, void *p2) {
    static int counter;
    counter += (int)(long)p1 + (int)(long)p2;
}

void side_effect(void) {
    /* Do nothing */
}
#endif
