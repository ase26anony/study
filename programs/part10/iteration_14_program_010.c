/* tls_main.c - Main file with TLS declarations and usage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern long get_tls_sum(void);
extern void use_tls_pointers(int *p1, long *p2, double *p3);

/* External TLS declarations from other file */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* File-local static TLS with various attributes */
static __thread int static_tls_int __attribute__((used, visibility("hidden")));
__thread long common_tls_long __attribute__((common));

/* Public TLS with explicit visibility */
__thread double public_tls_double __attribute__((visibility("default")));

/* Weak TLS declaration */
__thread int weak_tls_int __attribute__((weak));

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_values(void) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    static_tls_int = rand() % 1000;
    common_tls_long = rand() % 10000;
    public_tls_double = (double)rand() / RAND_MAX * 100.0;
    
    /* Initialize weak TLS if it's defined locally */
    if (&weak_tls_int != NULL) {
        weak_tls_int = rand() % 500;
    }
}

/* Noinline function to force address taking and potential declaration cloning */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &static_tls_int;
    long *p2 = &common_tls_long;
    double *p3 = &public_tls_double;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile (
        "# TLS address manipulation\n"
        : "+r" (p1), "+r" (p2), "+r" (p3)
        :
        : "memory"
    );
    
    /* Pass to external function */
    use_tls_pointers(p1, p2, p3);
}

/* Function using statement expression with TLS */
static long tls_statement_expr(void) {
    /* GNU statement expression that uses TLS variables */
    return ({
        __thread int local_stmt_tls __attribute__((used)) = 42;
        long result = local_stmt_tls + static_tls_int;
        
        /* Take address in statement expression */
        int *addr = &local_stmt_tls;
        __asm__ volatile ("# Statement expr TLS" : "+r" (addr) : : "memory");
        
        result;
    });
}

int main(void) {
    /* Local TLS variable in main */
    __thread int local_main_tls __attribute__((used)) = 123;
    
    /* Force TLS variable usage with complex expressions */
    long sum = 0;
    
    /* Use statement expression */
    sum += tls_statement_expr();
    
    /* Manipulate addresses */
    manipulate_tls_addresses();
    
    /* Use local TLS */
    sum += local_main_tls;
    
    /* Take address and use in inline assembly */
    int *local_addr = &local_main_tls;
    __asm__ volatile (
        "# Main TLS address\n"
        : "+r" (local_addr)
        :
        : "memory"
    );
    
    /* Get sum from external function */
    sum += get_tls_sum();
    
    /* Compute checksum */
    sum += static_tls_int;
    sum += common_tls_long;
    sum += (long)public_tls_double;
    
    printf("TLS checksum: %ld\n", sum);
    
    return 0;
}
