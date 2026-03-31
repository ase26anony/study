/* tls_main.c - Main file with TLS declarations and usage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
extern int compute_checksum(void);

/* External TLS declarations from other file */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* File-local static TLS with various attributes */
static __thread int static_tls_int __attribute__((used));
static __thread long static_tls_long __attribute__((visibility("hidden")));
__thread double global_tls_double __attribute__((weak));

/* TLS with DLL import attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls_int;
#else
/* Simulate similar attribute on non-Windows */
__thread int dllimport_tls_int __attribute__((visibility("default")));
#endif

/* Non-inline function that forces address taking */
static __attribute__((noinline)) 
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &static_tls_int;
    long *p2 = &static_tls_long;
    double *p3 = &global_tls_double;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
    
    /* Call external function with TLS addresses */
    use_tls_pointers(p1, p2, p3);
}

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_values(void) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    /* Initialize with non-constant values */
    static_tls_int = rand() % 1000;
    static_tls_long = rand() % 10000;
    global_tls_double = (double)(rand() % 1000) / 10.0;
    
    /* Initialize external TLS through function call */
    extern_tls_int = rand() % 500;
}

/* Function that creates complex context for TLS declaration */
static int complex_tls_usage(void) {
    /* Local TLS variable in function scope */
    __thread int local_tls_int __attribute__((used));
    local_tls_int = rand() % 200;
    
    /* GNU statement expression with TLS address */
    int result = ({
        int *addr = &local_tls_int;
        int temp = *addr + extern_tls_int;
        
        /* Inline assembly to force declaration duplication */
        __asm__ volatile (
            "# TLS variable reference %0" 
            : "+r" (temp) 
            : "m" (local_tls_int)
        );
        
        temp;
    });
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Force TLS declaration duplication through multiple paths */
    
    /* 1. Take address in main scope */
    __thread int main_tls_int = 42;
    int *main_tls_ptr = &main_tls_int;
    
    /* 2. Use in statement expression */
    int val = ({
        int x = main_tls_int + static_tls_int;
        int *ptr = &main_tls_int;
        __asm__ volatile ("# Statement expr TLS %0" : "+r"(x) : "m"(*ptr));
        x;
    });
    
    /* 3. Call function that manipulates TLS addresses */
    manipulate_tls_addresses();
    
    /* 4. Complex TLS usage */
    checksum += complex_tls_usage();
    
    /* 5. Multiple uses of TLS variables to ensure TREE_USED is set */
    checksum += static_tls_int;
    checksum += (int)static_tls_long;
    checksum += (int)global_tls_double;
    checksum += extern_tls_int;
    checksum += val;
    
    /* Compute final checksum using external function */
    checksum += compute_checksum();
    
    printf("TLS checksum: %d\n", checksum);
    
    /* Use DLL import TLS if available */
#ifdef _WIN32
    checksum += dllimport_tls_int;
#else
    /* Access to ensure it's marked used */
    checksum += (int)((long)&dllimport_tls_int % 1000);
#endif
    
    return checksum == 0 ? 0 : 1;
}
