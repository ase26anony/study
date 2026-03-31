#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable - may be overridden by another definition */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common symbol behavior - no initializer */
__thread int common_tls;

/* DLL import simulation (for Windows/MinGW targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls = 0;
#endif

/* External TLS variable declaration (defined elsewhere) */
extern __thread int external_tls;

/* Function prototypes */
void init_tls_vars(int seed);
int compute_checksum(void);
void use_tls_in_other_unit(void);

/* Opaque function to prevent optimization */
static void use_value(int val) {
    volatile int dummy = val;
    (void)dummy;
}

/* Initialize TLS variables with non-constant values */
void init_tls_vars(int seed) {
    srand(seed);
    
    /* Initialize with non-constant values */
    public_tls = rand() % 1000;
    static_tls = seed * 2;
    weak_tls = seed + 100;
    common_tls = seed * 3;
    
    /* imported_tls initialization depends on platform */
#ifndef _WIN32
    imported_tls = seed * 4;
#endif
    
    /* Mark all as used through various operations */
    TREE_USED simulation: take addresses, perform arithmetic
    int* ptr1 = &public_tls;
    int* ptr2 = &static_tls;
    int* ptr3 = &weak_tls;
    
    /* Use the values to prevent optimization */
    use_value(public_tls + static_tls);
    use_value(weak_tls * 2);
}

/* Compute checksum from all TLS variables */
int compute_checksum(void) {
    int sum = 0;
    
    /* Access all TLS variables in non-trivial ways */
    sum += public_tls;
    sum += static_tls * 2;
    sum += weak_tls / 3;
    sum += common_tls;
    
#ifndef _WIN32
    sum += imported_tls;
#endif
    
    /* External variable accessed through pointer indirection */
    if (&external_tls != NULL) {
        sum += 1;  /* Just to reference it */
    }
    
    /* More complex usage patterns */
    sum = (sum * 31) ^ public_tls;
    sum = (sum * 17) ^ static_tls;
    
    return sum & 0xFF;  /* Return byte-sized checksum */
}

int main(int argc, char** argv) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize random seed */
    srand(time(NULL) ^ seed);
    
    /* Initialize TLS variables */
    init_tls_vars(seed);
    
    /* Call function from another compilation unit */
    use_tls_in_other_unit();
    
    /* Compute and print checksum */
    int checksum = compute_checksum();
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional complex usage to ensure TREE_USED is set */
    public_tls += checksum;
    static_tls -= checksum;
    
    /* Take addresses and pass to printf for %p format */
    printf("Addresses - public: %p, static: %p, weak: %p\n", 
           (void*)&public_tls, (void*)&static_tls, (void*)&weak_tls);
    
    return checksum;
}

#else
int main(void) {
    printf("GCC TLS not supported\n");
    return 0;
}
#endif
