/* tls_main.c - Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, int *b, double *c, long *d);
extern int compute_checksum(void);

/* External TLS declarations from other TU */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* File-local static TLS with attributes */
static __thread int static_tls_int __attribute__((used)) 
    __attribute__((visibility("hidden")));

/* TLS with DLL import attribute (Windows-specific, with fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls_int;
#else
/* Simulate similar behavior with weak attribute */
__thread int dllimport_tls_int __attribute__((weak));
#endif

/* TLS with explicit visibility */
__thread double public_tls_double __attribute__((visibility("default")));

/* Constructor to initialize TLS variables */
__attribute__((constructor)) 
static void init_tls_vars(void) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    static_tls_int = rand() % 1000;
    public_tls_double = (double)rand() / RAND_MAX * 100.0;
    
    /* Initialize extern TLS if weak reference is resolved */
    if (&extern_tls_long) {
        extern_tls_long = rand() % 5000;
    }
}

/* Noinline helper to force address taking and potential declaration cloning */
static __attribute__((noinline)) 
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &static_tls_int;
    double *p2 = &public_tls_double;
    
    /* Use inline assembly to create complex context for TLS addresses */
    __asm__ volatile (
        "# TLS address manipulation\n"
        : "+r" (p1), "+r" (p2)
        :
        : "memory"
    );
    
    /* Pass addresses to external function */
    use_tls_pointers(&static_tls_int, &extern_tls_int, 
                     &public_tls_double, &extern_tls_long);
}

/* Another helper that uses statement expressions with TLS */
static int tls_in_statement_expr(void) {
    /* GNU statement expression using TLS address */
    int result = ({
        int temp = static_tls_int;
        /* Take address within statement expression */
        int *ptr = &temp;
        /* Use TLS variable in computation */
        temp += public_tls_double > 50.0 ? 10 : 5;
        /* Call external function with TLS-derived value */
        extern void external_func(int);
        external_func(temp);
        temp;
    });
    
    return result;
}

int main(void) {
    /* Local TLS variable in main */
    __thread int local_main_tls __attribute__((used));
    local_main_tls = rand() % 100;
    
    /* Force TLS declaration duplication through multiple mechanisms */
    
    /* 1. Address taking in different contexts */
    int *addr1 = &local_main_tls;
    int *addr2 = &static_tls_int;
    
    /* 2. Use in inline assembly */
    __asm__ volatile (
        "# Using TLS variables\n"
        : 
        : "r" (addr1), "r" (addr2)
        : "memory"
    );
    
    /* 3. Call function that manipulates TLS addresses */
    manipulate_tls_addresses();
    
    /* 4. Use statement expression with TLS */
    int expr_result = tls_in_statement_expr();
    
    /* 5. Complex computation using multiple TLS variables */
    double complex_result = (double)local_main_tls * public_tls_double;
    if (&extern_tls_long) {
        complex_result += (double)extern_tls_long;
    }
    
    /* 6. Take address in loop to create multiple uses */
    for (int i = 0; i < 3; i++) {
        volatile int *volatile_ptr = &local_main_tls;
        (void)volatile_ptr;
    }
    
    /* Compute and print checksum to prevent optimization */
    int checksum = compute_checksum();
    printf("TLS checksum: %d (expr: %d, complex: %.2f)\n", 
           checksum, expr_result, complex_result);
    
    return 0;
}
