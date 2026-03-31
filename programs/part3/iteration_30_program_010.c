/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);

/* TLS variables with diverse attributes */

/* Public TLS with external linkage */
__thread int tls_public = 42;
__thread int tls_public_uninit;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 100;

/* Hidden visibility TLS */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 200;

/* Protected visibility TLS */
__attribute__((visibility("protected"))) __thread int tls_protected = 300;

/* Internal visibility TLS */
__attribute__((visibility("internal"))) __thread int tls_internal = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute if supported */
__attribute__((dllimport)) __thread int tls_dllimport_sim;
#endif

/* Common TLS (tentative definition) */
__thread int tls_common;

/* Static TLS (internal linkage) */
static __thread int tls_static = 999;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 555;

/* TLS with complex initializer */
extern int compute_init(void);
__thread int tls_complex = 0;

/* External TLS declarations (simulating other compilation units) */
extern __thread int tls_extern_defined;
extern __thread int tls_extern_weak __attribute__((weak));

/* Function that uses TLS and should be inlined */
static inline int process_tls_inline(int idx) {
    /* Access various TLS variables */
    int sum = tls_static + tls_public;
    
    /* Take address of TLS variable - may trigger proxy creation */
    int *ptr = &tls_hidden;
    use_ptr(ptr);
    
    /* Conditional TLS access */
    if (idx % 2) {
        sum += tls_protected;
    } else {
        sum += tls_internal;
    }
    
    return sum;
}

/* Function that takes address of TLS variables */
static void take_tls_addresses(void) {
    /* Array of pointers to TLS variables */
    volatile void *tls_pointers[20];
    int i = 0;
    
    /* Take addresses of all TLS variables */
    tls_pointers[i++] = (void*)&tls_public;
    tls_pointers[i++] = (void*)&tls_public_uninit;
    tls_pointers[i++] = (void*)&tls_weak;
    tls_pointers[i++] = (void*)&tls_hidden;
    tls_pointers[i++] = (void*)&tls_protected;
    tls_pointers[i++] = (void*)&tls_internal;
    tls_pointers[i++] = (void*)&tls_common;
    tls_pointers[i++] = (void*)&tls_static;
    tls_pointers[i++] = (void*)&tls_volatile;
    tls_pointers[i++] = (void*)&tls_complex;
    tls_pointers[i++] = (void*)&tls_extern_defined;
    tls_pointers[i++] = (void*)&tls_extern_weak;
    
    /* Pass all addresses to opaque function */
    for (int j = 0; j < i; j++) {
        use_ptr((void*)tls_pointers[j]);
    }
}

/* Complex TLS usage in loops */
static int process_tls_loop(int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        int temp = tls_public + i;
        
        /* Conditional TLS access based on loop index */
        if (i % 3 == 0) {
            temp += tls_hidden;
        } else if (i % 3 == 1) {
            temp += tls_protected;
        } else {
            temp += tls_internal;
        }
        
        /* Use inline function with TLS access */
        temp += process_tls_inline(i);
        
        /* Volatile TLS access */
        temp += tls_volatile;
        
        result += temp;
    }
    
    return result;
}

/* Initialize TLS with runtime values */
static void init_tls_vars(int seed) {
    tls_public = seed;
    tls_public_uninit = seed * 2;
    tls_weak = seed * 3;
    tls_hidden = seed * 4;
    tls_protected = seed * 5;
    tls_internal = seed * 6;
    tls_common = seed * 7;
    tls_static = seed * 8;
    tls_volatile = seed * 9;
    tls_complex = seed * 10;
}

/* Compute checksum of all TLS variables */
static int tls_checksum(void) {
    int sum = 0;
    
    sum += tls_public;
    sum += tls_public_uninit;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_common;
    sum += tls_static;
    sum += tls_volatile;
    sum += tls_complex;
    
    /* Access external TLS variables if available */
    if (&tls_extern_defined) {
        sum += tls_extern_defined;
    }
    
    if (&tls_extern_weak) {
        sum += tls_extern_weak;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int seed = 1;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]) % 100;
    }
    
    /* Initialize TLS variables */
    init_tls_vars(seed);
    
    /* Force address-taking of TLS variables */
    take_tls_addresses();
    
    /* Complex TLS usage in loops */
    int loop_result = process_tls_loop(seed + 10);
    use_int(loop_result);
    
    /* More TLS operations with runtime decisions */
    for (int i = 0; i < seed % 5 + 1; i++) {
        /* Switch between different TLS variables */
        switch (i % 4) {
            case 0:
                tls_public += get_random() % 10;
                break;
            case 1:
                tls_hidden -= get_random() % 10;
                break;
            case 2:
                tls_protected *= (get_random() % 3) + 1;
                break;
            case 3:
                tls_internal = get_random() % 100;
                break;
        }
        
        /* Inline function with TLS access */
        int inline_result = process_tls_inline(i);
        use_int(inline_result);
    }
    
    /* Final checksum to prevent optimization */
    int checksum = tls_checksum();
    printf("TLS checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* External TLS definitions (would be in separate file normally) */
__thread int tls_extern_defined = 12345;
__thread int tls_extern_weak = 67890;

/* Opaque function stubs for linking */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_int(int x) {
    /* Prevent optimization */
    static volatile int last_val = 0;
    last_val = x;
}

int get_random(void) {
    /* Simple pseudo-random based on time */
    static int counter = 0;
    return (counter++ * 1103515245 + 12345) & 0x7FFFFFFF;
}

int compute_init(void) {
    return 42;
}
