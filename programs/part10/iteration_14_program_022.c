/* tls_main.c - Main file with TLS declarations and usage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, int *b, double *c, long *d);
extern int compute_checksum(void);

/* External TLS declarations from other TU */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* File-local static TLS with various attributes */
static __thread int static_tls_int __attribute__((used)) = 42;
static __thread double static_tls_double __attribute__((visibility("hidden"))) = 3.14159;

/* TLS with explicit visibility */
__thread long global_tls_long __attribute__((visibility("default"))) = 100;

/* Weak TLS declaration */
__thread int weak_tls_int __attribute__((weak)) = 0;

/* DLL import simulation (with fallback for non-Windows) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_int;
#else
/* Simulate DLL import attribute with a custom attribute */
#define __dllimport __attribute__((dllimport))
__thread int imported_tls_int __attribute__((dllimport));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor)) 
static void init_tls_values(void) {
    srand(time(NULL));
    static_tls_int = rand() % 1000;
    static_tls_double = (double)rand() / RAND_MAX * 100.0;
    global_tls_long = rand() % 10000;
    
    if (!__builtin_constant_p(weak_tls_int)) {
        weak_tls_int = rand() % 500;
    }
}

/* Noinline helper to force address taking and potential declaration cloning */
static __attribute__((noinline, used)) 
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &static_tls_int;
    double *p2 = &static_tls_double;
    long *p3 = &global_tls_long;
    int *p4 = &weak_tls_int;
    
    /* Use inline assembly to create opaque context for TLS addresses */
    /* This may trigger declaration duplication in the compiler */
    __asm__ volatile (
        "# TLS address manipulation\n\t"
        : "+r" (p1), "+r" (p2)
        : "r" (p3), "r" (p4)
        : "memory"
    );
    
    /* Complex statement expression with TLS address usage */
    int result = ({
        int sum = *p1 + (int)*p2 + (int)*p3 + *p4;
        
        /* Nested use of TLS variable address */
        __thread int local_tls_in_expr __attribute__((used)) = sum;
        int *local_ptr = &local_tls_in_expr;
        
        /* More inline assembly to prevent optimization */
        __asm__ volatile (
            "# Statement expr TLS\n\t"
            : "+r" (local_ptr)
            :: "memory"
        );
        
        *local_ptr + sum;
    });
    
    /* Prevent dead code elimination */
    __asm__ volatile (
        "# Opaque use of result\n\t"
        :: "r" (result)
        : "memory"
    );
}

/* Another function that takes TLS addresses as parameters */
static __attribute__((noinline))
void process_tls_pointers(int *a, double *b, long *c) {
    /* Complex operations that might trigger declaration cloning */
    *a = (*a * 2) % 1000;
    *b = *b * 1.5;
    *c = *c + (*a % 100);
    
    /* Use GNU statement expression with local TLS */
    __thread int temp_tls __attribute__((used)) = 0;
    int val = ({
        temp_tls = *a + (int)*b + *c;
        int *ptr = &temp_tls;
        
        /* Force address usage in assembly */
        __asm__ volatile (
            "# Process TLS pointers\n\t"
            : "+r" (ptr)
            :: "memory"
        );
        
        temp_tls;
    });
    
    (void)val; /* Use val to prevent warning */
}

int main(void) {
    /* Local TLS variable in main */
    __thread int main_local_tls __attribute__((used)) = 123;
    
    /* Take address and use in complex expression */
    int *main_tls_ptr = &main_local_tls;
    
    /* Trigger TLS declaration duplication through various mechanisms */
    
    /* 1. Direct manipulation */
    manipulate_tls_addresses();
    
    /* 2. Pass TLS addresses to noinline function */
    process_tls_pointers(&static_tls_int, &static_tls_double, &global_tls_long);
    
    /* 3. Use statement expression with external function call */
    int complex_result = ({
        /* Local TLS inside statement expression */
        __thread int stmt_tls __attribute__((used)) = 0;
        stmt_tls = static_tls_int + main_local_tls;
        
        /* Take address and use in inline asm */
        int *stmt_ptr = &stmt_tls;
        __asm__ volatile (
            "# Statement expr TLS manipulation\n\t"
            : "+r" (stmt_ptr)
            :: "memory"
        );
        
        /* Call external function with TLS addresses */
        use_tls_pointers(&static_tls_int, &weak_tls_int, 
                        &static_tls_double, &global_tls_long);
        
        stmt_tls;
    });
    
    /* 4. Multiple uses of same TLS variable in different contexts */
    for (int i = 0; i < 3; i++) {
        /* Each iteration creates new context for TLS usage */
        __thread int loop_tls __attribute__((used)) = i * 100;
        int *loop_ptr = &loop_tls;
        
        /* Different inline asm each iteration */
        __asm__ volatile (
            "# Loop TLS use %0\n\t"
            : "+r" (loop_ptr)
            :: "memory"
        );
        
        main_local_tls += loop_tls;
    }
    
    /* Compute and print checksum to prevent optimization */
    int checksum = compute_checksum();
    printf("TLS checksum: %d\n", checksum);
    printf("Main local TLS: %d\n", main_local_tls);
    printf("Static TLS int: %d\n", static_tls_int);
    printf("Global TLS long: %ld\n", global_tls_long);
    
    return 0;
}
