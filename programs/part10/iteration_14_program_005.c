/* tls_main.c - Main file with TLS declarations and usage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, int *b, double *c, long *d);
extern int compute_checksum(void);

/* External TLS variable declaration (defined in another file) */
extern __thread int extern_tls_var __attribute__((weak, visibility("default")));

/* File-local static TLS with various attributes */
static __thread int static_tls_int __attribute__((used, visibility("hidden")));

/* Plain TLS with common linkage */
__thread long plain_tls_long;

/* TLS with DLL import attribute simulation (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread double imported_tls_double;
#else
/* Simulate similar behavior with attributes */
__thread double imported_tls_double __attribute__((visibility("default")));
#endif

/* Weak TLS variable */
__thread int weak_tls_int __attribute__((weak));

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_vars(void) {
    srand(time(NULL));
    static_tls_int = rand() % 1000;
    plain_tls_long = rand() % 10000;
    weak_tls_int = rand() % 500;
    
    /* imported_tls_double initialized in main since it might be dllimport */
}

/* Noinline function to force address taking and potential declaration cloning */
static __attribute__((noinline)) 
void manipulate_tls_addresses(void) {
    /* Take addresses of TLS variables - may trigger declaration cloning */
    int *p1 = &static_tls_int;
    long *p2 = &plain_tls_long;
    double *p3 = &imported_tls_double;
    int *p4 = &weak_tls_int;
    
    /* Use inline assembly to ensure addresses are used
       This can trigger TLS declaration duplication */
    __asm__ volatile (
        "# TLS address use %0, %1, %2, %3"
        : 
        : "r" (p1), "r" (p2), "r" (p3), "r" (p4)
        : "memory"
    );
    
    /* Call external function with TLS addresses */
    use_tls_pointers(p1, p4, p3, p2);
}

/* Another function that uses statement expressions with TLS variables */
static int tls_in_statement_expr(void) {
    /* GNU statement expression using TLS address - may create new context */
    int result = ({
        int temp = static_tls_int;
        long *addr = &plain_tls_long;
        /* Inline asm to prevent optimization */
        __asm__ volatile ("# Statement expr TLS use" : "+r" (temp) : "r" (addr));
        temp + *addr;
    });
    
    return result;
}

int main(void) {
    /* Initialize imported TLS variable */
    imported_tls_double = (double)(rand() % 1000) / 10.0;
    
    /* Local TLS variable in main - another declaration context */
    __thread int local_main_tls __attribute__((used)) = 42;
    
    /* Force address taking of local TLS */
    int *local_addr = &local_main_tls;
    
    /* Complex expression using TLS addresses that might trigger cloning */
    int checksum = (int)((long)&local_main_tls ^ (long)&static_tls_int ^ 
                        (long)&plain_tls_long ^ (long)&imported_tls_double);
    
    /* Call functions that manipulate TLS addresses */
    manipulate_tls_addresses();
    
    /* Use statement expression with TLS */
    checksum += tls_in_statement_expr();
    
    /* Take address in a way that might create duplicate declaration */
    {
        /* Nested scope with address taking */
        __thread int * volatile tls_ptr = &local_main_tls;
        __asm__ volatile ("# Nested scope TLS use" : : "r" (tls_ptr));
    }
    
    /* Compute and print final checksum */
    checksum += compute_checksum();
    printf("TLS checksum: %d\n", checksum);
    
    /* Ensure all TLS variables are marked as used */
    printf("TLS values: static=%d, plain=%ld, weak=%d, imported=%.2f, local=%d\n",
           static_tls_int, plain_tls_long, weak_tls_int, 
           imported_tls_double, local_main_tls);
    
    return 0;
}
