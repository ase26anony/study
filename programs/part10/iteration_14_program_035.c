/* tls_main.c - Main test file for TLS declaration duplication */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for external TLS variables */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* Global TLS variables with various attributes */
__thread int global_tls_int __attribute__((used, visibility("default")));
__thread static int static_tls_int __attribute__((visibility("hidden")));
__thread double global_tls_double __attribute__((weak));

/* DLL import simulation (non-Windows fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_int;
#else
/* Simulate DLL import attribute for non-Windows */
__thread int imported_tls_int __attribute__((visibility("default")));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor)) 
static void init_tls_vars(void) {
    srand(time(NULL));
    global_tls_int = rand() % 1000;
    static_tls_int = rand() % 1000;
    global_tls_double = (double)rand() / RAND_MAX;
}

/* Opaque function that forces address taking */
static __attribute__((noinline, used))
void manipulate_tls_pointers(int *p1, long *p2, double *p3) {
    /* Use inline assembly to prevent optimization */
    asm volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
    
    /* Perform opaque operations */
    if (p1) *p1 += 42;
    if (p2) *p2 += 100;
    if (p3) *p3 *= 2.0;
}

/* Function that creates complex context for TLS declarations */
static __attribute__((noinline))
int complex_tls_expression(void) {
    /* Use GNU statement expression with TLS variable */
    int result = ({
        __thread int local_stmt_tls __attribute__((used)) = 0;
        int *addr = &local_stmt_tls;
        
        /* Force declaration duplication through multiple contexts */
        asm volatile ("# TLS statement expression" : : "r"(addr));
        
        /* Call external function */
        extern void external_func(void);
        external_func();
        
        local_stmt_tls = 123;
        local_stmt_tls;
    });
    
    return result;
}

/* Function that triggers declaration cloning */
static void trigger_tls_cloning(void) {
    /* Take addresses of various TLS variables */
    int *addr1 = &global_tls_int;
    int *addr2 = &static_tls_int;
    long *addr3 = &extern_tls_long;
    double *addr4 = &global_tls_double;
    
    /* Pass to opaque function */
    manipulate_tls_pointers(addr1, (long*)addr2, addr4);
    
    /* Use in inline assembly to force declaration reference */
    asm volatile (
        "# TLS address references\n\t"
        : : "r"(addr1), "r"(addr2), "r"(addr3), "r"(addr4)
    );
    
    /* Complex expression with TLS */
    int complex_result = complex_tls_expression();
    
    /* Use all TLS variables in computation */
    int checksum = global_tls_int + static_tls_int + (int)global_tls_double;
    checksum += complex_result;
    
    /* Prevent dead code elimination */
    asm volatile ("# Checksum: %0" : : "r"(checksum));
}

int main(void) {
    /* Local TLS variable in main */
    __thread int local_main_tls __attribute__((used)) = 999;
    
    /* Initialize external TLS (defined in another file) */
    extern_tls_int = 555;
    
    /* Trigger TLS declaration cloning */
    trigger_tls_cloning();
    
    /* Use TLS variables in various ways */
    int *local_addr = &local_main_tls;
    
    /* Force declaration duplication through multiple scopes */
    {
        __thread int block_tls __attribute__((visibility("hidden"))) = 777;
        int *block_addr = &block_tls;
        asm volatile ("# Block TLS" : : "r"(block_addr));
    }
    
    /* Compute final result using all TLS variables */
    int final_result = 
        global_tls_int + 
        static_tls_int + 
        extern_tls_int + 
        local_main_tls + 
        (int)global_tls_double;
    
    printf("TLS test result: %d\n", final_result);
    
    /* Prevent optimization */
    volatile int sink = final_result;
    
    return 0;
}
