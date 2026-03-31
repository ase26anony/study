/* Main file with various TLS declarations and attribute manipulations */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
static __attribute__((noinline)) void opaque_operation(void *ptr1, void *ptr2);

/* External TLS declaration (will be defined in another file) */
extern __thread int extern_tls_var __attribute__((weak, visibility("default")));

/* File-local static TLS with attributes */
static __thread int static_tls __attribute__((used, visibility("hidden"))) = 42;

/* TLS with common linkage potential */
__thread long common_tls __attribute__((visibility("default")));

/* TLS with DLL import simulation (using declspec for compatibility) */
#ifdef _WIN32
__declspec(dllimport) __thread double dllimport_tls;
#else
/* Simulate similar attributes for non-Windows */
__thread double dllimport_tls __attribute__((visibility("default")));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor)) 
static void init_tls_values(void) {
    srand(time(NULL));
    static_tls = rand() % 100;
    common_tls = rand() % 1000;
    
    /* Initialize extern_tls_var through function call */
    extern void init_extern_tls(void);
    init_extern_tls();
}

/* Helper function that forces address taking and complex contexts */
static __attribute__((noinline, used))
void opaque_operation(void *ptr1, void *ptr2) {
    /* Use inline assembly to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2) : "memory");
}

/* Function that uses TLS variables in complex ways */
static long compute_checksum(void) {
    /* Local TLS variable - may trigger declaration cloning */
    __thread int local_tls __attribute__((used)) = 123;
    
    /* Take addresses of multiple TLS variables */
    int *p1 = &static_tls;
    long *p2 = &common_tls;
    double *p3 = &dllimport_tls;
    int *p4 = &local_tls;
    extern int *get_extern_tls_addr(void);
    int *p5 = get_extern_tls_addr();
    
    /* Force duplication through statement expression with address use */
    long result = ({
        /* Complex context that may cause declaration cloning */
        int temp = *p1 + *p4;
        opaque_operation(p1, p4);
        
        /* Use inline assembly with TLS address */
        asm volatile ("# TLS address used: %0" : : "r"(p4));
        
        (long)(temp + *p2 + (long)(*p3));
    });
    
    /* More address passing to external function */
    use_tls_pointers(p1, p2, p3);
    
    /* Additional use that may trigger cloning during optimization */
    volatile int *volatile_p = &local_tls;
    asm volatile ("" : : "r"(volatile_p) : "memory");
    
    return result;
}

int main(void) {
    long checksum = compute_checksum();
    
    /* Use TLS variables in main to ensure they're marked used */
    static_tls += 1;
    common_tls += 2;
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: %ld\n", checksum);
    
    /* Additional complex use of local TLS */
    {
        __thread int another_local __attribute__((visibility("hidden"))) = 456;
        int *ptr = &another_local;
        
        /* Force address into asm */
        asm volatile ("# Another TLS: %0" : : "r"(ptr));
        
        /* Use in statement expression */
        int val = ({
            int x = *ptr;
            opaque_operation(ptr, &static_tls);
            x + static_tls;
        });
        printf("Combined value: %d\n", val);
    }
    
    return 0;
}
