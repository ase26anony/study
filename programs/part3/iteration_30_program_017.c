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
extern int get_random_value(void);
extern void side_effect(void);

/* ==================== TLS VARIABLES WITH DIVERSE ATTRIBUTES ==================== */

/* Public TLS with external linkage, default visibility */
__thread int public_tls = 42;
extern __thread int external_public_tls;  /* Will be defined in another "unit" */

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Use dllimport-like attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Static TLS (not public) with complex initializer */
static __thread int static_tls = 0;

/* TLS with volatile to prevent optimization */
volatile __thread int volatile_tls = 999;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 777;

/* Common TLS (tentative definition) */
__thread int common_tls;  /* No initializer - should become common */

/* TLS that will have its address taken and escape */
__thread int escaping_tls = 1234;

/* TLS used in inline function */
__thread int inline_used_tls = 555;

/* ==================== FUNCTION DECLARATIONS ==================== */

/* Inline function that accesses TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access TLS in a way that might require emulation structure */
    int val = inline_used_tls + idx;
    /* Take address to force TLS machinery */
    use_ptr(&inline_used_tls);
    return val;
}

/* Function that takes address of TLS and passes it around */
static void manipulate_tls_pointers(int seed) {
    void *ptrs[20];
    volatile void *volatile_ptr;  /* volatile to prevent optimization */
    
    /* Store addresses of various TLS variables */
    ptrs[0] = &public_tls;
    ptrs[1] = &weak_tls;
    ptrs[2] = &hidden_tls;
    ptrs[3] = &internal_tls;
    ptrs[4] = &protected_tls;
    ptrs[5] = (void *)&dllimport_tls;
    ptrs[6] = &static_tls;
    ptrs[7] = (void *)&volatile_tls;
    ptrs[8] = &aligned_tls;
    ptrs[9] = &common_tls;
    ptrs[10] = &escaping_tls;
    
    /* Use seed to make access pattern unpredictable */
    for (int i = 0; i < 11; i++) {
        if ((seed >> i) & 1) {
            use_ptr(ptrs[i]);
            volatile_ptr = ptrs[i];
            side_effect();
        }
    }
    
    /* Mix TLS and non-TLS addresses */
    int local_var = seed;
    use_ptr2(&inline_used_tls, &local_var);
}

/* Function with loop that uses TLS variables */
static void loop_with_tls(int iterations, int seed) {
    /* Dynamic initialization based on runtime value */
    static_tls = seed % 100;
    
    for (int i = 0; i < iterations; i++) {
        /* Access multiple TLS variables in loop */
        public_tls += i;
        hidden_tls ^= seed;
        internal_tls *= (i % 5) + 1;
        
        /* Use inline function with TLS access */
        int val = inline_tls_access(i);
        
        /* Volatile access to prevent optimization */
        volatile_tls = val;
        
        /* Conditional based on TLS values */
        if (public_tls > 1000) {
            protected_tls -= public_tls / 10;
        }
        
        /* Take address inside loop */
        if (i % 3 == 0) {
            use_ptr(&escaping_tls);
            escaping_tls++;
        }
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv to get seed value - makes execution unpredictable */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = get_random_value();
    }
    
    /* Initialize some TLS variables with runtime values */
    common_tls = seed * 2;
    aligned_tls = seed * 3;
    
    /* Manipulate TLS pointers - forces address taking */
    manipulate_tls_pointers(seed);
    
    /* Complex loop with TLS access */
    loop_with_tls(100 + (seed % 50), seed);
    
    /* Additional unpredictable access pattern */
    int *tls_array[] = {
        &public_tls,
        &weak_tls,
        &hidden_tls,
        &internal_tls,
        &protected_tls,
        &static_tls,
        &common_tls,
        &escaping_tls,
        &inline_used_tls
    };
    
    for (int i = 0; i < 9; i++) {
        if ((seed >> (i % 16)) & 1) {
            *tls_array[i] += i;
            use_ptr(tls_array[i]);
        }
    }
    
    /* Compute checksum of all TLS values to prevent removal */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += internal_tls;
    checksum += protected_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += aligned_tls;
    checksum += common_tls;
    checksum += escaping_tls;
    checksum += inline_used_tls;
    
    printf("TLS checksum: %d\n", checksum);
    
    /* Simulate external TLS variable access */
    external_public_tls = checksum % 1000;
    use_ptr(&external_public_tls);
    
    return checksum % 256;
}

/* ==================== "EXTERNAL" COMPILATION UNIT SIMULATION ==================== */

/* This simulates a variable defined in another source file */
__thread int external_public_tls = 9999;

/* Weak TLS definition that might be overridden */
__thread int weak_tls = 100;  /* Redefinition allowed because it's weak */

/* Opaque function stubs for linking */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    (void)p;
}

void use_ptr2(void *p1, void *p2) {
    (void)p1;
    (void)p2;
}

int get_random_value(void) {
    return 42;  /* Deterministic for testing */
}

void side_effect(void) {
    /* Empty side effect */
}
