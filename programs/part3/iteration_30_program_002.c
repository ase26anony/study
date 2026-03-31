/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and force
 * the compiler to copy declaration attributes between TLS variables.
 * It uses a wide variety of TLS variables with different linkage,
 * visibility, and storage attributes to cover all lines in the
 * target block from tree-emutls.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int i);
extern int get_random_value(void);
extern void side_effect(void);

/* Define a dummy use_ptr to avoid linker errors when testing */
void use_ptr(void *p) { (void)p; }
void use_int(int i) { (void)i; }
int get_random_value(void) { return 42; }
void side_effect(void) { }

/* ===================== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===================== */

/* Public TLS with external linkage - will have TREE_PUBLIC = 1 */
__thread int public_tls = 100;
__thread int public_tls_uninit;

/* Weak TLS symbol - will have DECL_WEAK = 1 */
__thread int weak_tls __attribute__((weak)) = 200;

/* TLS with hidden visibility - covers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 300;

/* TLS with protected visibility */
__thread int protected_tls __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with attribute if supported */
__thread int imported_tls __attribute__((dllimport));
#endif

/* Common TLS symbol (tentative definition) - may get DECL_COMMON = 1 */
__thread int common_tls;

/* Static TLS (file scope) - not public, tests DECL_EXTERNAL = 0 */
static __thread int static_tls = 500;

/* TLS that should be preserved (used in asm-like context) */
__thread int preserve_tls = 600;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 700;

/* TLS with dynamic initialization */
__thread int dynamic_tls = 0;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 800;

/* ===================== FUNCTIONS THAT USE TLS ===================== */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 900;
    inline_tls += idx;
    return inline_tls;
}

/* Function that takes address of TLS and uses it in complex ways */
static void manipulate_tls_pointers(int seed) {
    /* Array of volatile pointers to prevent optimization */
    volatile void *ptr_array[20];
    int i = 0;
    
    /* Take addresses of various TLS variables */
    ptr_array[i++] = (void*)&public_tls;
    ptr_array[i++] = (void*)&weak_tls;
    ptr_array[i++] = (void*)&hidden_tls;
    ptr_array[i++] = (void*)&protected_tls;
    ptr_array[i++] = (void*)&imported_tls;
    ptr_array[i++] = (void*)&common_tls;
    ptr_array[i++] = (void*)&static_tls;
    ptr_array[i++] = (void*)&preserve_tls;
    ptr_array[i++] = (void*)&volatile_tls;
    ptr_array[i++] = (void*)&aligned_tls;
    
    /* Use the pointers in opaque calls */
    for (int j = 0; j < i; j++) {
        use_ptr((void*)ptr_array[j]);
    }
    
    /* Mix TLS access with runtime values */
    if (seed & 1) {
        public_tls += seed;
    }
    if (seed & 2) {
        hidden_tls -= seed;
    }
    if (seed & 4) {
        protected_tls *= (seed % 10) + 1;
    }
}

/* Function that forces TLS address to escape */
static void escape_tls_address(void) {
    /* Simulate asm statement that might force preservation */
    __asm__ volatile ("# TLS address escape point" : : "r"(&preserve_tls));
    
    /* Pass TLS address to external function */
    use_ptr(&preserve_tls);
    
    /* Use in a way that might require preservation */
    volatile int *volatile p = &preserve_tls;
    *p = get_random_value();
}

/* Complex initialization function for dynamic TLS */
static int init_value(void) {
    side_effect();
    return get_random_value();
}

/* ===================== MAIN FUNCTION ===================== */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize dynamic TLS */
    dynamic_tls = init_value();
    
    /* Initialize common TLS based on seed */
    common_tls = seed * 2;
    
    /* Access external TLS (simulating cross-file access) */
    external_tls = seed + 1000;
    external_weak_tls = seed + 2000;
    
    /* Manipulate TLS variables in loops */
    for (int i = 0; i < (seed % 5) + 1; i++) {
        public_tls += i;
        static_tls -= i;
        
        /* Use inline function with TLS */
        int inline_result = inline_tls_access(i);
        use_int(inline_result);
        
        /* Access volatile TLS */
        volatile_tls = volatile_tls + 1;
    }
    
    /* Complex expression with TLS address-taking */
    int *tls_ptr_array[5];
    tls_ptr_array[0] = &public_tls;
    tls_ptr_array[1] = &hidden_tls;
    tls_ptr_array[2] = &protected_tls;
    tls_ptr_array[3] = &static_tls;
    tls_ptr_array[4] = &dynamic_tls;
    
    /* Use TLS pointers in opaque way */
    for (int i = 0; i < 5; i++) {
        use_ptr(tls_ptr_array[i]);
        /* Modify through pointer */
        *(tls_ptr_array[i]) += (seed >> (i * 2)) & 0x3;
    }
    
    /* Force TLS emulation scenarios */
    manipulate_tls_pointers(seed);
    escape_tls_address();
    
    /* Conditional TLS access based on runtime values */
    switch (seed % 4) {
        case 0:
            aligned_tls = seed * 3;
            break;
        case 1:
            aligned_tls = seed / 2;
            break;
        case 2:
            aligned_tls = -seed;
            break;
        default:
            aligned_tls = aligned_tls ^ seed;
    }
    
    /* Compute checksum of all TLS values to prevent elimination */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += imported_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += preserve_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    checksum += aligned_tls;
    checksum += external_tls;
    checksum += external_weak_tls;
    
    /* Add inline TLS value */
    checksum += inline_tls_access(0);
    
    printf("TLS checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* External TLS definitions (simulating another source file) */
__thread int external_tls = 1500;
__thread int external_weak_tls __attribute__((weak)) = 2500;
