/* Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
extern int checksum(void);

/* External TLS declarations from another TU */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* TLS with various attributes */
__thread int global_tls_int __attribute__((used, visibility("default")));
static __thread int static_tls_int __attribute__((used));
__thread long global_tls_long __attribute__((weak, visibility("hidden")));
__thread double global_tls_double;

/* DLL import simulation (non-Windows fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_var;
#else
/* Simulate with alias attribute */
extern __thread int imported_tls_var __attribute__((weak));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    global_tls_int = rand() % 1000;
    static_tls_int = rand() % 1000;
    global_tls_long = rand() % 1000;
    global_tls_double = (double)rand() / RAND_MAX;
}

/* Noinline helper to force address taking */
static __attribute__((noinline))
void manipulate_tls_addresses(void) {
    /* Take addresses in various ways to potentially trigger duplication */
    int *p1 = &global_tls_int;
    long *p2 = &global_tls_long;
    double *p3 = &global_tls_double;
    
    /* Use inline assembly to create complex context */
    asm volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
    
    /* Use statement expression with TLS address */
    int result = ({
        int temp = *p1 + *p2;
        asm volatile ("" : "+r"(temp) : : "memory");
        temp;
    });
    
    (void)result;
}

/* Another noinline function that uses TLS variables */
__attribute__((noinline))
static void complex_tls_usage(void) {
    /* Create a local TLS variable in a function */
    __thread int local_func_tls __attribute__((used)) = 42;
    
    /* Use GNU statement expression with TLS variable */
    int val = ({
        int x = local_func_tls;
        int *ptr = &x;
        asm volatile ("" : "+r"(*ptr) : : "memory");
        x + global_tls_int;
    });
    
    /* Pass TLS addresses to external function */
    use_tls_pointers(&local_func_tls, &global_tls_long, &global_tls_double);
    
    /* Force preservation of declaration attributes */
    if (val > 0) {
        static_tls_int += val;
    }
}

int main(void) {
    /* Local TLS in main */
    __thread int local_main_tls __attribute__((used)) = 123;
    
    /* Take address and use in inline asm */
    int *local_ptr = &local_main_tls;
    asm volatile ("" : : "r"(local_ptr) : "memory");
    
    /* Call functions that manipulate TLS */
    manipulate_tls_addresses();
    complex_tls_usage();
    
    /* Use external TLS variables */
    extern_tls_int = 100;
    if (&extern_tls_long) {  /* Force reference for weak symbol */
        extern_tls_long = 200;
    }
    
    /* Compute and print checksum */
    int sum = checksum();
    printf("TLS checksum: %d\n", sum);
    
    return 0;
}
