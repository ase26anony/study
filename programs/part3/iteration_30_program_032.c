/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios in GCC,
 * specifically targeting the declaration attribute copying logic in
 * tree-emutls.cc lines 295-304.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int i);
extern int get_random_value(void);

/* ============================================
   TLS VARIABLES WITH DIVERSE ATTRIBUTES
   ============================================ */

/* Public TLS with external linkage and default visibility */
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
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with attribute if supported */
__thread int imported_tls __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int common_tls;

/* Static TLS (file scope, internal linkage) */
static __thread int static_tls = 999;

/* Volatile TLS to prevent optimization */
__thread volatile int volatile_tls = 1234;

/* TLS with dynamic initialization */
__thread int dynamic_tls = 0;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 777;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* ============================================
   HELPER FUNCTIONS
   ============================================ */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 0;
    inline_tls += idx;
    return inline_tls;
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
    ptr_array[i++] = (void*)&imported_tls;
    ptr_array[i++] = (void*)&common_tls;
    ptr_array[i++] = (void*)&static_tls;
    ptr_array[i++] = (void*)&volatile_tls;
    ptr_array[i++] = (void*)&dynamic_tls;
    ptr_array[i++] = (void*)&aligned_tls;
    ptr_array[i++] = (void*)&external_tls;
    ptr_array[i++] = (void*)&external_weak_tls;
    
    /* Use the pointers in opaque calls */
    for (int j = 0; j < i; j++) {
        use_ptr((void*)ptr_array[j]);
    }
    
    /* Mix with non-TLS data */
    int local_data[10];
    for (int j = 0; j < 10; j++) {
        local_data[j] = seed + j;
        if (j % 3 == 0) {
            public_tls += local_data[j];
        } else if (j % 3 == 1) {
            hidden_tls -= local_data[j];
        } else {
            protected_tls ^= local_data[j];
        }
    }
}

/* Complex expression with TLS address-taking */
static int complex_tls_expression(int x) {
    /* Taking address in a way that might require proxy creation */
    int * volatile ptr1 = &public_tls;
    int * volatile ptr2 = &hidden_tls;
    
    /* Use TLS addresses in computation */
    int result = (*ptr1 * x) + (*ptr2 / (x ? x : 1));
    
    /* Chain of operations that might trigger declaration cloning */
    result += inline_tls_access(x);
    
    /* Use TLS in loop with address taken */
    for (int i = 0; i < x % 5; i++) {
        volatile int *ptr = (i % 2) ? &static_tls : &volatile_tls;
        result += *ptr + i;
    }
    
    return result;
}

/* Function that uses TLS in switch statement */
static void tls_switch_case(int value) {
    switch (value % 4) {
        case 0:
            public_tls += value;
            /* Take address in case block */
            use_ptr(&public_tls);
            break;
        case 1:
            hidden_tls -= value;
            use_ptr(&hidden_tls);
            break;
        case 2:
            protected_tls *= value;
            use_ptr(&protected_tls);
            break;
        case 3:
            internal_tls ^= value;
            use_ptr(&internal_tls);
            break;
    }
}

/* ============================================
   MAIN FUNCTION
   ============================================ */

int main(int argc, char **argv) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    /* Initialize dynamic TLS with function call */
    dynamic_tls = get_random_value() + seed;
    
    /* Initialize common TLS */
    common_tls = seed * 2;
    
    /* Access all TLS variables to ensure they're used */
    public_tls += seed;
    weak_tls -= seed;
    hidden_tls |= seed;
    protected_tls &= ~seed;
    internal_tls ^= seed;
    static_tls = seed % 100;
    volatile_tls = seed * 3;
    aligned_tls = seed << 2;
    
    /* Simulate external TLS access */
    external_tls = seed + 1000;
    external_weak_tls = seed + 2000;
    
    /* Manipulate TLS pointers */
    manipulate_tls_pointers(seed);
    
    /* Use TLS in complex expressions */
    int complex_result = 0;
    for (int i = 0; i < (seed % 10) + 5; i++) {
        complex_result += complex_tls_expression(i + seed);
        tls_switch_case(i + complex_result);
    }
    
    /* Use TLS in nested loops with volatile accesses */
    volatile int counter = 0;
    for (int i = 0; i < (seed % 8) + 2; i++) {
        for (int j = 0; j < (seed % 6) + 3; j++) {
            if ((i + j) % 2) {
                public_tls += counter;
                use_int(public_tls);
            } else {
                hidden_tls -= counter;
                use_int(hidden_tls);
            }
            counter++;
            
            /* Access TLS through volatile pointer */
            volatile int *volatile ptr = (i % 3) ? &volatile_tls : &static_tls;
            *ptr += j;
        }
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
    checksum += dynamic_tls;
    checksum += aligned_tls;
    checksum += external_tls;
    checksum += external_weak_tls;
    checksum += complex_result;
    
    /* Use inline function TLS */
    checksum += inline_tls_access(seed);
    
    printf("TLS checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* ============================================
   EXTERNAL TLS DEFINITIONS
   (Simulating another compilation unit)
   ============================================ */

/* In a real multi-file test, these would be in a separate source file */
__thread int external_tls = 5555;
__thread int external_weak_tls __attribute__((weak)) = 6666;

/* ============================================
   OPAQUE FUNCTION STUBS
   (For actual compilation and execution)
   ============================================ */

#ifdef BUILD_AS_STANDALONE
void use_ptr(void *p) {
    /* Prevent optimization but don't actually do anything */
    static volatile int sink;
    sink = (int)(long)p;
}

void use_int(int i) {
    static volatile int sink;
    sink = i;
}

int get_random_value(void) {
    return 42; /* Deterministic for testing */
}
#endif
