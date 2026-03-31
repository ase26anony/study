#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations from other files */
extern void use_tls_variables(int seed);
extern int compute_tls_checksum(void);

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable - may be overridden by another definition */
__thread __attribute__((weak)) int weak_tls = 0;

/* External TLS variable - declared here, defined elsewhere */
extern __thread int external_tls;

/* Common TLS variable (when compiled with -fcommon) */
__thread int common_tls;

/* DLL import attribute simulation (for Windows/MinGW targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, we'll just use a regular TLS variable */
__thread int imported_tls = 0;
#endif

/* Function to initialize TLS variables with non-constant values */
void init_tls_variables(int seed) {
    /* Use seed to make initialization non-constant */
    public_tls = seed * 2;
    static_tls = seed + 1;
    weak_tls = seed * 3;
    /* external_tls is defined in another file */
    common_tls = seed * 4;
    imported_tls = seed * 5;
    
    /* Force TREE_USED to be set by taking addresses */
    volatile int *ptr1 = &public_tls;
    volatile int *ptr2 = &static_tls;
    volatile int *ptr3 = &weak_tls;
    volatile int *ptr5 = &common_tls;
    volatile int *ptr6 = &imported_tls;
    
    /* Use the pointers to prevent optimization */
    (void)ptr1;
    (void)ptr2;
    (void)ptr3;
    (void)ptr5;
    (void)ptr6;
}

/* Function that performs operations on TLS variables */
int manipulate_tls(void) {
    int result = 0;
    
    /* Perform arithmetic operations */
    public_tls += 1;
    static_tls *= 2;
    weak_tls -= 3;
    common_tls = public_tls + static_tls;
    imported_tls = weak_tls * 2;
    
    /* Mix operations to create dependencies */
    result = public_tls + static_tls + weak_tls + common_tls + imported_tls;
    
    /* Take addresses again to ensure usage */
    if (&public_tls && &static_tls && &weak_tls && &common_tls && &imported_tls) {
        result %= 1000;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed;
    
    /* Use command line argument or time for non-constant seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) % 100;
    }
    
    srand(seed);
    
    /* Initialize TLS variables */
    init_tls_variables(seed);
    
    /* Call function from another compilation unit */
    use_tls_variables(seed * 7);
    
    /* Manipulate TLS variables */
    int local_result = manipulate_tls();
    
    /* Compute checksum from all TLS variables */
    int checksum = compute_tls_checksum();
    
    /* Final aggregation */
    int final_result = (local_result + checksum) % 1000;
    
    /* Print result to ensure observable side effects */
    printf("TLS test result: %d (seed: %d)\n", final_result, seed);
    
    /* Print addresses to ensure variables are used */
    printf("Addresses: public=%p, static=%p, weak=%p, common=%p, imported=%p\n",
           (void*)&public_tls, (void*)&static_tls, (void*)&weak_tls,
           (void*)&common_tls, (void*)&imported_tls);
    
    return final_result != 0 ? 0 : 1;
}
#else
int main(void) {
    printf("GCC TLS not supported on this compiler\n");
    return 0;
}
#endif
