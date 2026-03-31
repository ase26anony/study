/* Main file with various TLS declarations and usage patterns */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
extern void opaque_operation(void *ptr);

/* External TLS declaration - will be defined in another TU */
extern __thread int extern_tls_var __attribute__((weak));

/* TLS with various attributes to set the flags in target block */
__thread int global_tls __attribute__((used, visibility("default")));
static __thread long static_tls __attribute__((visibility("hidden")));
__thread double weak_tls_var __attribute__((weak, used));

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor)) 
static void init_tls_values(void) {
    srand(time(NULL));
    global_tls = rand() % 1000;
    static_tls = rand() % 1000;
    weak_tls_var = (double)(rand() % 1000) / 10.0;
    
    /* Force TLS address usage in constructor */
    volatile long *ptr = &static_tls;
    (void)ptr;
}

/* Noinline function to force declaration duplication */
static __attribute__((noinline, used)) 
void manipulate_tls_addresses(void) {
    /* Take addresses of TLS variables - may trigger duplication */
    int *p1 = &global_tls;
    long *p2 = &static_tls;
    double *p3 = &weak_tls_var;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
    
    /* Pass to external function */
    use_tls_pointers(p1, p2, p3);
}

/* Complex statement expression using TLS address */
#define TLS_ADDR_EXPR(var) ({ \
    __typeof__(var) *ptr = &(var); \
    opaque_operation(ptr); \
    *ptr; \
})

int main(void) {
    /* Local TLS variable in main */
    __thread int local_tls_main = 42;
    
    /* Force address taking in multiple contexts */
    int *addr1 = &local_tls_main;
    long *addr2 = &static_tls;
    
    /* Use statement expression macro */
    int val1 = TLS_ADDR_EXPR(global_tls);
    long val2 = TLS_ADDR_EXPR(static_tls);
    
    /* Call function that manipulates addresses */
    manipulate_tls_addresses();
    
    /* Complex computation using multiple TLS variables */
    int checksum = global_tls + (int)static_tls + (int)weak_tls_var + local_tls_main;
    
    /* Use inline assembly with TLS addresses */
    __asm__ volatile (
        "addl %1, %0\n\t"
        : "+r"(checksum)
        : "r"(global_tls)
        : "cc"
    );
    
    /* Prevent dead code elimination */
    volatile int result = checksum;
    
    printf("TLS checksum: %d\n", checksum);
    printf("Values: global=%d, static=%ld, weak=%.2f, local=%d\n",
           global_tls, static_tls, weak_tls_var, local_tls_main);
    
    return 0;
}
