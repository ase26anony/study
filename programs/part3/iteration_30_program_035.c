/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int i);
extern int get_random(void);
extern void escape(void *p);

/* Global seed for unpredictable control flow */
static volatile int global_seed = 0;

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage and default visibility */
__thread int tls_public = 42;
extern __thread int tls_extern;  /* Will be defined in another "compilation unit" */

/* Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int tls_internal __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int tls_common;  /* No initializer for common symbol */

/* Static TLS with preservation requirement (address escapes) */
static __thread int tls_static_preserve = 500;

/* TLS with dynamic initialization */
extern int compute_init(void);
__thread int tls_dynamic = 0;  /* Will be initialized at runtime */

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64))) = 600;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 700;

/* TLS array */
__thread int tls_array[10];

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function that accesses TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Mix different TLS accesses */
    int result = tls_public + tls_hidden;
    
    /* Take address of TLS variable - forces emulation machinery */
    int *ptr = &tls_array[idx];
    escape(ptr);
    
    /* Use TLS in complex expression */
    result += tls_weak * (tls_internal > 0 ? 1 : -1);
    
    return result;
}

/* Function that takes address of TLS and uses it in ways that might require emulation */
static void manipulate_tls_pointers(int seed) {
    /* Array of pointers to TLS variables - their addresses escape */
    void *tls_pointers[20];
    volatile void *volatile_ptr;  /* Volatile to prevent optimization */
    
    /* Store addresses of various TLS variables */
    tls_pointers[0] = (void*)&tls_public;
    tls_pointers[1] = (void*)&tls_extern;
    tls_pointers[2] = (void*)&tls_weak;
    tls_pointers[3] = (void*)&tls_hidden;
    tls_pointers[4] = (void*)&tls_internal;
    tls_pointers[5] = (void*)&tls_protected;
    tls_pointers[6] = (void*)&tls_dllimport;
    tls_pointers[7] = (void*)&tls_common;
    tls_pointers[8] = (void*)&tls_static_preserve;
    tls_pointers[9] = (void*)&tls_dynamic;
    tls_pointers[10] = (void*)&tls_aligned;
    tls_pointers[11] = (void*)&tls_volatile;
    tls_pointers[12] = (void*)tls_array;
    
    /* Pass pointers to opaque function to prevent optimization */
    for (int i = 0; i < 13; i++) {
        use_ptr(tls_pointers[i]);
    }
    
    /* Volatile access to force compiler to keep TLS machinery */
    volatile_ptr = tls_pointers[seed % 13];
    escape((void*)volatile_ptr);
}

/* Function that uses TLS in loops and conditionals */
static int compute_tls_checksum(int iterations) {
    int checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Access TLS variables in loop - prevents dead code elimination */
        checksum += tls_public;
        checksum -= tls_hidden;
        checksum ^= tls_internal;
        
        /* Use inline function that accesses TLS */
        checksum += inline_tls_access(i % 10);
        
        /* Modify TLS based on loop index */
        tls_array[i % 10] = i * checksum;
        
        /* Conditional TLS access */
        if (checksum & 1) {
            checksum += tls_weak;
        } else {
            checksum += tls_protected;
        }
        
        /* Volatile TLS access */
        checksum += tls_volatile;
    }
    
    return checksum;
}

/* Function that simulates complex TLS initialization */
static void initialize_tls_variables(int seed) {
    /* Dynamic initialization */
    tls_dynamic = seed * 2;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = seed + i * 7;
    }
    
    /* Modify TLS variables based on seed */
    tls_public += seed;
    tls_hidden -= seed;
    tls_internal ^= seed;
    tls_protected |= seed;
    
    /* Common symbol initialization */
    tls_common = seed * 3;
    
    /* Aligned TLS access */
    tls_aligned = seed * 4;
    
    /* Volatile TLS modification */
    tls_volatile = seed * 5;
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char **argv) {
    int seed = 0;
    int checksum = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    } else {
        seed = 12345;
    }
    
    global_seed = seed;
    
    /* Phase 1: Initialize TLS variables */
    initialize_tls_variables(seed);
    
    /* Phase 2: Manipulate TLS pointers to force emulation */
    manipulate_tls_pointers(seed);
    
    /* Phase 3: Use TLS in complex computations */
    checksum = compute_tls_checksum(100 + (seed % 100));
    
    /* Phase 4: Additional TLS stress - nested loops with TLS access */
    {
        int temp = 0;
        for (int i = 0; i < 50; i++) {
            for (int j = 0; j < 20; j++) {
                /* Access different TLS variables based on loop indices */
                temp += tls_array[(i + j) % 10];
                temp -= tls_public * (j % 3);
                
                /* Take address of TLS in inner loop */
                if ((i * j) % 7 == 0) {
                    int *ptr = &tls_hidden;
                    escape(ptr);
                }
            }
            
            /* Call inline function with TLS access */
            temp += inline_tls_access(i % 10);
        }
        checksum += temp;
    }
    
    /* Phase 5: Final TLS computations and output */
    checksum += tls_weak;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_common;
    checksum += tls_dynamic;
    checksum += tls_aligned;
    
    /* Use volatile TLS in final computation */
    checksum += tls_volatile;
    
    /* Print checksum to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* ================= "EXTERNAL" COMPILATION UNIT SIMULATION ================= */

/* In a real multi-file test, this would be in a separate source file */
#ifdef SEPARATE_COMPILATION_UNIT
/* Definition of the extern TLS variable */
__thread int tls_extern = 999;

/* Additional TLS variables with different attributes */
__thread int tls_another_weak __attribute__((weak));
__thread int tls_another_hidden __attribute__((visibility("hidden"))) = 1111;

/* Opaque function definitions (stubs for linking) */
void use_ptr(void *p) {
    /* Empty - just to prevent optimization */
    (void)p;
}

void use_int(int i) {
    /* Empty - just to prevent optimization */
    (void)i;
}

int get_random(void) {
    return 42; /* Deterministic for testing */
}

void escape(void *p) {
    /* Empty - forces compiler to keep variable alive */
    (void)p;
}
#endif
