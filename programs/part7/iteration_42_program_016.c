#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations for TLS variables defined in other files */
extern __thread int external_tls;
extern __thread int weak_tls;
extern __thread int common_tls;

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable definition (will be overridden if strong definition exists elsewhere) */
__thread __attribute__((weak)) int weak_tls_def = 0;

/* Function prototypes */
void init_tls_vars(int seed);
int compute_checksum(void);
void use_tls_from_other_unit(void);

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    init_tls_vars(seed);
    
    /* Use TLS variables in non-trivial ways */
    public_tls += rand() % 100;
    static_tls = public_tls * 2;
    
    /* Take address to ensure TREE_USED is set */
    int *ptr1 = &public_tls;
    int *ptr2 = &static_tls;
    
    /* Call function from another compilation unit */
    use_tls_from_other_unit();
    
    /* Compute checksum using all accessible TLS variables */
    int checksum = compute_checksum();
    
    /* Print results to prevent optimization */
    printf("TLS Checksum: %d (seed: %d)\n", checksum, seed);
    printf("Addresses: public=%p, static=%p\n", (void*)ptr1, (void*)ptr2);
    
    return checksum % 256;
}

void init_tls_vars(int seed) {
    /* Initialize with values derived from seed to prevent constant folding */
    public_tls = (seed % 1000) + 1;
    static_tls = (seed % 500) + 1001;
    weak_tls_def = (seed % 300) + 500;
    
    /* Complex initialization to ensure variable is marked used */
    for (int i = 0; i < 10; i++) {
        public_tls += i;
        static_tls -= i;
    }
}

int compute_checksum(void) {
    int sum = 0;
    
    /* Access all TLS variables in a non-trivial computation */
    sum += public_tls;
    sum += static_tls;
    sum += weak_tls_def;
    
    /* External variables might be defined elsewhere */
    sum += external_tls;
    sum += weak_tls;
    sum += common_tls;
    
    /* Prevent optimization with opaque operation */
    volatile int temp = sum;
    return temp ^ 0x55AA55AA;
}
#else
int main(void) {
    printf("GCC __thread not supported on this compiler\n");
    return 0;
}
#endif
