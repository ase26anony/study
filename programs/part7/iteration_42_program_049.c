#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer to encourage common symbol behavior) */
__thread int common_tls;

/* DLL import simulation (for Windows/MinGW targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, simulate with external declaration */
extern __thread int imported_tls;
#endif

/* External TLS variable declaration (will be defined in another file) */
extern __thread int external_tls;

/* Function prototypes */
void init_tls_vars(int seed);
void modify_tls_vars(void);
int compute_tls_checksum(void);
void use_tls_from_other_unit(void);

/* Opaque function to prevent optimization */
static void use_value(int val) {
    /* Use volatile to prevent optimization */
    volatile int dummy = val;
    (void)dummy;
}

/* Function that takes addresses of TLS variables */
static void take_addresses(void) {
    /* Taking addresses ensures TREE_USED is set */
    int *p1 = &public_tls;
    int *p2 = &static_tls;
    int *p3 = &weak_tls;
    int *p4 = &common_tls;
    
    use_value((int)(long)p1);
    use_value((int)(long)p2);
    use_value((int)(long)p3);
    use_value((int)(long)p4);
}

int main(int argc, char **argv) {
    /* Use argv to get non-constant initialization values */
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    init_tls_vars(seed);
    
    /* Take addresses to ensure variables are marked used */
    take_addresses();
    
    /* Modify TLS variables through function calls */
    modify_tls_vars();
    
    /* Use TLS variables from another compilation unit */
    use_tls_from_other_unit();
    
    /* Compute and print checksum to ensure all variables are used */
    int checksum = compute_tls_checksum();
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    return checksum != 0 ? 0 : 1;
}

/* Initialize TLS variables with non-constant values */
void init_tls_vars(int seed) {
    public_tls = seed % 100;
    static_tls = (seed * 3) % 100;
    weak_tls = (seed * 5) % 100;
    common_tls = (seed * 7) % 100;
    
    /* Mark as used */
    TREE_USED(&public_tls);
    TREE_USED(&static_tls);
    TREE_USED(&weak_tls);
    TREE_USED(&common_tls);
}

/* Modify TLS variables */
void modify_tls_vars(void) {
    public_tls += rand() % 10;
    static_tls -= rand() % 10;
    weak_tls *= (rand() % 3) + 1;
    common_tls ^= rand() % 256;
    
    /* Complex enough to prevent optimization */
    if (public_tls > 1000) {
        static_tls = public_tls / 2;
    }
}

/* Compute checksum using all TLS variables */
int compute_tls_checksum(void) {
    int sum = 0;
    
    sum += public_tls;
    sum += static_tls;
    sum += weak_tls;
    sum += common_tls;
    
    /* Try to access external TLS variable */
    extern int external_tls;
    sum += external_tls;
    
    return sum % 1000;
}

#else
int main(void) {
    printf("GCC TLS not supported on this compiler\n");
    return 0;
}
#endif
