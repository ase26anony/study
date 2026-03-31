#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __GNUC__
/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* External TLS declaration (defined in test_lib1.c) */
extern __thread int external_tls;

/* Common symbol TLS (no initializer) */
__thread int common_tls;

/* DLL import simulation for Windows targets */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls = 0;
#endif

/* Function prototypes from other files */
void init_tls_vars(int seed);
int compute_tls_sum(void);
void modify_tls_from_lib2(void);

/* Opaque function to prevent optimization */
static void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Take address to ensure TREE_USED is set */
static void take_addresses(void) {
    volatile void *addrs[] = {
        (void*)&public_tls,
        (void*)&static_tls,
        (void*)&weak_tls,
        (void*)&external_tls,
        (void*)&common_tls,
        (void*)&imported_tls
    };
    (void)addrs;
}
#endif /* __GNUC__ */

int main(int argc, char *argv[]) {
#ifdef __GNUC__
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    public_tls = rand() % 100;
    static_tls = rand() % 100;
    weak_tls = rand() % 100;
    common_tls = rand() % 100;
    imported_tls = rand() % 100;
    
    /* Call functions from other compilation units */
    init_tls_vars(seed * 2);
    modify_tls_from_lib2();
    
    /* Use the variables in non-trivial ways */
    take_addresses();
    
    /* Perform arithmetic operations */
    public_tls += static_tls * 2;
    weak_tls = public_tls % 17;
    common_tls ^= imported_tls;
    
    /* Compute final checksum */
    int sum = compute_tls_sum();
    sum += public_tls + static_tls + weak_tls + common_tls + imported_tls;
    
    /* Print using opaque function (printf) */
    printf("TLS checksum: %d (seed: %d)\n", sum % 1000, seed);
    
    /* Ensure all variables are marked as used */
    use_value(public_tls);
    use_value(static_tls);
    use_value(weak_tls);
    use_value(common_tls);
    use_value(imported_tls);
    
    return sum % 256;
#else
    printf("GCC TLS not supported on this compiler\n");
    return 0;
#endif
}
