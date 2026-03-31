/* Main file with various TLS declarations and attribute manipulations */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
extern int compute_checksum(void);

/* External TLS variable defined in another file */
extern __thread int extern_tls_var;

/* TLS with various attributes to set all the flags in the uncovered block */

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls_var = 42;

/* TLS with explicit visibility */
__thread __attribute__((visibility("hidden"))) int hidden_tls_var;

/* TLS marked as used to ensure TREE_USED is set */
__thread __attribute__((used)) int used_tls_var;

/* Static TLS (file-local) */
static __thread int static_tls_var;

/* TLS with common linkage */
__thread int common_tls_var;

/* For Windows DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_var;
#else
/* Simulate similar attribute */
__thread int imported_tls_var __attribute__((visibility("default")));
#endif

/* Constructor to initialize TLS variables with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    hidden_tls_var = rand() % 100;
    used_tls_var = rand() % 100;
    static_tls_var = rand() % 100;
    common_tls_var = rand() % 100;
    imported_tls_var = rand() % 100;
}

/* Noinline function that takes TLS variable addresses */
static __attribute__((noinline))
void manipulate_tls_addresses(void) {
    /* Take addresses of TLS variables - may trigger declaration duplication */
    int *p1 = &weak_tls_var;
    long *p2 = (long*)&hidden_tls_var;  /* Type punning to get long* */
    double *p3 = (double*)&used_tls_var; /* Type punning to get double* */
    
    /* Use inline assembly to force address usage */
    __asm__ volatile (
        "# TLS address usage %0, %1, %2"
        : 
        : "r" (p1), "r" (p2), "r" (p3)
        : "memory"
    );
    
    /* Pass to external function */
    use_tls_pointers(p1, p2, p3);
}

/* Another function that creates complex context for TLS variables */
static int complex_tls_usage(void) {
    /* GNU statement expression with TLS address usage */
    int result = ({
        int local_tls __thread = 123;  /* Local TLS declaration */
        int *addr = &local_tls;
        
        /* Mix with external TLS */
        *addr += extern_tls_var;
        
        /* Use in inline assembly */
        __asm__ volatile (
            "addl %1, %0\n\t"
            : "+r" (*addr)
            : "r" (weak_tls_var)
            : "cc"
        );
        
        /* Final value */
        *addr;
    });
    
    return result;
}

int main(void) {
    /* Initialize external TLS variable */
    extern_tls_var = rand() % 1000;
    
    /* Call function that manipulates TLS addresses */
    manipulate_tls_addresses();
    
    /* Use complex TLS usage */
    int complex_result = complex_tls_usage();
    
    /* Compute checksum using all TLS variables */
    int checksum = compute_checksum();
    
    /* Use TLS variables in arithmetic to prevent optimization */
    int total = weak_tls_var + hidden_tls_var + used_tls_var + 
                static_tls_var + common_tls_var + imported_tls_var +
                extern_tls_var + complex_result;
    
    printf("TLS checksum: %d, Total: %d\n", checksum, total);
    
    /* Take address of TLS variable in main - another context */
    int *main_tls_addr = &extern_tls_var;
    
    /* Use in volatile assembly to prevent dead code elimination */
    __asm__ volatile (
        "# Final TLS reference %0"
        :
        : "r" (main_tls_addr)
        : "memory"
    );
    
    return 0;
}
