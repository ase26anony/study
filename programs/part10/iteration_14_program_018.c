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

/* TLS with various attributes to set the flags in the uncovered block */

/* Will set DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC, DECL_EXTERNAL */
__thread int global_tls_int __attribute__((used)) = 42;

/* Will set DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread double global_tls_double __attribute__((visibility("hidden"))) = 3.14159;

/* Will set DECL_WEAK */
__thread long global_tls_long __attribute__((weak)) = 100;

/* Static TLS - file scope */
static __thread int static_tls_int = 0;

/* TLS with common linkage */
__thread int common_tls_int;

/* For Windows DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_int;
#else
/* Simulate similar attribute */
__thread int imported_tls_int __attribute__((visibility("default"))) = 0;
#endif

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    static int initialized = 0;
    if (initialized) return;
    initialized = 1;
    
    srand(time(NULL));
    
    /* Initialize with non-constant values to prevent constant folding */
    static_tls_int = rand() % 1000;
    common_tls_int = rand() % 1000;
    
    /* Modify global TLS */
    global_tls_int += rand() % 100;
    global_tls_double += (double)(rand() % 100) / 100.0;
}

/* Noinline function to force address taking and potential declaration cloning */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses of TLS variables - may trigger declaration duplication */
    int *p1 = &global_tls_int;
    double *p2 = &global_tls_double;
    long *p3 = &global_tls_long;
    int *p4 = &static_tls_int;
    int *p5 = &common_tls_int;
    
    /* Use inline assembly to create opaque context for TLS addresses */
    __asm__ volatile (
        "# TLS address manipulation\n"
        : "+r" (p1), "+r" (p2)
        : "r" (p3), "r" (p4), "r" (p5)
        : "memory"
    );
    
    /* Pass to external function to prevent optimization */
    use_tls_pointers(p1, p4, p2, p3);
}

/* Complex statement expression using TLS address */
#define TLS_ADDR_EXPR(var) ({ \
    __typeof__(var) *ptr = &(var); \
    __typeof__(var) val = *ptr; \
    val += 1; \
    *ptr = val; \
    (void)0; \
})

int main(void) {
    /* Local TLS variable in function scope */
    __thread int local_tls_int = 10;
    
    /* Force TLS declaration duplication by using address in multiple contexts */
    
    /* 1. Use GNU statement expression */
    TLS_ADDR_EXPR(local_tls_int);
    TLS_ADDR_EXPR(global_tls_int);
    
    /* 2. Take address and pass to noinline function */
    int *local_ptr = &local_tls_int;
    
    /* 3. Use in inline assembly to create complex context */
    __asm__ volatile (
        "# Main TLS usage %0 %1\n"
        : "+m" (local_tls_int), "+m" (global_tls_int)
        :
        : "memory"
    );
    
    /* 4. Call function that manipulates TLS addresses */
    manipulate_tls_addresses();
    
    /* 5. Complex computation using multiple TLS variables */
    int checksum = global_tls_int + local_tls_int + static_tls_int + common_tls_int;
    checksum += (int)global_tls_double;
    checksum += global_tls_long;
    
    /* Use external TLS variables */
    checksum += extern_tls_int;
    checksum += extern_tls_long;
    
    /* Prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional check */
    printf("External TLS int: %d, long: %ld\n", extern_tls_int, extern_tls_long);
    
    return checksum % 256;
}
