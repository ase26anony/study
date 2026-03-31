/* tls_main.c - Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern int __thread extern_tls_int;
extern double __thread extern_tls_double;
static void __attribute__((noinline)) use_tls_pointers(int *a, long *b, double *c);

/* TLS declarations with various attributes and linkages */

/* Static TLS with hidden visibility */
static __thread int static_tls_int __attribute__((used, visibility("hidden")));

/* Weak TLS variable that might be overridden */
__thread long weak_tls_long __attribute__((weak, visibility("default")));

/* Regular TLS with default visibility */
__thread int regular_tls_int;

/* TLS with DLL import attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls_int;
#else
/* Simulate similar behavior with weak attribute */
__thread int dllimport_tls_int __attribute__((weak));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor)) 
static void init_tls_vars(void) {
    srand(time(NULL));
    static_tls_int = rand() % 1000;
    weak_tls_long = rand() % 10000;
    regular_tls_int = rand() % 500;
    dllimport_tls_int = rand() % 2000;
    
    /* Force TREE_USED to be set by accessing the variables */
    (void)static_tls_int;
    (void)weak_tls_long;
    (void)regular_tls_int;
    (void)dllimport_tls_int;
}

/* Helper function that takes TLS variable addresses */
static __attribute__((noinline)) 
void use_tls_pointers(int *a, long *b, double *c) {
    /* Opaque operations to prevent optimization */
    __asm__ volatile ("" : : "r"(a), "r"(b), "r"(c) : "memory");
    
    /* Perform actual operations to ensure variables are used */
    if (a) *a += 1;
    if (b) *b += 2;
    if (c) *c += 3.0;
}

/* Function that creates complex context for TLS declarations */
static int process_tls_vars(void) {
    /* Local TLS variable - creates new declaration context */
    __thread int local_tls_int = 42;
    
    /* Take address of TLS variables */
    int *p1 = &static_tls_int;
    long *p2 = &weak_tls_long;
    double *p3 = &extern_tls_double;
    int *p4 = &local_tls_int;
    
    /* Use GNU statement expression to create complex context */
    int result = ({
        /* Reference TLS variables in statement expression */
        int sum = static_tls_int + weak_tls_long + regular_tls_int;
        
        /* Take address within statement expression - may trigger duplication */
        int *addr = &local_tls_int;
        
        /* Call noinline function with TLS addresses */
        use_tls_pointers(&static_tls_int, &weak_tls_long, &extern_tls_double);
        
        /* Complex computation using TLS variables */
        sum += *addr + extern_tls_int;
        sum;
    });
    
    /* Additional uses to ensure all attributes are considered */
    {
        /* Inline assembly referencing TLS variables */
        __asm__ volatile (
            "addl %1, %0\n\t"
            : "+r"(result)
            : "r"(dllimport_tls_int)
            : "cc"
        );
    }
    
    return result;
}

/* Another function that uses TLS in different ways */
static void manipulate_tls(void) {
    /* Array of pointers to TLS variables */
    void *tls_pointers[] = {
        &static_tls_int,
        &weak_tls_long,
        &regular_tls_int,
        &dllimport_tls_int,
        &extern_tls_int,
        &extern_tls_double
    };
    
    /* Use inline assembly to force address taking */
    for (int i = 0; i < sizeof(tls_pointers)/sizeof(tls_pointers[0]); i++) {
        __asm__ volatile ("" : : "r"(tls_pointers[i]) : "memory");
    }
}

int main(void) {
    int checksum = 0;
    
    /* Initialize extern TLS variables */
    extern_tls_int = 100;
    extern_tls_double = 3.14159;
    
    /* Process TLS variables multiple times */
    for (int i = 0; i < 3; i++) {
        checksum += process_tls_vars();
        manipulate_tls();
        
        /* Modify TLS variables */
        static_tls_int += i;
        weak_tls_long *= (i + 1);
        regular_tls_int -= i;
        dllimport_tls_int ^= i;
        extern_tls_int += 10;
        extern_tls_double += 0.1;
    }
    
    /* Compute final checksum */
    checksum += static_tls_int + weak_tls_long + regular_tls_int + 
                dllimport_tls_int + extern_tls_int + (int)extern_tls_double;
    
    printf("TLS checksum: %d\n", checksum);
    
    /* Ensure all TLS variables are marked as used */
    volatile int force_use = 
        static_tls_int + weak_tls_long + regular_tls_int + 
        dllimport_tls_int + extern_tls_int + (int)extern_tls_double;
    (void)force_use;
    
    return 0;
}
