/* tls_main.c - Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, int *b, double *c, long *d);
extern int checksum_tls(void);

/* External TLS declarations (defined in tls_aux.c) */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* File-local static TLS with various attributes */
static __thread int static_tls_int __attribute__((used));
static __thread double static_tls_double __attribute__((visibility("hidden")));

/* TLS with weak attribute */
__thread int weak_tls_int __attribute__((weak));

/* TLS with default visibility explicitly specified */
__thread long default_vis_tls_long __attribute__((visibility("default")));

/* Global TLS with potential common linkage */
__thread int global_tls_int;

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_vars(void) {
    srand(time(NULL));
    static_tls_int = rand() % 1000;
    static_tls_double = (double)(rand() % 1000) / 10.0;
    global_tls_int = rand() % 1000;
    
    /* Initialize weak TLS if it's the strong definition */
    if (&weak_tls_int) {
        weak_tls_int = rand() % 1000;
    }
}

/* Noinline helper to force address taking and potential declaration cloning */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &static_tls_int;
    double *p2 = &static_tls_double;
    int *p3 = &global_tls_int;
    long *p4 = &default_vis_tls_long;
    
    /* Use inline assembly to create opaque context for TLS addresses */
    __asm__ volatile (
        "# TLS address manipulation %0, %1, %2, %3"
        : /* no outputs */
        : "r" (p1), "r" (p2), "r" (p3), "r" (p4)
        : "memory"
    );
    
    /* Complex statement expression with TLS address usage */
    int result = ({
        int temp = *p1 + *p3;
        /* Call external function with TLS addresses */
        use_tls_pointers(p1, p3, p2, p4);
        temp;
    });
    
    (void)result; /* Prevent unused variable warning */
}

/* Another function that creates different context for TLS usage */
__attribute__((noinline))
static void tls_in_different_scope(void) {
    /* Local TLS in function scope - may trigger declaration cloning */
    __thread int local_func_tls __attribute__((used)) = 123;
    
    /* Take address and use in assembly */
    int *local_ptr = &local_func_tls;
    
    __asm__ volatile (
        "# Local TLS usage %0"
        : /* no outputs */
        : "r" (local_ptr)
        : "memory"
    );
    
    /* Use in statement expression */
    int val = ({
        int x = *local_ptr;
        x * 2;
    });
    
    /* Pass to external function */
    use_tls_pointers(local_ptr, &global_tls_int, &static_tls_double, &default_vis_tls_long);
    
    (void)val;
}

int main(void) {
    printf("TLS Coverage Test Program\n");
    
    /* Initialize external TLS through function call */
    extern_tls_int = 42;
    if (&extern_tls_long) {
        extern_tls_long = 100;
    }
    
    /* Force manipulation of TLS addresses */
    manipulate_tls_addresses();
    
    /* Create another context for TLS usage */
    tls_in_different_scope();
    
    /* Use GNU statement expression with TLS variable address */
    int complex_result = ({
        /* Declare TLS inside statement expression */
        static __thread int stmt_expr_tls __attribute__((used)) = 999;
        int *ptr = &stmt_expr_tls;
        
        /* Multiple operations to create complex context */
        *ptr += global_tls_int;
        *ptr += static_tls_int;
        
        /* Inline assembly barrier */
        __asm__ volatile ("# Statement expr TLS" : : "r"(ptr) : "memory");
        
        *ptr;
    });
    
    printf("Statement expression result: %d\n", complex_result);
    
    /* Compute and print checksum using all TLS variables */
    int final_checksum = checksum_tls();
    printf("Final TLS checksum: %d\n", final_checksum);
    
    return 0;
}
