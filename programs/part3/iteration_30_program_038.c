/* test_tls_emulation.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random_value(void);

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage */
__thread int tls_public = 42;
__thread int tls_public_uninit;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 100;

/* Hidden visibility TLS */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 200;

/* Internal visibility TLS */
__attribute__((visibility("internal"))) __thread int tls_internal = 300;

/* Protected visibility TLS */
__attribute__((visibility("protected"))) __thread int tls_protected = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute if supported */
__attribute__((dllimport)) __thread int tls_dllimport;
#endif

/* TLS with complex initialization */
__thread int tls_dynamic_init = 0; /* Will be initialized in main */

/* Static TLS (not public) */
static __thread int tls_static = 999;

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64))) = 777;

/* TLS that might become common symbol */
__thread int tls_common; /* Tentative definition */

/* TLS used in asm (forces DECL_PRESERVE_P) */
register __thread int tls_asm_register asm("ebx"); /* Suggest register */

/* External TLS declarations (simulating other compilation units) */
extern __thread int tls_external;
extern __thread int tls_external_hidden __attribute__((visibility("hidden")));

/* ================= HELPER FUNCTIONS ================= */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int modify_tls_inline(int idx) {
    /* Access different TLS variables based on index */
    switch (idx % 4) {
        case 0: tls_static += 1; return tls_static;
        case 1: tls_hidden -= 1; return tls_hidden;
        case 2: tls_internal *= 2; return tls_internal;
        case 3: tls_protected /= 2; return tls_protected;
        default: return 0;
    }
}

/* Function that takes address of TLS - forces address computation */
static void take_tls_addresses(void) {
    volatile void *addrs[12];
    
    /* Take addresses of various TLS variables */
    addrs[0] = (void*)&tls_public;
    addrs[1] = (void*)&tls_weak;
    addrs[2] = (void*)&tls_hidden;
    addrs[3] = (void*)&tls_internal;
    addrs[4] = (void*)&tls_protected;
    addrs[5] = (void*)&tls_dllimport;
    addrs[6] = (void*)&tls_dynamic_init;
    addrs[7] = (void*)&tls_static;
    addrs[8] = (void*)&tls_aligned;
    addrs[9] = (void*)&tls_common;
    addrs[10] = (void*)&tls_asm_register;
    addrs[11] = (void*)&tls_external;
    
    /* Pass addresses to opaque function */
    for (int i = 0; i < 12; i++) {
        use_ptr((void*)addrs[i]);
    }
}

/* Function using TLS in complex expressions */
static int compute_with_tls(int seed) {
    int result = 0;
    
    /* Mix TLS and non-TLS computations */
    result += tls_public * seed;
    result -= tls_hidden / (seed + 1);
    result ^= tls_internal;
    result |= tls_protected;
    
    /* Use inline function */
    result += modify_tls_inline(seed);
    
    /* Volatile access to prevent optimization */
    volatile int temp = tls_static;
    result += temp;
    
    return result;
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]) % 256;
    } else {
        seed = 123; /* Default seed */
    }
    
    /* Dynamic initialization of TLS */
    tls_dynamic_init = seed * 3;
    
    /* Initialize TLS that might be common */
    tls_common = seed + 100;
    
    /* Initialize register TLS if not already */
    tls_asm_register = seed;
    
    /* Array of volatile pointers to TLS variables */
    volatile int * volatile tls_ptrs[10];
    tls_ptrs[0] = &tls_public;
    tls_ptrs[1] = &tls_weak;
    tls_ptrs[2] = &tls_hidden;
    tls_ptrs[3] = &tls_internal;
    tls_ptrs[4] = &tls_protected;
    tls_ptrs[5] = &tls_dynamic_init;
    tls_ptrs[6] = &tls_static;
    tls_ptrs[7] = &tls_aligned;
    tls_ptrs[8] = &tls_common;
    tls_ptrs[9] = &tls_asm_register;
    
    /* Complex loop with TLS accesses */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        /* Unpredictable index based on seed and iteration */
        int idx = (seed + i * 7) % 10;
        
        /* Volatile access through pointer array */
        volatile int *ptr = tls_ptrs[idx];
        int val = *ptr;
        
        /* Modify based on index */
        switch (idx) {
            case 0: *ptr = val + i; break;
            case 1: *ptr = val - i; break;
            case 2: *ptr = val ^ i; break;
            case 3: *ptr = val | i; break;
            case 4: *ptr = val & ~i; break;
            case 5: *ptr = val * (i + 1); break;
            case 6: *ptr = val / ((i % 5) + 1); break;
            case 7: *ptr = val << (i % 4); break;
            case 8: *ptr = val >> (i % 3); break;
            case 9: *ptr = val + seed; break;
        }
        
        /* Call opaque function */
        use_int(val);
        
        /* Update checksum */
        checksum += val;
        
        /* Use inline function periodically */
        if (i % 7 == 0) {
            checksum += modify_tls_inline(i);
        }
    }
    
    /* Take addresses of all TLS variables */
    take_tls_addresses();
    
    /* Compute final checksum using all TLS variables */
    checksum += compute_with_tls(seed);
    
    /* Additional unpredictable access pattern */
    if (seed % 3 == 0) {
        tls_public += tls_hidden;
        tls_internal -= tls_protected;
    } else if (seed % 3 == 1) {
        tls_hidden *= tls_static;
        tls_protected ^= tls_aligned;
    } else {
        tls_static = tls_common + tls_dynamic_init;
    }
    
    /* Final checksum computation */
    checksum += tls_public;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_dynamic_init;
    checksum += tls_static;
    checksum += tls_aligned;
    checksum += tls_common;
    checksum += tls_asm_register;
    
    /* Simulate external TLS access */
    checksum += tls_external;
    checksum += tls_external_hidden;
    
    printf("TLS checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* ================= STUB DEFINITIONS FOR LINKING ================= */

/* These would normally be in separate files, but included here for completeness */

/* External TLS definitions */
__thread int tls_external = 5555;
__attribute__((visibility("hidden"))) __thread int tls_external_hidden = 6666;

/* Opaque function stubs */
void use_ptr(void *p) {
    /* Empty stub - in real test this would be external */
    static volatile void *last_ptr;
    last_ptr = p;
}

void use_int(int x) {
    /* Empty stub - in real test this would be external */
    static volatile int last_val;
    last_val = x;
}
