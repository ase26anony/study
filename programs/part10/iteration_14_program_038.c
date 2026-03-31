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

/* TLS with various attributes to set the flags we want to cover */
__thread int global_tls_int __attribute__((used, visibility("default")));
static __thread int static_tls_int __attribute__((used));

/* Weak TLS variable */
__thread long weak_tls_long __attribute__((weak, visibility("hidden")));

/* TLS with explicit visibility */
__thread double visible_tls_double __attribute__((visibility("hidden")));

/* For DLL import simulation (will be defined in another TU) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_int;
#else
/* Simulate similar attribute with alias */
extern __thread int imported_tls_int __attribute__((weak));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_vars(void) {
    srand(time(NULL));
    global_tls_int = rand() % 1000;
    static_tls_int = rand() % 1000;
    weak_tls_long = rand() % 1000;
    visible_tls_double = (double)(rand() % 1000) / 10.0;
    
    printf("Initialized TLS variables in constructor\n");
}

/* Noinline function that takes TLS variable addresses */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses of TLS variables - may trigger duplication */
    int *p1 = &global_tls_int;
    long *p2 = &weak_tls_long;
    double *p3 = &visible_tls_double;
    
    /* Use inline assembly to ensure addresses are used */
    __asm__ volatile (
        "# TLS address use %0, %1, %2"
        : 
        : "r" (p1), "r" (p2), "r" (p3)
        : "memory"
    );
    
    /* Call external function with TLS addresses */
    use_tls_pointers(p1, p2, p3);
}

/* Another function that creates complex context for TLS */
static __attribute__((noinline))
int tls_in_statement_expr(void) {
    /* GNU statement expression with TLS variable address */
    int result = ({
        int local_tls __thread = 42;
        int *addr = &local_tls;
        
        /* Complex operation that might trigger declaration cloning */
        __asm__ volatile (
            "addl $1, %0"
            : "+m" (*addr)
        );
        
        /* Use external function call in the expression */
        extern int external_helper(int);
        external_helper(*addr);
        
        *addr;
    });
    
    return result;
}

int main(void) {
    /* Local TLS variable in main */
    __thread int main_local_tls = 123;
    
    /* Take address and use in various ways */
    int *main_tls_addr = &main_local_tls;
    
    /* Force multiple uses of TLS addresses */
    manipulate_tls_addresses();
    
    /* Use statement expression with TLS */
    int from_expr = tls_in_statement_expr();
    
    /* Complex computation using multiple TLS variables */
    int checksum = global_tls_int + 
                   static_tls_int + 
                   (int)weak_tls_long + 
                   (int)visible_tls_double +
                   main_local_tls +
                   from_expr;
    
    /* Add external TLS variables if available */
    checksum += extern_tls_int;
    
    printf("TLS checksum: %d\n", checksum);
    
    /* Compute final checksum from all TUs */
    int final_checksum = compute_checksum();
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
