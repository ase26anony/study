/* test-tls-emulation.c
 * 
 * This program is designed to trigger TLS emulation scenarios and
 * declaration attribute copying in GCC's tree-emutls.cc.
 * It creates TLS variables with diverse attributes and uses them
 * in ways that force the compiler to generate emulation structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern int opaque_int(void);
extern void *opaque_ptr(void);

/* External TLS variables (simulating another compilation unit) */
extern __thread int ext_tls_public;
extern __thread int ext_tls_weak __attribute__((weak));
extern __thread int ext_tls_hidden __attribute__((visibility("hidden")));

/* Public TLS with different attributes */
__thread int tls_public = 42;
__thread int tls_common;  /* Tentative definition - should be COMMON */
__thread volatile int tls_volatile = 100;

/* Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 200;

/* TLS with specified visibility */
__thread int tls_default __attribute__((visibility("default"))) = 300;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;
__thread int tls_internal __attribute__((visibility("internal"))) = 500;
__thread int tls_protected __attribute__((visibility("protected"))) = 600;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Use dllimport attribute if supported, otherwise regular TLS */
# ifdef __has_attribute
#  if __has_attribute(dllimport)
__attribute__((dllimport)) __thread int tls_dllimport;
#  else
__thread int tls_dllimport = 700;
#  endif
# else
__thread int tls_dllimport = 700;
# endif
#endif

/* Static TLS inside and outside functions */
static __thread int tls_static_file = 800;

/* TLS with dynamic initialization */
__thread int tls_dynamic = 0;

/* Function that might be inlined, accessing TLS */
static inline int inline_tls_access(int idx) {
    static __thread int tls_static_func = 900;
    tls_static_func += idx;
    
    /* Mix with other TLS variables */
    tls_hidden += tls_static_func;
    return tls_static_func;
}

/* Function that takes address of TLS - forces addressability */
static void *take_tls_address(void) {
    /* Array of pointers to TLS variables */
    static volatile void *tls_addrs[20];
    
    tls_addrs[0] = &tls_public;
    tls_addrs[1] = &tls_common;
    tls_addrs[2] = &tls_weak;
    tls_addrs[3] = &tls_hidden;
    tls_addrs[4] = &tls_internal;
    tls_addrs[5] = &tls_protected;
    tls_addrs[6] = &tls_dllimport;
    tls_addrs[7] = &tls_static_file;
    tls_addrs[8] = &tls_dynamic;
    
    /* Pass addresses to opaque function */
    for (int i = 0; i < 9; i++) {
        use_ptr((void *)tls_addrs[i]);
    }
    
    return (void *)tls_addrs[opaque_int() % 9];
}

/* Complex expression with TLS that might trigger declaration cloning */
static int complex_tls_expression(int x) {
    /* Taking address in complex way */
    int *ptr = (x > 0) ? &tls_public : &tls_hidden;
    
    /* Using TLS in loop with runtime bounds */
    int sum = 0;
    for (int i = 0; i < (x % 10 + 1); i++) {
        sum += *ptr + inline_tls_access(i);
        tls_volatile = sum; /* volatile write */
    }
    
    /* Mix with external TLS */
    if (x % 3 == 0) {
        sum += ext_tls_public;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    srand(seed);
    
    /* Initialize dynamic TLS */
    tls_dynamic = opaque_int();
    
    /* Force TLS variable preservation by using in asm-like context */
    /* Simulate asm statement that might reference TLS */
    volatile int *force_preserve = &tls_public;
    use_ptr(force_preserve);
    
    /* Access all TLS variables in unpredictable order */
    int checksum = 0;
    
    /* Array of TLS variable pointers for indirect access */
    int *tls_ptrs[] = {
        &tls_public,
        &tls_common,
        &tls_weak,
        &tls_hidden,
        &tls_internal,
        &tls_protected,
        &tls_dllimport,
        &tls_static_file,
        &tls_dynamic,
        &tls_default
    };
    
    /* Unpredictable access pattern based on seed */
    for (int i = 0; i < 100; i++) {
        int idx = (rand() + i) % (sizeof(tls_ptrs) / sizeof(tls_ptrs[0]));
        
        /* Modify TLS variable */
        *tls_ptrs[idx] += (rand() % 100) - 50;
        
        /* Use in complex expression */
        checksum += complex_tls_expression(*tls_ptrs[idx]);
        
        /* Call inline function that uses TLS */
        checksum += inline_tls_access(i % 5);
        
        /* Take address periodically */
        if (i % 7 == 0) {
            use_ptr(take_tls_address());
        }
    }
    
    /* Final checksum computation using all TLS variables */
    checksum += tls_public;
    checksum += tls_common;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_dllimport;
    checksum += tls_static_file;
    checksum += tls_dynamic;
    checksum += tls_default;
    checksum += tls_volatile;
    
    /* Simulate access to external TLS */
    checksum += ext_tls_public;
    if (&ext_tls_weak != NULL) {
        checksum += ext_tls_weak;
    }
    checksum += ext_tls_hidden;
    
    printf("TLS checksum: %d\n", checksum);
    
    /* Use TLS addresses in final opaque call */
    for (int i = 0; i < sizeof(tls_ptrs) / sizeof(tls_ptrs[0]); i++) {
        use_ptr(tls_ptrs[i]);
    }
    
    return checksum % 256;
}

/* Stub definitions for opaque functions (for actual compilation) */
#ifdef COMPILE_WITH_STUBS
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    static volatile void *last_ptr;
    last_ptr = p;
}

int opaque_int(void) {
    return rand() % 1000;
}

void *opaque_ptr(void) {
    return NULL;
}

/* Definitions for external TLS variables */
__thread int ext_tls_public = 1111;
__thread int ext_tls_weak = 2222;
__thread int ext_tls_hidden = 3333;
#endif
