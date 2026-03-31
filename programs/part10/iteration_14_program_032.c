/* tls_main.c - Main file with TLS declarations and usage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern long get_checksum(void);
extern void use_tls_pointers(int *p1, long *p2, double *p3);

/* External TLS declarations from other TU */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* File-local static TLS with various attributes */
static __thread int static_tls_int __attribute__((used, visibility("hidden")));
__thread long global_tls_long __attribute__((visibility("default")));
__thread double global_tls_double __attribute__((weak));

/* DLL import simulation (non-Windows fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_int;
#else
/* Simulate DLL import attribute */
__thread int imported_tls_int __attribute__((dllimport));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_values(void) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    static_tls_int = rand() % 1000;
    global_tls_long = rand() % 10000;
    global_tls_double = (double)rand() / RAND_MAX * 100.0;
    
    printf("Initialized TLS values: %d, %ld, %.2f\n", 
           static_tls_int, global_tls_long, global_tls_double);
}

/* Noinline helper to force address taking */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses and use in opaque ways */
    int *p1 = &static_tls_int;
    long *p2 = &global_tls_long;
    double *p3 = &global_tls_double;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
    
    /* Pass to external function */
    use_tls_pointers(p1, p2, p3);
}

/* Complex statement expression using TLS */
static long tls_statement_expr(void) {
    /* GNU statement expression that creates new context for TLS */
    long result = ({
        __thread int local_stmt_tls __attribute__((used)) = 42;
        int *addr = &local_stmt_tls;
        /* Force declaration duplication through complex use */
        __asm__ volatile ("# TLS in statement expr %0" : : "r"(addr) : "memory");
        local_stmt_tls * 2 + extern_tls_int;
    });
    
    return result;
}

int main(void) {
    /* Local TLS variable in main */
    __thread int main_local_tls __attribute__((used)) = 123;
    
    /* Force address taking with inline assembly */
    int *main_tls_addr = &main_local_tls;
    __asm__ volatile ("# Main TLS addr %0" : : "r"(main_tls_addr) : "memory");
    
    /* Manipulate various TLS addresses */
    manipulate_tls_addresses();
    
    /* Use statement expression */
    long stmt_result = tls_statement_expr();
    printf("Statement expr result: %ld\n", stmt_result);
    
    /* Complex computation using multiple TLS variables */
    int *static_addr = &static_tls_int;
    long *global_addr = &global_tls_long;
    
    /* Force declaration duplication through multiple address uses */
    __asm__ volatile (
        "# Complex TLS use %0 %1"
        : 
        : "r"(static_addr), "r"(global_addr)
        : "memory"
    );
    
    /* Compute checksum */
    long checksum = get_checksum();
    printf("TLS checksum: %ld\n", checksum);
    
    /* Additional use to prevent optimization */
    volatile int dummy = main_local_tls + static_tls_int;
    (void)dummy;
    
    return 0;
}
