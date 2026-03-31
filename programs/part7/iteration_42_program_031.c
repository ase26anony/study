#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations for TLS variables defined elsewhere */
extern __thread int external_tls;
extern __thread int weak_tls;
extern __thread int common_tls;
extern __thread int imported_tls;

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable definition (will be overridden if strong exists) */
__thread __attribute__((weak)) int weak_tls = 0;

/* Function prototypes from other files */
void init_tls_vars(int seed);
int compute_tls_sum(void);
void modify_tls_from_other_unit(void);

/* Opaque function to prevent optimization */
static void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function that takes address of TLS variables */
static void take_addresses(void) {
    volatile void *addrs[] = {
        (void*)&public_tls,
        (void*)&static_tls,
        (void*)&weak_tls,
        (void*)&external_tls,
        (void*)&common_tls,
#ifdef _WIN32
        (void*)&imported_tls,
#endif
        NULL
    };
    (void)addrs;
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    public_tls = rand() % 100;
    static_tls = argc + rand() % 50;
    weak_tls = seed % 77;
    
    /* Call functions from other compilation units */
    init_tls_vars(seed * 2);
    modify_tls_from_other_unit();
    
    /* Force usage of all TLS variables */
    take_addresses();
    
    /* Compute checksum using all accessible TLS variables */
    int sum = compute_tls_sum();
    sum += public_tls + static_tls + weak_tls;
    
    /* Use external variables if available */
    sum += external_tls;
    sum += common_tls;
    
    /* Print result to ensure side effects */
    printf("TLS checksum: %d (seed: %d)\n", sum % 1000, seed);
    
    return (sum % 256);
}
#else
int main(void) {
    printf("GCC TLS not supported\n");
    return 0;
}
#endif
