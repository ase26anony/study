/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);
extern void side_effect(void);

/* External TLS variables (simulating another compilation unit) */
extern __thread int ext_tls_public;
extern __thread int ext_tls_hidden __attribute__((visibility("hidden")));
extern __thread int ext_tls_weak __attribute__((weak));

/* Public TLS with various attributes */
__thread int tls_public = 42;
__thread int tls_public_weak __attribute__((weak)) = 100;
__thread volatile int tls_public_vol = 255;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 123;
static __thread int tls_static_hidden __attribute__((visibility("hidden"))) = 456;

/* Weak TLS symbols */
__thread int tls_weak_def __attribute__((weak)) = 789;
__thread int tls_tentative; /* Should become DECL_COMMON */

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute if supported */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* TLS with complex initialization */
extern int compute_init(void);
__thread int tls_dynamic_init = 0; /* Will be set dynamically */
__thread int tls_func_init = 0; /* Will be initialized via function */

/* TLS with alignment requirements */
__thread int tls_aligned __attribute__((aligned(64))) = 999;

/* TLS used in asm (forces DECL_PRESERVE_P) */
register __thread int tls_asm_used asm("tls_reg") = 111;

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int tls_inline_helper = 555;
    tls_inline_helper += idx;
    
    /* Mix with external TLS */
    ext_tls_public += tls_inline_helper;
    
    /* Address taken in inline function */
    void *addr = &tls_inline_helper;
    use_ptr(addr);
    
    return tls_inline_helper;
}

/* Function that takes address of TLS with varying attributes */
static void process_tls_variable(void *ptr, int id) {
    volatile static __thread int tls_local_proc = 0;
    tls_local_proc++;
    
    int *tls_ptr = (int *)ptr;
    *tls_ptr += id + tls_local_proc;
    
    /* Force compiler to keep the variable */
    asm volatile("" : "+m" (*tls_ptr));
}

/* Main test function */
int main(int argc, char *argv[]) {
    int seed = 0;
    if (argc > 1) seed = argv[1][0];
    
    /* Dynamic initialization */
    tls_dynamic_init = seed * 2;
    tls_func_init = get_random();
    
    /* Array of TLS variable pointers for varied access */
    volatile void *tls_pointers[20];
    int tls_values[20];
    int i = 0;
    
    /* Take addresses of all TLS variables */
    tls_pointers[i++] = &tls_public;
    tls_pointers[i++] = &tls_public_weak;
    tls_pointers[i++] = &tls_public_vol;
    tls_pointers[i++] = &tls_hidden;
    tls_pointers[i++] = &tls_static_hidden;
    tls_pointers[i++] = &tls_weak_def;
    tls_pointers[i++] = &tls_tentative;
    tls_pointers[i++] = &tls_dllimport;
    tls_pointers[i++] = &tls_dynamic_init;
    tls_pointers[i++] = &tls_func_init;
    tls_pointers[i++] = &tls_aligned;
    tls_pointers[i++] = &tls_asm_used;
    
    /* External TLS addresses (may be unresolved) */
    tls_pointers[i++] = &ext_tls_public;
    tls_pointers[i++] = &ext_tls_hidden;
    tls_pointers[i++] = &ext_tls_weak;
    
    /* Access TLS variables in unpredictable order based on seed */
    for (int iter = 0; iter < 100; iter++) {
        int idx = (iter * seed) % i;
        
        /* Mix of direct and indirect access */
        if (iter % 3 == 0) {
            /* Direct access with volatile to prevent optimization */
            volatile int *ptr = (volatile int *)tls_pointers[idx];
            *ptr += iter + idx;
        } else if (iter % 3 == 1) {
            /* Use inline function */
            int val = inline_tls_access(iter);
            ((int *)tls_pointers[idx])[0] ^= val;
        } else {
            /* Process through function */
            process_tls_variable(tls_pointers[idx], iter);
        }
        
        /* Call opaque function with TLS pointer */
        use_ptr((void *)tls_pointers[idx]);
        
        /* Side effect to prevent reordering */
        side_effect();
    }
    
    /* Complex expression with multiple TLS variables */
    int complex_result = 
        tls_public * tls_hidden +
        tls_public_weak / (tls_dynamic_init ? tls_dynamic_init : 1) +
        tls_aligned - tls_asm_used;
    
    /* Loop with TLS-dependent condition */
    int counter = 0;
    while (counter < 10) {
        static __thread int tls_loop_counter = 0;
        tls_loop_counter++;
        
        /* TLS address escape in loop */
        int *escape_ptr = &tls_loop_counter;
        use_ptr(escape_ptr);
        
        /* Condition depends on multiple TLS variables */
        if ((tls_public + tls_loop_counter) % 3 == seed % 3) {
            tls_hidden += complex_result;
        }
        
        counter++;
    }
    
    /* Compute checksum of all TLS values */
    unsigned int checksum = 0;
    checksum ^= tls_public;
    checksum ^= tls_public_weak * 2;
    checksum ^= tls_public_vol;
    checksum ^= tls_hidden;
    checksum ^= tls_static_hidden;
    checksum ^= tls_weak_def;
    checksum ^= tls_tentative;
    checksum ^= tls_dynamic_init;
    checksum ^= tls_func_init;
    checksum ^= tls_aligned;
    checksum ^= tls_asm_used;
    
    /* Use checksum to prevent dead code elimination */
    use_int(checksum);
    
    /* Print result (prevents optimization) */
    printf("TLS checksum: %u\n", checksum);
    
    return checksum & 0xFF;
}

/* External function definitions (stubs for linking) */
void use_ptr(void *p) {
    /* Prevent optimization */
    volatile static int sink;
    sink = (int)(long)p;
}

void use_int(int x) {
    volatile static int sink;
    sink = x;
}

int get_random(void) {
    return 42; /* Deterministic for testing */
}

void side_effect(void) {
    volatile static int counter = 0;
    counter++;
}

/* External TLS definitions (simulating another file) */
__thread int ext_tls_public = 1;
__thread int ext_tls_hidden __attribute__((visibility("hidden"))) = 2;
__thread int ext_tls_weak __attribute__((weak)) = 3;
