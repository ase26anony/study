/* tls_main.c - Main file with various TLS declarations and attribute manipulations */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
extern int compute_checksum(void);

/* External TLS declarations from other TU */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* File-local static TLS with various attributes */
static __thread int static_tls_int __attribute__((used, visibility("hidden")));

/* TLS with explicit visibility */
__thread long global_tls_long __attribute__((visibility("default")));

/* Weak TLS variable */
__thread double weak_tls_double __attribute__((weak));

/* DLL import simulation (with fallback for non-Windows) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_int;
#else
/* Simulate DLL import attribute with a custom attribute */
#define __dllimport __attribute__((dllimport))
__thread int imported_tls_int __attribute__((dllimport));
#endif

/* Constructor to initialize TLS variables with non-constant values */
__attribute__((constructor))
static void init_tls_vars(void) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    static_tls_int = rand() % 1000;
    global_tls_long = rand() % 10000;
    weak_tls_double = (double)rand() / RAND_MAX * 100.0;
    
    /* Initialize extern TLS through pointer manipulation */
    extern int *get_extern_tls_int_ptr(void);
    int *ptr = get_extern_tls_int_ptr();
    if (ptr) *ptr = rand() % 500;
}

/* Noinline function to force address taking and potential declaration cloning */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &static_tls_int;
    long *p2 = &global_tls_long;
    double *p3 = &weak_tls_double;
    
    /* Use inline assembly to ensure addresses are used in a way that
       might trigger declaration duplication */
    __asm__ volatile (
        "# TLS address manipulation\n"
        : "+r" (p1), "+r" (p2), "+r" (p3)
        :
        : "memory"
    );
    
    /* Pass to external function */
    use_tls_pointers(p1, p2, p3);
}

/* Function using GNU statement expressions with TLS variables */
static int tls_statement_expr(void) {
    /* Complex statement expression that uses TLS variables */
    return ({
        __thread int local_tls_in_expr __attribute__((used));
        local_tls_in_expr = static_tls_int + global_tls_long;
        
        /* Take address within statement expression */
        int *addr = &local_tls_in_expr;
        
        /* Inline assembly to prevent optimization */
        __asm__ volatile (
            "# Statement expr TLS use\n"
            : "+r" (addr)
            :
            : "memory"
        );
        
        local_tls_in_expr * 2;
    });
}

int main(void) {
    /* Local TLS variable in main */
    __thread int main_local_tls __attribute__((used));
    main_local_tls = 42;
    
    /* Take address and use in opaque way */
    int *main_tls_addr = &main_local_tls;
    
    /* Force potential declaration cloning through complex use */
    manipulate_tls_addresses();
    
    /* Use statement expression with TLS */
    int result = tls_statement_expr();
    
    /* Compute and print checksum using all TLS variables */
    int checksum = compute_checksum();
    printf("TLS checksum: %d\n", checksum);
    printf("Statement expr result: %d\n", result);
    
    /* Use main's TLS variable address in inline asm */
    __asm__ volatile (
        "# Main TLS reference %0\n"
        :
        : "r" (main_tls_addr)
        : "memory"
    );
    
    return 0;
}
