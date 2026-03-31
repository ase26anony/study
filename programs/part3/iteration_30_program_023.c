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
extern void side_effect(void);

/* ============================================
 * TLS VARIABLES WITH DIVERSE ATTRIBUTES
 * ============================================ */

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

/* Common TLS symbol (tentative definition) */
__thread int common_tls;

/* Static TLS (internal linkage) */
static __thread int static_tls = 999;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 1234;

/* Dynamic initializer TLS */
extern int get_random(void);
__thread int dynamic_init_tls = 0; /* Will be initialized in main */

/* Alignment-specified TLS */
__thread int aligned_tls __attribute__((aligned(64))) = 777;

/* Extern declarations (simulating other compilation units) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* ============================================
 * FUNCTIONS THAT USE TLS
 * ============================================ */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 0;
    inline_tls += idx;
    return inline_tls + public_tls;
}

/* Function that takes address of TLS and uses it */
static void manipulate_tls_pointers(int seed) {
    /* Array of volatile pointers to prevent optimization */
    volatile void *ptr_array[20];
    int i = 0;
    
    /* Take addresses of various TLS variables */
    ptr_array[i++] = (void*)&public_tls;
    ptr_array[i++] = (void*)&weak_tls;
    ptr_array[i++] = (void*)&hidden_tls;
    ptr_array[i++] = (void*)&protected_tls;
    ptr_array[i++] = (void*)&internal_tls;
    ptr_array[i++] = (void*)&common_tls;
    ptr_array[i++] = (void*)&static_tls;
    ptr_array[i++] = (void*)&volatile_tls;
    ptr_array[i++] = (void*)&aligned_tls;
    
    /* Use seed to select which pointer to pass */
    for (int j = 0; j < i; j++) {
        if ((seed >> j) & 1) {
            use_ptr((void*)ptr_array[j]);
        }
    }
    
    /* Complex expression with TLS address */
    int *tls_ptr = &public_tls;
    tls_ptr += (seed % 4);
    use_ptr2(tls_ptr);
}

/* Function with loop that uses TLS */
static int tls_loop_computation(int iterations, int seed) {
    volatile __thread int loop_tls = 0;
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        loop_tls += i + seed;
        result += inline_tls_access(loop_tls);
        
        /* Conditional TLS access */
        if (i % 3 == 0) {
            hidden_tls += public_tls;
        } else if (i % 3 == 1) {
            protected_tls += weak_tls;
        } else {
            internal_tls += static_tls;
        }
        
        /* Address taking in loop */
        void *addr = (i % 2) ? (void*)&loop_tls : (void*)&public_tls;
        use_ptr(addr);
    }
    
    return result;
}

/* Function that escapes TLS pointers */
static void escape_tls_pointers(void) {
    /* Force TLS addresses to escape */
    static void *escaped_ptrs[10];
    static int ptr_idx = 0;
    
    escaped_ptrs[ptr_idx++ % 10] = (void*)&public_tls;
    escaped_ptrs[ptr_idx++ % 10] = (void*)&weak_tls;
    escaped_ptrs[ptr_idx++ % 10] = (void*)&hidden_tls;
    escaped_ptrs[ptr_idx++ % 10] = (void*)&volatile_tls;
    
    /* Use asm to prevent optimization (GCC-style) */
    asm volatile("" : : "r"(&escaped_ptrs) : "memory");
}

/* ============================================
 * MAIN FUNCTION
 * ============================================ */

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Dynamic initialization of TLS */
    dynamic_init_tls = seed * 2;
    
    /* Initialize common TLS with runtime value */
    common_tls = seed % 100;
    
    /* Manipulate TLS variables based on seed */
    public_tls += seed;
    weak_tls -= seed;
    hidden_tls ^= seed;
    protected_tls |= seed;
    internal_tls &= ~seed;
    static_tls *= (seed % 10) + 1;
    
    /* Force volatile access */
    volatile_tls = seed * 3;
    
    /* Call functions that use TLS extensively */
    manipulate_tls_pointers(seed);
    
    int loop_result = tls_loop_computation(100 + (seed % 50), seed);
    
    escape_tls_pointers();
    
    /* Use inline function with TLS */
    int inline_result = 0;
    for (int i = 0; i < 10; i++) {
        inline_result += inline_tls_access(i + seed);
    }
    
    /* Compute checksum of all TLS values to prevent removal */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += dynamic_init_tls;
    checksum += aligned_tls;
    checksum += loop_result;
    checksum += inline_result;
    
    /* Mix in external TLS if available */
    checksum += (int)(long)&external_tls;
    checksum += (int)(long)&external_weak_tls;
    
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    /* Prevent dead code elimination */
    side_effect();
    
    return checksum % 256;
}

/* ============================================
 * EXTERNAL FUNCTION STUBS (for linking)
 * ============================================ */

/* These would be defined in a separate file in a real multi-file test */
__thread int external_tls = 5555;
__thread int external_weak_tls __attribute__((weak)) = 6666;

/* Stub implementations to satisfy references */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_ptr2(void *p) {
    /* Another opaque use */
    asm volatile("" : : "r"(p) : "memory");
}

int opaque_func(void) {
    return rand();
}

void side_effect(void) {
    /* Memory clobber to prevent optimization */
    asm volatile("" : : : "memory");
}
