/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and
 * specifically exercise the declaration attribute copying logic in
 * tree-emutls.cc (lines 295-304).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p1, void *p2);
extern int opaque_func(void);
extern void escape_ptr(void **ptr);

/* ===================== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===================== */

/* Public TLS with external linkage and default visibility */
__thread int public_tls = 42;
__thread int public_tls_uninit;

/* Weak TLS symbol */
__attribute__((weak)) __thread int weak_tls = 100;

/* Hidden visibility TLS */
__attribute__((visibility("hidden"))) __thread int hidden_tls = 200;

/* Protected visibility TLS */
__attribute__((visibility("protected"))) __thread int protected_tls = 300;

/* Internal visibility TLS */
__attribute__((visibility("internal"))) __thread int internal_tls = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Use dllimport attribute if supported, otherwise simulate */
__attribute__((dllimport)) __thread int dllimport_tls;
#endif

/* Common TLS symbol (tentative definition) */
__thread int common_tls;

/* Static TLS (file scope, internal linkage) */
static __thread int static_tls = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* TLS with dynamic initializer */
__thread int dynamic_tls = 0;

/* TLS with alignment requirement */
__attribute__((aligned(64))) __thread int aligned_tls = 700;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* ===================== FUNCTIONS THAT USE TLS ===================== */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 0;
    inline_tls += idx;
    return inline_tls + public_tls;
}

/* Function that takes address of TLS and escapes it */
static void escape_tls_addresses(void) {
    void *ptrs[10];
    int i = 0;
    
    ptrs[i++] = (void *)&public_tls;
    ptrs[i++] = (void *)&weak_tls;
    ptrs[i++] = (void *)&hidden_tls;
    ptrs[i++] = (void *)&protected_tls;
    ptrs[i++] = (void *)&internal_tls;
    ptrs[i++] = (void *)&static_tls;
    ptrs[i++] = (void *)&volatile_tls;
    ptrs[i++] = (void *)&aligned_tls;
    
    /* Escape all pointers to prevent optimization */
    for (int j = 0; j < i; j++) {
        use_ptr(ptrs[j]);
    }
}

/* Complex expression with TLS address computation */
static void complex_tls_expression(int seed) {
    /* Take address of TLS in a way that might require temporary */
    int * volatile ptr_array[5];
    
    ptr_array[0] = &public_tls;
    ptr_array[1] = &weak_tls + (seed % 2);
    ptr_array[2] = (int *)((char *)&hidden_tls + 1);
    ptr_array[3] = &static_tls;
    ptr_array[4] = &volatile_tls;
    
    for (int i = 0; i < 5; i++) {
        use_ptr(ptr_array[i]);
    }
    
    /* Mix TLS and non-TLS in expressions */
    int local = seed;
    public_tls = local + weak_tls;
    hidden_tls = public_tls * 2 - local;
    
    /* Use in conditional that depends on runtime value */
    if (seed & 1) {
        protected_tls += inline_tls_access(local);
    } else {
        internal_tls -= inline_tls_access(local);
    }
}

/* Function that forces TLS variable preservation */
static void preserve_tls_vars(void) {
    /* Use TLS in asm-like context by taking address and escaping */
    void *tls_addr = &public_tls;
    escape_ptr(&tls_addr);
    
    /* Volatile access ensures preservation */
    volatile int *vp = &volatile_tls;
    *vp = *vp + 1;
    
    /* Common symbol usage */
    common_tls = opaque_func();
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
    
    /* Initialize dynamic TLS with runtime value */
    dynamic_tls = seed;
    
    /* Initialize external TLS (if linked) */
    if (&external_tls != NULL) {
        external_tls = seed * 2;
    }
    
    /* Array of TLS pointers for indirection */
    int *tls_ptrs[] = {
        &public_tls,
        &weak_tls,
        &hidden_tls,
        &protected_tls,
        &internal_tls,
        &static_tls,
        &volatile_tls,
        &dynamic_tls,
        &aligned_tls,
        &common_tls
    };
    
    /* Access TLS through indirection to prevent optimization */
    for (int i = 0; i < 10; i++) {
        int idx = (seed + i) % 10;
        *tls_ptrs[idx] += i;
        
        /* Pass address to opaque function */
        use_ptr(tls_ptrs[idx]);
    }
    
    /* Force various TLS usage patterns */
    escape_tls_addresses();
    complex_tls_expression(seed);
    preserve_tls_vars();
    
    /* Use inline function with TLS */
    for (int i = 0; i < 5; i++) {
        static_tls += inline_tls_access(i + seed);
    }
    
    /* Mix TLS variables in loops with runtime-dependent bounds */
    int loop_bound = (seed % 10) + 5;
    for (int i = 0; i < loop_bound; i++) {
        if (i & 1) {
            public_tls += weak_tls;
        } else {
            hidden_tls -= protected_tls;
        }
        
        /* Take address in loop */
        use_ptr2(&internal_tls, &volatile_tls);
    }
    
    /* Compute checksum of all TLS values to prevent removal */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += protected_tls;
    checksum += internal_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    checksum += aligned_tls;
    checksum += common_tls;
    
    printf("TLS checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* ===================== EXTERNAL TLS DEFINITIONS ===================== */

/* Define the external TLS variables (simulating another source file) */
__thread int external_tls = 999;
__attribute__((weak)) __thread int external_weak_tls = 888;

/* Opaque function stubs (would be empty in actual test environment) */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr;
    last_ptr = p;
}

void use_ptr2(void *p1, void *p2) {
    static volatile void *last_ptr1, *last_ptr2;
    last_ptr1 = p1;
    last_ptr2 = p2;
}

int opaque_func(void) {
    return rand() % 100;
}

void escape_ptr(void **ptr) {
    static volatile void *escaped_ptr;
    escaped_ptr = *ptr;
}
