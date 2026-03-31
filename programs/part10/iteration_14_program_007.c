/* Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern __thread int extern_tls_var;
__thread int global_tls_var __attribute__((used, visibility("default")));
static __thread int static_tls_var __attribute__((weak, used));

/* Weak TLS declaration that might be overridden */
__thread long weak_tls_var __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread double hidden_tls __attribute__((visibility("hidden"), used));

/* DLL import simulation (non-Windows fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate DLL import attribute for non-Windows */
__thread int imported_tls __attribute__((dllimport));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_vars(void) {
    srand(time(NULL));
    global_tls_var = rand() % 1000;
    static_tls_var = rand() % 500;
    hidden_tls = (double)rand() / RAND_MAX * 100.0;
    
    /* Initialize extern variable if we're the defining module */
    extern int extern_tls_var;
    extern_tls_var = rand() % 2000;
}

/* Opaque function that forces address taking */
static __attribute__((noinline, used))
void manipulate_tls_pointers(int *p1, long *p2, double *p3) {
    /* Use inline assembly to prevent optimization */
    asm volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
    
    /* Perform opaque operations */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 0.5;
}

/* Function that creates complex context for TLS variables */
static __attribute__((noinline))
int complex_tls_usage(void) {
    /* Local TLS variable - may trigger declaration cloning */
    __thread int local_tls __attribute__((used)) = 42;
    
    /* Take address in statement expression */
    int result = ({
        int *addr = &local_tls;
        *addr += global_tls_var;
        
        /* Nested statement expression with external call */
        int temp = ({
            extern int printf(const char *, ...);
            temp = *addr + static_tls_var;
            temp;
        });
        
        /* Use address in assembly */
        asm volatile ("# TLS address: %0" : : "r"(addr));
        temp;
    });
    
    /* Force address taking of multiple TLS variables */
    manipulate_tls_pointers(&local_tls, &weak_tls_var, &hidden_tls);
    
    return result;
}

/* Function that uses GNU statement expressions extensively */
static __attribute__((noinline))
long statement_expr_tls(void) {
    /* Multiple statement expressions using TLS addresses */
    long val1 = ({
        __thread int inner_tls = 10;
        int *p = &inner_tls;
        asm volatile ("# Inner TLS: %0" : : "r"(p));
        (long)(*p * 2);
    });
    
    long val2 = ({
        /* Reference extern TLS */
        extern int extern_tls_var;
        int *p = &extern_tls_var;
        (long)(*p + val1);
    });
    
    return val1 + val2;
}

int main(void) {
    int checksum = 0;
    
    /* Complex TLS usage */
    checksum += complex_tls_usage();
    
    /* Statement expression usage */
    checksum += statement_expr_tls();
    
    /* Direct manipulation of TLS variables */
    global_tls_var += 5;
    checksum += global_tls_var;
    
    /* Take address and use in inline assembly */
    __thread int main_local_tls = 99;
    int *tls_ptr = &main_local_tls;
    
    asm volatile (
        "# Main TLS pointer: %0\n\t"
        "addl $1, (%0)"
        : : "r"(tls_ptr) : "memory"
    );
    
    checksum += *tls_ptr;
    checksum += static_tls_var;
    
    /* Use weak TLS variable */
    if (&weak_tls_var) {
        checksum += weak_tls_var;
    }
    
    /* Reference imported TLS (may trigger special handling) */
    checksum += (int)hidden_tls;
    
    printf("TLS checksum: %d\n", checksum);
    return 0;
}
