/* Main file with various TLS declarations and usage patterns */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, int *b, double *c, long *d);
extern unsigned long compute_checksum(void);

/* External TLS declarations from other translation unit */
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

/* Weak TLS variable */
__thread long weak_tls_long __attribute__((weak));

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_vars(void) {
    srand(time(NULL));
    static_tls_int = rand() % 1000;
    global_tls_double = (double)rand() / RAND_MAX * 100.0;
    if (&weak_tls_long) {
        weak_tls_long = rand() % 5000;
    }
}

/* Noinline function to force address taking and potential declaration cloning */
static __attribute__((noinline)) 
void manipulate_tls_addresses(void) {
    /* Take addresses of TLS variables in various contexts */
    int *p1 = &static_tls_int;
    double *p2 = &global_tls_double;
    long *p3 = &weak_tls_long;
    
    /* Use inline assembly to force variable usage */
    __asm__ volatile (
        "# TLS variable addresses: %0, %1, %2"
        : 
        : "r" (p1), "r" (p2), "r" (p3)
        : "memory"
    );
    
    /* Complex statement expression with TLS address usage */
    int result = ({
        int temp = *p1 + (int)(*p2);
        /* Nested use of TLS variable address */
        long *inner_ptr = p3;
        temp += (*inner_ptr) % 256;
        temp;
    });
    
    (void)result; /* Prevent unused variable warning */
}

/* Another function that takes TLS addresses */
static __attribute__((noinline))
void another_tls_user(void) {
    /* Local TLS variable - may trigger declaration cloning */
    __thread int local_tls_int __attribute__((used)) = 42;
    
    /* Take address and use in inline assembly */
    int *local_ptr = &local_tls_int;
    
    __asm__ volatile (
        "# Local TLS address: %0\n"
        "addl $1, (%0)"
        : 
        : "r" (local_ptr)
        : "memory"
    );
    
    /* Pass TLS addresses to external function */
    use_tls_pointers(&static_tls_int, &local_tls_int, 
                     &global_tls_double, &weak_tls_long);
}

int main(void) {
    /* Call functions that manipulate TLS addresses */
    manipulate_tls_addresses();
    another_tls_user();
    
    /* Use statement expression with TLS variable */
    int complex_value = ({
        __thread int temp_tls __attribute__((used)) = 100;
        int *ptr = &temp_tls;
        
        /* Multiple uses of the same TLS variable */
        *ptr += static_tls_int;
        *ptr *= 2;
        
        /* Inline assembly reference */
        __asm__ volatile (
            "# Complex TLS usage: %0"
            : 
            : "r" (ptr)
            : "memory"
        );
        
        *ptr;
    });
    
    printf("Complex value: %d\n", complex_value);
    
    /* Compute and print checksum using all TLS variables */
    unsigned long checksum = compute_checksum();
    printf("TLS checksum: %lu\n", checksum);
    
    return 0;
}
