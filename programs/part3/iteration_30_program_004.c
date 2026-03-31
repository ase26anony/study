/* test-tls-emulation-attributes.c
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
extern void use_int(int i);
extern int get_random_value(void);

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage, default visibility */
__thread int public_tls = 42;
extern __thread int external_public_tls;  /* Will be defined below */

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows-targeting compilations) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Use dllimport attribute if supported, else simulate */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) - may become DECL_COMMON */
__thread int common_tls;  /* No initializer */

/* Static TLS (not public) with complex initialization */
static __thread int static_tls = 0;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 999;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 777;

/* External declarations (simulating other compilation units) */
extern __thread int external_tls_unit2;
extern __thread int external_tls_unit3 __attribute__((weak));

/* ================= FUNCTION DEFINITIONS ================= */

/* Inline function that accesses TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch (idx % 5) {
        case 0: return public_tls;
        case 1: return hidden_tls;
        case 2: return static_tls;
        case 3: return volatile_tls;
        case 4: return aligned_tls;
        default: return 0;
    }
}

/* Function that takes address of TLS and uses it in ways that might
 * force emulation structure creation */
static void manipulate_tls_addresses(int seed) {
    /* Array of volatile pointers to prevent optimization */
    volatile void *addr_array[20];
    int idx = 0;
    
    /* Take addresses of all TLS variables */
    addr_array[idx++] = (void*)&public_tls;
    addr_array[idx++] = (void*)&weak_tls;
    addr_array[idx++] = (void*)&hidden_tls;
    addr_array[idx++] = (void*)&internal_tls;
    addr_array[idx++] = (void*)&protected_tls;
    addr_array[idx++] = (void*)&dllimport_tls;
    addr_array[idx++] = (void*)&common_tls;
    addr_array[idx++] = (void*)&static_tls;
    addr_array[idx++] = (void*)&volatile_tls;
    addr_array[idx++] = (void*)&aligned_tls;
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < idx; i++) {
        use_ptr((void*)addr_array[i]);
    }
    
    /* Use seed to modify TLS variables unpredictably */
    for (int i = 0; i < 100; i++) {
        int var_idx = (seed + i) % 10;
        
        /* Force compiler to generate different access patterns */
        switch (var_idx) {
            case 0:
                public_tls += i;
                break;
            case 1:
                /* Cast away volatile for assignment */
                *(int*)&volatile_tls += i * 2;
                break;
            case 2:
                hidden_tls ^= seed;
                break;
            case 3:
                static_tls = inline_tls_access(i);
                break;
            case 4:
                aligned_tls = (aligned_tls << 3) | (seed & 0x7);
                break;
            case 5:
                /* Access through pointer */
                int *ptr = &common_tls;
                *ptr += i;
                break;
            case 6:
                internal_tls -= seed;
                break;
            case 7:
                protected_tls = protected_tls * 3 + i;
                break;
            case 8:
                /* Simulate external TLS access */
                if (&external_tls_unit2 != NULL) {
                    use_int(external_tls_unit2);
                }
                break;
            case 9:
                /* Use weak TLS */
                if (&weak_tls != NULL) {
                    weak_tls = (weak_tls & 0xFF) | (seed << 8);
                }
                break;
        }
    }
}

/* Function with loop that uses TLS - may trigger optimizations that copy declarations */
static void tls_loop_operations(int iterations, int seed) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        int val = inline_tls_access(i + seed);
        
        /* Complex expression with TLS address */
        if (i % 3 == 0) {
            use_ptr(&public_tls);
            public_tls += val;
        }
        
        if (i % 5 == 0) {
            /* Force address computation */
            int *tls_ptr = (i % 2) ? &hidden_tls : &static_tls;
            *tls_ptr += i;
            use_ptr(tls_ptr);
        }
        
        /* Use runtime value to decide which TLS to access */
        int tls_choice = (seed + i) % 4;
        switch (tls_choice) {
            case 0: sum += public_tls; break;
            case 1: sum += hidden_tls; break;
            case 2: sum += static_tls; break;
            case 3: sum += aligned_tls; break;
        }
    }
    
    /* Prevent dead code elimination */
    use_int(sum);
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char **argv) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = get_random_value();
    }
    
    /* Initialize some TLS variables with runtime values */
    common_tls = seed * 2;
    static_tls = seed % 100;
    
    /* Dynamic initialization of TLS (might require emulation) */
    volatile_tls = get_random_value();
    
    /* Take addresses early to force TLS structure setup */
    void *tls_addresses[5];
    tls_addresses[0] = &public_tls;
    tls_addresses[1] = &hidden_tls;
    tls_addresses[2] = &internal_tls;
    tls_addresses[3] = &protected_tls;
    tls_addresses[4] = &aligned_tls;
    
    for (int i = 0; i < 5; i++) {
        use_ptr(tls_addresses[i]);
    }
    
    /* Perform operations that stress TLS emulation */
    manipulate_tls_addresses(seed);
    tls_loop_operations(100 + (seed % 50), seed);
    
    /* Compute checksum of all TLS values to prevent optimization removal */
    int checksum = 0;
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += internal_tls;
    checksum += protected_tls;
    checksum += dllimport_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += aligned_tls;
    
    /* Use checksum so it can't be optimized away */
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional stress: nested loops with TLS access */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = (i * 10 + j + seed) % 8;
            switch (idx) {
                case 0: public_tls ^= j; break;
                case 1: hidden_tls += i; break;
                case 2: static_tls -= j; break;
                case 3: aligned_tls |= i; break;
                case 4: common_tls = common_tls * 3 + j; break;
                case 5: internal_tls &= ~(1 << (i % 8)); break;
                case 6: protected_tls = protected_tls / 2 + i; break;
                case 7: volatile_tls = (volatile_tls << 1) | (j & 1); break;
            }
        }
    }
    
    /* Final checksum */
    int final_checksum = 
        public_tls + weak_tls + hidden_tls + internal_tls + 
        protected_tls + dllimport_tls + common_tls + 
        static_tls + volatile_tls + aligned_tls;
    
    printf("Final TLS checksum: %d\n", final_checksum);
    
    return final_checksum % 256;
}

/* ================= TLS DEFINITIONS ================= */

/* Define external TLS variables */
__thread int external_public_tls = 1234;
__thread int external_tls_unit2 = 5678;
__thread int external_tls_unit3 __attribute__((weak)) = 9999;

/* ================= STUB FUNCTIONS ================= */

/* These would be provided in a separate file in a real test,
   but included here for completeness */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_int(int i) {
    /* Prevent optimization */
    static volatile int last_int = 0;
    last_int = i;
}

int get_random_value(void) {
    /* Simple pseudo-random for testing */
    static int counter = 0;
    return (counter++ * 1103515245 + 12345) & 0x7FFFFFFF;
}
