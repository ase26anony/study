/* Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
extern int compute_checksum(void);

/* External TLS declarations from another TU */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* File-local static TLS with attributes */
static __thread int static_tls_int __attribute__((used)) 
    __attribute__((visibility("hidden")));

/* TLS with DLL import attribute simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* TLS with visibility attribute */
__thread double global_tls_double __attribute__((visibility("default")));

/* Weak TLS declaration */
__thread char weak_tls_char __attribute__((weak));

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_values(void) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    static_tls_int = rand() % 1000;
    global_tls_double = (double)rand() / RAND_MAX * 100.0;
    weak_tls_char = 'A' + (rand() % 26);
}

/* Noinline function that takes TLS addresses */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &static_tls_int;
    double *p2 = &global_tls_double;
    char *p3 = &weak_tls_char;
    
    /* Use inline assembly to force address usage */
    __asm__ volatile (
        "# TLS address manipulation\n"
        : "+r" (p1), "+r" (p2), "+r" (p3)
        :
        : "memory"
    );
    
    /* Opaque operations to prevent optimization */
    use_tls_pointers(p1, (long *)p2, (double *)p3);
}

/* Function using statement expression with TLS */
static int tls_in_statement_expr(void) {
    /* GNU statement expression using TLS address */
    int result = ({
        __thread int local_stmt_tls __attribute__((used)) = 42;
        int *ptr = &local_stmt_tls;
        
        /* Force declaration duplication context */
        extern int external_func(int *);
        int val = external_func(ptr);
        
        /* Complex expression */
        val += static_tls_int;
        val;
    });
    
    return result;
}

int main(void) {
    /* Local TLS variable in main */
    __thread int main_local_tls __attribute__((used)) = 123;
    
    /* Take address and use in complex way */
    int *main_tls_ptr = &main_local_tls;
    
    /* Force multiple uses of TLS addresses */
    manipulate_tls_addresses();
    
    /* Use statement expression */
    int stmt_result = tls_in_statement_expr();
    
    /* Complex computation using multiple TLS variables */
    int checksum = compute_checksum();
    
    /* Use inline assembly with TLS address */
    __asm__ volatile (
        "# Final TLS usage\n"
        : 
        : "r" (main_tls_ptr), "r" (&global_tls_double)
        : "memory"
    );
    
    printf("TLS checksum: %d\n", checksum);
    printf("Statement expr result: %d\n", stmt_result);
    
    return 0;
}
