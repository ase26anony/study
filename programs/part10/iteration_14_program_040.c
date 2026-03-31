/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force emulated TLS */
#pragma GCC tls_model emulated

/* TLS declarations with different attributes */
__thread int tls_global __attribute__((used)) = 42;
static __thread long tls_static __attribute__((visibility("hidden"))) = 100;
extern __thread double tls_extern __attribute__((weak));

/* DLL import simulation (non-Windows fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
__thread int tls_imported __attribute__((weak));
#endif

/* Function to force address taking and declaration duplication */
static __attribute__((noinline)) 
void manipulate_tls_pointers(void *p1, void *p2, void *p3) {
    /* Opaque operations using inline assembly */
    asm volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
}

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    tls_global = rand() % 1000;
    tls_static = rand() % 1000;
    
    /* Local TLS in constructor - different scope */
    __thread int local_in_ctor __attribute__((used)) = rand() % 500;
    
    /* Take address to force declaration handling */
    int *ptr = &local_in_ctor;
    asm volatile ("" : : "r"(ptr) : "memory");
}

/* Complex statement expression using TLS */
#define TLS_COMPLEX_EXPR(var) ({ \
    __thread int temp_tls __attribute__((used)) = (var) * 2; \
    int *addr = &temp_tls; \
    asm volatile ("" : : "r"(addr) : "memory"); \
    temp_tls + 1; \
})

/* Function that creates multiple declaration contexts */
static int use_tls_in_different_contexts(void) {
    /* First context */
    {
        __thread int ctx1_var __attribute__((visibility("default"))) = 1;
        int *p1 = &ctx1_var;
        manipulate_tls_pointers(p1, &tls_global, &tls_static);
    }
    
    /* Second context - might trigger duplication */
    {
        __thread int ctx1_var __attribute__((visibility("default"))) = 2;
        /* Use statement expression */
        int result = TLS_COMPLEX_EXPR(ctx1_var);
        return result;
    }
}

int main(void) {
    int checksum = 0;
    
    /* Use local TLS in main */
    __thread int local_main __attribute__((used)) = 123;
    
    /* Take addresses of various TLS variables */
    int *addr1 = &tls_global;
    long *addr2 = &tls_static;
    int *addr3 = &local_main;
    
    /* Force declaration duplication through multiple uses */
    manipulate_tls_pointers(addr1, addr2, addr3);
    
    /* Use in statement expressions */
    checksum += TLS_COMPLEX_EXPR(tls_global);
    checksum += TLS_COMPLEX_EXPR(local_main);
    
    /* Create different declaration contexts */
    checksum += use_tls_in_different_contexts();
    
    /* Use external TLS */
    checksum += (int)tls_extern;
    
    /* Complex computation using all TLS variables */
    checksum += tls_global + (int)tls_static + local_main;
    
    printf("Checksum: %d\n", checksum);
    printf("TLS values: global=%d, static=%ld\n", tls_global, tls_static);
    
    return 0;
}
