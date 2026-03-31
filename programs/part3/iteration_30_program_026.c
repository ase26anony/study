/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and
 * force the compiler to copy declaration attributes between TLS
 * variables during emulation setup, specifically targeting the
 * uncovered lines in tree-emutls.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p1, void *p2);
extern int get_random_value(void);
extern void side_effect(void);

/* Force inlining of a function that accesses TLS */
static inline int inline_tls_access(int idx) __attribute__((always_inline));

/* TLS variables with diverse attributes */

/* Public TLS with external linkage */
__thread int public_tls = 42;
__thread int public_tls2 __attribute__((aligned(64)));

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
__declspec(dllimport) __thread int imported_tls;
#else
/* Use dllimport attribute if supported */
__thread int imported_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;

/* Static TLS (internal linkage) */
static __thread int static_tls = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* Dynamic initializer TLS */
__thread int dynamic_tls = 0;

/* TLS with preserve attribute (simulated via asm) */
__thread int preserved_tls __asm__("preserved_tls_symbol") = 700;

/* External TLS declarations (simulating other compilation units) */
extern __thread int external_tls;
extern __thread int external_tls2 __attribute__((weak));

/* Complex TLS with multiple attributes */
__thread int complex_tls __attribute__((weak, visibility("hidden"))) = 800;

/* Function that takes address of TLS variables - may force proxy creation */
static void take_tls_addresses(void) {
    void *addresses[20];
    volatile void *volatile_addresses[20];
    int i = 0;
    
    /* Take addresses of all TLS variables */
    addresses[i++] = &public_tls;
    addresses[i++] = &public_tls2;
    addresses[i++] = &weak_tls;
    addresses[i++] = &hidden_tls;
    addresses[i++] = &internal_tls;
    addresses[i++] = &protected_tls;
    addresses[i++] = &imported_tls;
    addresses[i++] = &common_tls;
    addresses[i++] = &static_tls;
    addresses[i++] = (void*)&volatile_tls;
    addresses[i++] = &dynamic_tls;
    addresses[i++] = &preserved_tls;
    addresses[i++] = &external_tls;
    addresses[i++] = &external_tls2;
    addresses[i++] = &complex_tls;
    
    /* Store in volatile array to prevent optimization */
    for (int j = 0; j < i; j++) {
        volatile_addresses[j] = addresses[j];
    }
    
    /* Pass addresses to opaque functions */
    for (int j = 0; j < i; j++) {
        use_ptr((void*)volatile_addresses[j]);
        use_ptr2((void*)volatile_addresses[j], 
                 (void*)volatile_addresses[(j + 1) % i]);
    }
}

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    int result = 0;
    
    /* Access different TLS variables based on index */
    switch (idx % 8) {
        case 0: result = public_tls; break;
        case 1: result = weak_tls; break;
        case 2: result = hidden_tls; break;
        case 3: result = static_tls; break;
        case 4: result = volatile_tls; break;
        case 5: result = common_tls; break;
        case 6: result = complex_tls; break;
        case 7: result = protected_tls; break;
    }
    
    /* Take address within inline function */
    void *addr;
    switch (idx % 5) {
        case 0: addr = &public_tls; break;
        case 1: addr = &weak_tls; break;
        case 2: addr = &hidden_tls; break;
        case 3: addr = &static_tls; break;
        case 4: addr = &complex_tls; break;
    }
    use_ptr(addr);
    
    return result;
}

/* Function with loop that uses TLS variables */
static void process_tls_in_loop(int iterations, int seed) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        int idx = (i + seed) % 12;
        
        /* Use inline function */
        sum += inline_tls_access(idx);
        
        /* Direct TLS accesses with complex expressions */
        switch (idx) {
            case 0:
                public_tls += i;
                break;
            case 1:
                weak_tls ^= (i * seed);
                break;
            case 2:
                hidden_tls = (hidden_tls * 1103515245 + 12345) & 0x7fffffff;
                break;
            case 3:
                static_tls -= seed;
                break;
            case 4:
                volatile_tls = i;
                break;
            case 5:
                common_tls = (common_tls + i) % 1000;
                break;
            case 6:
                complex_tls *= 3;
                break;
            case 7:
                protected_tls |= (1 << (i % 16));
                break;
            case 8:
                internal_tls = internal_tls / (seed + 1);
                break;
            case 9:
                public_tls2 = i * i;
                break;
            case 10:
                dynamic_tls = get_random_value();
                break;
            case 11:
                preserved_tls = ~preserved_tls;
                break;
        }
        
        /* Take address in loop */
        if (i % 3 == 0) {
            void *addr;
            switch (i % 4) {
                case 0: addr = &public_tls; break;
                case 1: addr = &weak_tls; break;
                case 2: addr = &static_tls; break;
                case 3: addr = &complex_tls; break;
            }
            use_ptr(addr);
        }
    }
    
    /* Prevent dead code elimination */
    if (sum != 0x12345678) {  /* Arbitrary constant */
        side_effect();
    }
}

/* Initialize dynamic TLS */
static void init_dynamic_tls(void) {
    dynamic_tls = get_random_value();
    common_tls = 999;
    
    /* Initialize external TLS (simulating definition elsewhere) */
    extern __thread int external_tls;
    extern __thread int external_tls2 __attribute__((weak));
    
    /* These would be defined in another compilation unit */
    /* For test purposes, we'll simulate them here */
    static __thread int external_tls_def = 1111;
    static __thread int external_tls2_def = 2222;
    
    /* Take addresses to force TLS machinery */
    use_ptr(&external_tls_def);
    use_ptr(&external_tls2_def);
}

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize TLS variables */
    init_dynamic_tls();
    
    /* Take addresses of all TLS variables early */
    take_tls_addresses();
    
    /* Process TLS in loops with runtime-dependent iterations */
    int iterations = 100 + (seed % 50);
    process_tls_in_loop(iterations, seed);
    
    /* Compute checksum of all TLS values to prevent removal */
    int checksum = 0;
    
    checksum += public_tls;
    checksum ^= weak_tls;
    checksum += hidden_tls;
    checksum ^= internal_tls;
    checksum += protected_tls;
    checksum ^= static_tls;
    checksum += volatile_tls;
    checksum ^= dynamic_tls;
    checksum += preserved_tls;
    checksum ^= common_tls;
    checksum += complex_tls;
    checksum ^= public_tls2;
    
    /* Use checksum to affect control flow */
    if (checksum % 7 == 0) {
        take_tls_addresses();
    }
    
    /* Print checksum to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional complex expression with TLS addresses */
    void *ptr_array[5];
    ptr_array[0] = &public_tls;
    ptr_array[1] = &weak_tls;
    ptr_array[2] = &hidden_tls;
    ptr_array[3] = &static_tls;
    ptr_array[4] = &complex_tls;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if ((seed + i + j) % 3 == 0) {
                use_ptr2(ptr_array[i], ptr_array[j]);
            }
        }
    }
    
    return checksum == 0 ? 0 : 1;
}

/* Stub definitions for opaque functions (for actual compilation) */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile int sink;
    sink = *(int*)p;
}

void use_ptr2(void *p1, void *p2) {
    static volatile int sink;
    sink = *(int*)p1 + *(int*)p2;
}

int get_random_value(void) {
    return rand();
}

void side_effect(void) {
    /* Do nothing but prevent optimization */
    static volatile int counter = 0;
    counter++;
}

/* Definitions for external TLS variables */
__thread int external_tls = 1111;
__thread int external_tls2 __attribute__((weak)) = 2222;
