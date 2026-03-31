/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and
 * force the compiler to copy declaration attributes between TLS variables
 * during emulation setup, specifically targeting lines 295-304 in tree-emutls.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_ptr2(void *p1, void *p2);
extern int get_random_value(void);
extern void side_effect(void);

/* Force TLS emulation by using __thread extensively with varied attributes */

/* 1. TLS variables with different linkage and visibility */

/* Public TLS with default visibility */
__thread int public_tls = 42;
__thread int public_tls_uninit;

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
/* Use dllimport attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Static TLS (not public) - should have different TREE_PUBLIC value */
static __thread int static_tls = 500;

/* Common TLS symbol (tentative definition) */
__thread int common_tls;

/* External TLS declaration (simulating definition in another file) */
extern __thread int external_tls;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 600;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 700;

/* 2. Complex TLS with dynamic initialization */
__thread int dynamic_tls = 0;

/* Function to force dynamic initialization */
static int init_value(void) {
    side_effect();
    return 1234;
}

/* 3. TLS inside structures */
struct TLSContainer {
    __thread int member_tls;
    __thread char char_tls;
};

/* 4. Function-local static TLS */
void function_with_local_tls(void) {
    static __thread int local_static_tls = 800;
    local_static_tls++;
    use_ptr(&local_static_tls);
}

/* 5. Inline function accessing TLS (may trigger declaration copying during inlining) */
static inline int inline_tls_access(int idx) {
    /* Access different TLS variables based on index */
    switch(idx) {
        case 0: return public_tls;
        case 1: return hidden_tls;
        case 2: return static_tls;
        default: return volatile_tls;
    }
}

/* 6. Function that takes address of TLS and does complex operations */
void process_tls_addresses(void) {
    /* Array of pointers to TLS variables - volatile to prevent optimization */
    volatile void *tls_pointers[12];
    int i = 0;
    
    /* Take addresses of various TLS variables */
    tls_pointers[i++] = (void*)&public_tls;
    tls_pointers[i++] = (void*)&weak_tls;
    tls_pointers[i++] = (void*)&hidden_tls;
    tls_pointers[i++] = (void*)&internal_tls;
    tls_pointers[i++] = (void*)&protected_tls;
    tls_pointers[i++] = (void*)&dllimport_tls;
    tls_pointers[i++] = (void*)&static_tls;
    tls_pointers[i++] = (void*)&common_tls;
    tls_pointers[i++] = (void*)&aligned_tls;
    tls_pointers[i++] = (void*)&volatile_tls;
    tls_pointers[i++] = (void*)&dynamic_tls;
    
    /* Pass addresses to opaque function */
    for (int j = 0; j < i; j++) {
        use_ptr((void*)tls_pointers[j]);
    }
    
    /* Mix TLS addresses with non-TLS addresses */
    int local_var = 999;
    use_ptr2(&public_tls, &local_var);
    use_ptr2(&static_tls, &local_var);
}

/* 7. Complex expression with TLS */
int complex_tls_expression(void) {
    /* Force address-taking and arithmetic */
    int *ptr1 = &public_tls;
    int *ptr2 = &hidden_tls;
    
    /* Prevent optimization with volatile */
    volatile int result = (*ptr1 * *ptr2) + static_tls;
    
    /* Use inline function that accesses TLS */
    result += inline_tls_access(result % 4);
    
    return result;
}

/* 8. Loop with TLS access - may trigger optimization passes */
void loop_with_tls_access(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix different TLS accesses in loop */
        public_tls += i;
        hidden_tls -= i % 3;
        static_tls = (static_tls * 2) % 1000;
        
        /* Use inline function */
        sum += inline_tls_access(i % 4);
        
        /* Volatile access to prevent dead code elimination */
        sum += volatile_tls;
    }
    
    /* Use the sum to prevent removal */
    use_ptr((void*)(intptr_t)sum);
}

/* 9. Conditional TLS initialization based on runtime values */
void conditional_tls_init(int condition) {
    if (condition) {
        /* Force compiler to consider different initialization paths */
        public_tls = get_random_value();
        hidden_tls = public_tls * 2;
    } else {
        static_tls = get_random_value();
        volatile_tls = static_tls / 2;
    }
    
    /* Complex expression that uses both paths' results */
    int mixed = public_tls + hidden_tls + static_tls + volatile_tls;
    use_ptr((void*)(intptr_t)mixed);
}

/* 10. TLS in asm statement (for DECL_PRESERVE_P) */
void tls_in_asm(void) {
    int result;
    
    /* Use TLS variable in asm to force preservation */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "r" (public_tls)
        : "%eax"
    );
    
    /* Use the result */
    use_ptr((void*)(intptr_t)result);
}

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    srand(seed);
    
    /* Initialize dynamic TLS with function call */
    dynamic_tls = init_value();
    
    /* Initialize common TLS */
    common_tls = 555;
    
    /* Process TLS addresses - forces address-taking */
    process_tls_addresses();
    
    /* Call function with local static TLS */
    function_with_local_tls();
    
    /* Complex expressions with TLS */
    int complex_result = complex_tls_expression();
    
    /* Loop with TLS access - number of iterations based on seed */
    loop_with_tls_access((seed % 20) + 5);
    
    /* Conditional TLS initialization */
    conditional_tls_init(seed % 2);
    
    /* TLS in asm */
    tls_in_asm();
    
    /* Compute checksum of all TLS values to prevent removal */
    int checksum = 0;
    
    /* Access all TLS variables non-volatilely for checksum */
    checksum += public_tls;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += internal_tls;
    checksum += protected_tls;
    checksum += dllimport_tls;
    checksum += static_tls;
    checksum += common_tls;
    checksum += aligned_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    
    /* Use inline function in checksum */
    checksum += inline_tls_access(checksum % 4);
    
    /* Print checksum to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional volatile store to force all TLS to be considered "used" */
    volatile int final_check = checksum;
    
    return final_check % 256;
}

/* Stub implementations for opaque functions (for actual compilation) */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    (void)p;
}

void use_ptr2(void *p1, void *p2) {
    (void)p1;
    (void)p2;
}

int get_random_value(void) {
    return rand();
}

void side_effect(void) {
    /* Empty side effect */
}

/* Definition for external TLS */
__thread int external_tls = 9999;
