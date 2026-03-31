/* tls_main.c - Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern long get_tls_sum(void);
extern void opaque_operation(int* a, long* b, double* c);

/* External TLS variable defined in another file */
extern __thread int extern_tls_var;

/* TLS with various attributes to set the flags in the uncovered block */
__thread int global_tls __attribute__((used)) = 42;
__thread long weak_tls __attribute__((weak)) = 100;
__thread double hidden_tls __attribute__((visibility("hidden"))) = 3.14;

/* Static TLS (file-local) */
static __thread int static_tls = 0;

/* TLS with explicit visibility */
__thread int default_visibility_tls __attribute__((visibility("default")));

/* For DLL import simulation (non-Windows fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Use weak attribute as alternative to simulate external linkage */
__thread int imported_tls __attribute__((weak));
#endif

/* Constructor to initialize TLS variables with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        static_tls = rand() % 100;
        default_visibility_tls = rand() % 256;
        hidden_tls = (double)rand() / RAND_MAX * 100.0;
        initialized = 1;
    }
}

/* Opaque function that takes TLS variable addresses */
static __attribute__((noinline, used))
void use_tls_addresses(void) {
    int* p1 = &global_tls;
    long* p2 = &weak_tls;
    double* p3 = &hidden_tls;
    int* p4 = &static_tls;
    
    /* Inline assembly to force address usage and prevent optimization */
    __asm__ volatile (
        "# TLS address usage\n"
        : "+r" (p1), "+r" (p2), "+r" (p3), "+r" (p4)
        :
        : "memory"
    );
    
    /* Call external opaque function */
    opaque_operation(p1, p2, p3);
}

/* Function that creates complex context for TLS variable */
static __attribute__((noinline))
int complex_tls_usage(void) {
    /* Local TLS variable - may trigger declaration in local scope */
    __thread int local_tls = 0;
    
    /* GNU statement expression with TLS address usage */
    int result = ({
        int temp = local_tls;
        int* addr = &local_tls;
        
        /* Force address computation */
        __asm__ volatile (
            "# Statement expression TLS\n"
            : "+r" (addr)
            :
            : "memory"
        );
        
        /* Use the TLS variable in computation */
        temp += *addr;
        temp += global_tls;
        temp;
    });
    
    return result;
}

/* Function that returns address of TLS variable - may cause duplication */
static __attribute__((noinline))
int* get_tls_address(void) {
    /* Taking address of TLS with attributes */
    static __thread int internal_tls __attribute__((used)) = 99;
    
    /* Complex expression with address taking */
    int* addr = &internal_tls;
    
    /* Prevent optimization */
    __asm__ volatile (
        "# Returning TLS address\n"
        : "+r" (addr)
        :
        : "memory"
    );
    
    return addr;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize extern TLS variable */
    extern_tls_var = 123;
    
    /* Use various TLS variables to ensure they're marked as used */
    checksum += global_tls;
    checksum += weak_tls;
    checksum += (int)hidden_tls;
    checksum += static_tls;
    checksum += default_visibility_tls;
    checksum += extern_tls_var;
    
    /* Force address taking and complex usage patterns */
    use_tls_addresses();
    
    /* Complex TLS usage in statement expression */
    checksum += complex_tls_usage();
    
    /* Get address of TLS variable - may trigger declaration cloning */
    int* tls_addr = get_tls_address();
    checksum += *tls_addr;
    
    /* Get sum from another translation unit */
    checksum += get_tls_sum();
    
    /* Use imported TLS if available */
    checksum += imported_tls;
    
    printf("TLS checksum: %d\n", checksum);
    
    return 0;
}
