/* Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, int *b, double *c, long *d);
extern unsigned long compute_checksum(void);

/* External TLS declarations from other TU */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* File-local static TLS with attributes */
static __thread int static_tls_int __attribute__((used)) 
    __attribute__((visibility("hidden")));

/* TLS with explicit visibility */
__thread double global_tls_double __attribute__((visibility("default")));

/* Weak TLS variable */
__thread long weak_tls_long __attribute__((weak));

/* TLS with dllimport simulation (using declspec for compatibility) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_int;
#else
/* Simulate similar attribute on non-Windows */
__thread int imported_tls_int __attribute__((visibility("default")));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor)) 
static void init_tls_variables(void) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    static_tls_int = rand() % 1000;
    global_tls_double = (double)rand() / RAND_MAX * 100.0;
    weak_tls_long = rand() % 10000;
    
    printf("Initialized TLS variables in constructor\n");
}

/* Noinline helper to force address taking and potential declaration cloning */
static __attribute__((noinline)) 
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &static_tls_int;
    double *p2 = &global_tls_double;
    long *p3 = &weak_tls_long;
    
    /* Use inline assembly to create opaque context for TLS addresses */
    __asm__ volatile (
        "# TLS address manipulation\n"
        : "+r" (p1), "+r" (p2), "+r" (p3)
        :
        : "memory"
    );
    
    /* Pass to external function to prevent optimization */
    use_tls_pointers(&static_tls_int, &extern_tls_int, 
                     &global_tls_double, &weak_tls_long);
}

/* Complex statement expression using TLS address */
static unsigned long tls_statement_expression(void) {
    /* GNU statement expression that uses TLS address */
    return ({
        __thread int local_tls_in_expr __attribute__((used));
        int *ptr = &local_tls_in_expr;
        local_tls_in_expr = rand() % 256;
        
        /* Opaque assembly to prevent optimization */
        __asm__ volatile (
            "# Statement expr TLS use\n"
            : "+r" (ptr)
            :
            : "memory"
        );
        
        (unsigned long)local_tls_in_expr * 31;
    });
}

int main(void) {
    /* Local TLS variable in main */
    __thread int main_local_tls __attribute__((used));
    main_local_tls = 42;
    
    /* Take address and use in complex ways */
    int *main_tls_ptr = &main_local_tls;
    
    /* Force potential declaration cloning through multiple uses */
    for (int i = 0; i < 3; i++) {
        /* Each iteration creates different context for TLS use */
        __thread int loop_tls __attribute__((visibility("hidden")));
        loop_tls = i * 100;
        
        /* Use address in inline assembly */
        __asm__ volatile (
            "# Loop TLS manipulation %0\n"
            : "+m" (loop_tls)
            :
            : "memory"
        );
        
        main_local_tls += loop_tls;
    }
    
    /* Manipulate various TLS addresses */
    manipulate_tls_addresses();
    
    /* Use statement expression with TLS */
    unsigned long expr_result = tls_statement_expression();
    
    /* Compute and print checksum */
    unsigned long checksum = compute_checksum();
    checksum += expr_result;
    checksum += (unsigned long)main_local_tls;
    
    printf("TLS checksum: %lu\n", checksum);
    printf("Expression result: %lu\n", expr_result);
    
    return 0;
}
