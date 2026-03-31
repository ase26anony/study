/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and
 * force the compiler to copy declaration attributes between TLS
 * variables during emulation setup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p);
extern int opaque_func(void);
extern void *get_random_ptr(void);

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage - will have TREE_PUBLIC=1 */
__thread int public_tls = 42;

/* Weak TLS symbol - will have DECL_WEAK=1 */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS - tests DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 300;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Simulate with attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Common TLS symbol (tentative definition) - tests DECL_COMMON */
__thread int common_tls;  /* No initializer at file scope */

/* Static TLS (not public) with complex initialization */
static __thread int static_tls = 0;

/* TLS with alignment requirement that might affect emulation */
__thread int aligned_tls __attribute__((aligned(64))) = 512;

/* External TLS declaration (simulating definition in another file) */
extern __thread int external_tls;

/* TLS variable whose address escapes in ways that require preservation */
__thread int preserve_tls = 999;

/* TLS with volatile qualification to prevent optimization */
volatile __thread int volatile_tls = 777;

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function that accesses TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch (idx % 4) {
        case 0: return public_tls;
        case 1: return weak_tls;
        case 2: return hidden_tls;
        case 3: return protected_tls;
        default: return 0;
    }
}

/* Function that takes address of TLS and does complex operations */
static void manipulate_tls_pointers(int seed) {
    /* Array of pointers to TLS variables - their addresses escape */
    void *tls_ptrs[10];
    volatile void *volatile_ptr; /* volatile to prevent optimization */
    
    /* Take addresses of various TLS variables */
    tls_ptrs[0] = &public_tls;
    tls_ptrs[1] = &weak_tls;
    tls_ptrs[2] = &hidden_tls;
    tls_ptrs[3] = &protected_tls;
    tls_ptrs[4] = &common_tls;
    tls_ptrs[5] = &static_tls;
    tls_ptrs[6] = &aligned_tls;
    tls_ptrs[7] = (void*)&volatile_tls;
    tls_ptrs[8] = &preserve_tls;
    
    /* Force the address of preserve_tls to escape through opaque function */
    /* This should set DECL_PRESERVE_P */
    use_ptr(&preserve_tls);
    
    /* Use seed to determine which pointer to pass to opaque function */
    /* This prevents dead code elimination */
    volatile_ptr = tls_ptrs[seed % 9];
    use_ptr2((void*)volatile_ptr);
    
    /* Complex expression with TLS address computation */
    int offset = seed % 64;
    char *tls_char_ptr = (char*)&aligned_tls + offset;
    use_ptr(tls_char_ptr);
}

/* Function with loop that accesses TLS variables */
static int compute_tls_checksum(int iterations, int seed) {
    int sum = 0;
    
    /* Mix TLS and non-TLS accesses in loop */
    for (int i = 0; i < iterations; i++) {
        /* Access TLS variables in ways that might trigger emulation */
        sum += public_tls;
        sum -= weak_tls;
        sum += hidden_tls * (i % 3);
        sum += protected_tls / ((seed % 5) + 1);
        
        /* Use inline function that accesses TLS */
        sum += inline_tls_access(i + seed);
        
        /* Modify TLS variables */
        static_tls += i;
        common_tls = (common_tls + 1) % 100;
        
        /* Volatile access prevents optimization */
        sum += volatile_tls;
    }
    
    return sum;
}

/* Function that creates a local TLS-like context */
static void tls_in_complex_context(void) {
    /* Dynamic initialization of TLS-like context */
    static __thread int dynamic_init_tls = 0;
    dynamic_init_tls = opaque_func();
    
    /* Take address and pass to opaque function */
    use_ptr(&dynamic_init_tls);
    
    /* Nested scope with TLS access */
    {
        volatile __thread int local_scope_tls = 123;
        local_scope_tls += public_tls;
        use_ptr(&local_scope_tls);
    }
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char **argv) {
    int seed = 0;
    int checksum = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize TLS variables with seed-dependent values */
    public_tls = seed;
    weak_tls = seed * 2;
    hidden_tls = seed % 1000;
    protected_tls = seed * seed;
    common_tls = seed % 256;
    static_tls = 1;
    aligned_tls = seed & ~63; /* Ensure alignment */
    volatile_tls = seed ^ 0xAAAA;
    preserve_tls = seed | 0x5555;
    
    /* Force TLS emulation by taking addresses and passing to opaque functions */
    manipulate_tls_pointers(seed);
    
    /* Complex TLS usage pattern */
    for (int phase = 0; phase < 3; phase++) {
        /* Each phase uses TLS differently */
        checksum += compute_tls_checksum(10 + (seed % 10), seed + phase);
        
        /* Call function that uses TLS in inline context */
        tls_in_complex_context();
        
        /* Modify seed based on TLS values (unpredictable) */
        seed = (seed + public_tls + weak_tls) % 1000000;
    }
    
    /* Final checksum computation using all TLS variables */
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += aligned_tls;
    checksum += volatile_tls;
    checksum += preserve_tls;
    
    /* Use checksum so compiler can't eliminate TLS accesses */
    printf("TLS checksum: %d\n", checksum);
    
    /* Simulate external TLS access */
    extern_tls = checksum % 100;
    printf("External TLS would be: %d\n", extern_tls);
    
    return checksum % 256;
}

/* ================= STUB FUNCTIONS FOR LINKING ================= */

/* These would be defined elsewhere in a real test environment */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile int sink;
    sink = *(int*)p;
}

void use_ptr2(void *p) {
    /* Another opaque use */
    static volatile long sink2;
    sink2 = (long)p;
}

int opaque_func(void) {
    return rand();
}

/* External TLS variable definition (simulating another compilation unit) */
__thread int external_tls = 8888;
