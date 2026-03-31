/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and force
 * the compiler to copy declaration attributes between TLS variables,
 * specifically targeting the uncovered lines in tree-emutls.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p1, void *p2);
extern int opaque_func(void);
extern void escape_ptr(void **p);

/* Visibility attributes */
#define HIDDEN __attribute__((visibility("hidden")))
#define PROTECTED __attribute__((visibility("protected")))
#define DEFAULT __attribute__((visibility("default")))

/* Weak symbol attribute */
#define WEAK __attribute__((weak))

/* DLL import simulation for Windows targets */
#ifdef _WIN32
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLIMPORT __attribute__((dllimport))
#endif

/* ===================== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===================== */

/* Public TLS with external linkage and default visibility */
__thread int public_tls DEFAULT = 42;
extern __thread int external_public_tls;  /* Will be defined in another "unit" */

/* Weak TLS symbol */
__thread int weak_tls WEAK = 100;

/* Hidden TLS (static-like visibility) */
__thread int hidden_tls HIDDEN = 200;

/* Protected visibility TLS */
__thread int protected_tls PROTECTED = 300;

/* TLS with DLL import attribute (simulated) */
DLLIMPORT __thread int dllimport_tls = 400;

/* Common TLS (tentative definition) */
__thread int common_tls;  /* No initializer -> common symbol */

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 500;

/* TLS with dynamic initialization */
__thread int dynamic_tls = 0;

/* TLS variable defined inside a function (simulating static TLS in function scope) */
static void define_function_tls(void) {
    static __thread int function_scope_tls = 600;
    use_ptr(&function_scope_tls);
}

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls_unit2;
extern __thread int external_weak_tls WEAK;

/* ===================== HELPER FUNCTIONS ===================== */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 700;
    inline_tls += idx;
    return inline_tls;
}

/* Function that takes address of TLS and escapes it */
static void escape_tls_addresses(void) {
    void *ptrs[10];
    int i = 0;
    
    ptrs[i++] = (void *)&public_tls;
    ptrs[i++] = (void *)&hidden_tls;
    ptrs[i++] = (void *)&protected_tls;
    ptrs[i++] = (void *)&weak_tls;
    ptrs[i++] = (void *)&volatile_tls;
    ptrs[i++] = (void *)&common_tls;
    
    /* Escape all pointers to prevent optimization */
    for (int j = 0; j < i; j++) {
        escape_ptr(&ptrs[j]);
    }
}

/* Function using TLS in a loop - may trigger transformations */
static void loop_with_tls(int iterations) {
    static __thread int loop_tls = 0;
    
    for (int i = 0; i < iterations; i++) {
        loop_tls += i;
        /* Mix with other TLS variables */
        public_tls ^= loop_tls;
        hidden_tls += public_tls;
    }
    
    /* Take address and use */
    use_ptr(&loop_tls);
}

/* ===================== MAIN FUNCTION ===================== */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]) % 10;
    }
    
    /* Initialize dynamic TLS with opaque function call */
    dynamic_tls = opaque_func();
    
    /* Define the external TLS variable (simulating definition in this "unit") */
    __thread int external_public_tls = 999;
    
    /* Array of volatile pointers to TLS variables */
    volatile void *tls_pointers[20];
    int ptr_count = 0;
    
    /* Take addresses of all TLS variables - forces addressability */
    tls_pointers[ptr_count++] = (void *)&public_tls;
    tls_pointers[ptr_count++] = (void *)&external_public_tls;
    tls_pointers[ptr_count++] = (void *)&weak_tls;
    tls_pointers[ptr_count++] = (void *)&hidden_tls;
    tls_pointers[ptr_count++] = (void *)&protected_tls;
    tls_pointers[ptr_count++] = (void *)&dllimport_tls;
    tls_pointers[ptr_count++] = (void *)&common_tls;
    tls_pointers[ptr_count++] = (void *)&volatile_tls;
    tls_pointers[ptr_count++] = (void *)&dynamic_tls;
    
    /* Use TLS variables in complex expressions */
    int *aliased_tls = (int *)&public_tls;
    *aliased_tls += seed;
    
    /* Conditional based on seed */
    if (seed & 1) {
        hidden_tls *= 2;
    } else {
        protected_tls /= 2;
    }
    
    /* Call inline function multiple times */
    for (int i = 0; i < 5; i++) {
        int val = inline_tls_access(i + seed);
        volatile_tls ^= val;
    }
    
    /* Define TLS inside function scope */
    define_function_tls();
    
    /* Escape TLS addresses */
    escape_tls_addresses();
    
    /* Loop with TLS */
    loop_with_tls(10 + seed);
    
    /* Use opaque function with TLS addresses */
    for (int i = 0; i < ptr_count; i++) {
        use_ptr((void *)tls_pointers[i]);
    }
    
    /* Pairwise combinations */
    use_ptr2(&public_tls, &hidden_tls);
    use_ptr2(&weak_tls, &protected_tls);
    use_ptr2(&volatile_tls, &common_tls);
    
    /* Compute checksum of all TLS values (prevens dead code elimination) */
    unsigned int checksum = 0;
    checksum += public_tls;
    checksum += external_public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += dllimport_tls;
    checksum += common_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    
    /* Use checksum in a way that can't be optimized away */
    printf("TLS checksum: %u (seed: %d)\n", checksum, seed);
    
    /* Return checksum modulo 256 to prevent optimization */
    return checksum % 256;
}

/* ===================== STUB FUNCTIONS FOR COMPILATION ===================== */

/* These would be defined elsewhere in a real test environment */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_ptr2(void *p1, void *p2) {
    static volatile void *last_p1 = NULL;
    static volatile void *last_p2 = NULL;
    last_p1 = p1;
    last_p2 = p2;
}

int opaque_func(void) {
    return rand() % 1000;
}

void escape_ptr(void **p) {
    static volatile void *escaped_ptr = NULL;
    escaped_ptr = *p;
}

/* External TLS definitions (simulating another compilation unit) */
__thread int external_tls_unit2 = 1234;
__thread int external_weak_tls WEAK = 5678;
