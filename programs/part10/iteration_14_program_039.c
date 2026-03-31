/* tls_main.c - Main file with TLS declarations and usage */

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
static __thread int static_tls_int __attribute__((used)) = 42;
static __thread long static_tls_long __attribute__((visibility("hidden"))) = 100;

/* TLS with explicit visibility */
__thread double global_tls_double __attribute__((visibility("default"))) = 3.14159;

/* Weak TLS declaration */
__thread char weak_tls_char __attribute__((weak)) = 'A';

/* Constructor to initialize TLS with non-constant values */
void __attribute__((constructor)) init_tls_values(void) {
    srand(time(NULL));
    static_tls_int = rand() % 1000;
    static_tls_long = rand() % 10000;
    global_tls_double = (double)rand() / RAND_MAX * 100.0;
}

/* Noinline helper to force address taking and potential declaration cloning */
static __attribute__((noinline)) 
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &static_tls_int;
    long *p2 = &static_tls_long;
    double *p3 = &global_tls_double;
    char *p4 = &weak_tls_char;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    
    /* Complex statement expression that may trigger declaration cloning */
    int result = ({
        int temp = *p1 + (int)*p2;
        temp += (int)*p3;
        temp += *p4;
        temp;
    });
    
    /* Opaque use of result */
    __asm__ volatile ("" : : "r"(result) : "memory");
}

/* Another function that takes TLS addresses */
void __attribute__((noinline)) process_tls(int *iptr, long *lptr, double *dptr) {
    if (iptr && lptr && dptr) {
        *iptr += 1;
        *lptr += *iptr;
        *dptr += *lptr;
    }
}

int main(void) {
    /* Local TLS in main function */
    __thread int local_tls_int = 123;
    
    /* Initialize external TLS (defined in another file) */
    extern_tls_int = 999;
    
    /* Force address taking of local TLS */
    int *local_ptr = &local_tls_int;
    
    /* Use GNU statement expression with TLS address */
    int complex_result = ({
        /* This creates a new context for the TLS variable */
        __thread int temp_tls __attribute__((used)) = local_tls_int * 2;
        int *temp_ptr = &temp_tls;
        process_tls(&local_tls_int, &static_tls_long, &global_tls_double);
        *temp_ptr + local_tls_int;
    });
    
    /* Call function that manipulates TLS addresses */
    manipulate_tls_addresses();
    
    /* Take address and pass to external function */
    use_tls_pointers(&static_tls_int, &static_tls_long, &global_tls_double);
    
    /* Compute and print checksum to prevent optimization */
    int checksum = compute_checksum();
    printf("TLS checksum: %d\n", checksum);
    printf("Complex result: %d\n", complex_result);
    
    return 0;
}
