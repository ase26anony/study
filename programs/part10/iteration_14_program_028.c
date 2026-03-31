/* tls_main.c - Main file with TLS declarations and usage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, int *b, double *c, long *d);
extern int checksum_tls_values(void);

/* External TLS declarations from other TU */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* TLS declarations with various attributes */
__thread int global_tls_int __attribute__((used, visibility("default")));
static __thread int static_tls_int __attribute__((used));
__thread double global_tls_double __attribute__((visibility("hidden")));
__thread long global_tls_long __attribute__((weak));

/* Constructor to initialize TLS variables */
__attribute__((constructor)) 
static void init_tls_vars(void) {
    srand(time(NULL));
    global_tls_int = rand() % 1000;
    static_tls_int = rand() % 1000;
    global_tls_double = (double)(rand() % 1000) / 10.0;
    global_tls_long = rand() % 1000;
    
    /* Initialize extern TLS if weak symbol not resolved */
    if (&extern_tls_long == NULL) {
        /* This branch won't be taken if symbol is defined elsewhere,
           but ensures the address is taken */
    }
}

/* Opaque function that forces address taking */
static __attribute__((noinline, used)) 
void manipulate_tls_addresses(void) {
    /* Take addresses of TLS variables in various ways */
    int *p1 = &global_tls_int;
    int *p2 = &static_tls_int;
    double *p3 = &global_tls_double;
    long *p4 = &global_tls_long;
    
    /* Use inline assembly to ensure addresses are used */
    __asm__ volatile (
        "# TLS address manipulation\n"
        : "+r" (p1), "+r" (p2)
        : "r" (p3), "r" (p4)
        : "memory"
    );
    
    /* Call external function with TLS pointers */
    use_tls_pointers(p1, p2, p3, p4);
}

/* Complex statement expression using TLS */
static int tls_statement_expr(void) {
    /* GNU statement expression that creates a new context for TLS */
    int result = ({
        __thread int local_tls_in_expr __attribute__((used));
        local_tls_in_expr = global_tls_int + static_tls_int;
        
        /* Take address within statement expression */
        int *addr = &local_tls_in_expr;
        
        /* Opaque operation */
        __asm__ volatile (
            "# Statement expr TLS\n"
            : "+r" (addr)
            :: "memory"
        );
        
        local_tls_in_expr;
    });
    
    return result;
}

int main(void) {
    /* Local TLS variable in main */
    __thread int main_local_tls __attribute__((used));
    main_local_tls = rand() % 100;
    
    /* Force address taking of local TLS */
    int *main_tls_addr = &main_local_tls;
    
    /* Use statement expression */
    int expr_result = tls_statement_expr();
    
    /* Manipulate TLS addresses */
    manipulate_tls_addresses();
    
    /* Complex use of TLS addresses */
    {
        /* Another scope with TLS usage */
        __thread int scope_tls __attribute__((visibility("hidden")));
        scope_tls = *main_tls_addr + expr_result;
        
        /* Force address in inline asm */
        __asm__ volatile (
            "# Scope TLS use\n"
            : "+m" (scope_tls)
            :: "memory"
        );
    }
    
    /* Compute and print checksum */
    int checksum = checksum_tls_values();
    printf("TLS checksum: %d\n", checksum);
    printf("Expression result: %d\n", expr_result);
    
    return 0;
}
