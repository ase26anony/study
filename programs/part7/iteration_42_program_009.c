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

/* External TLS declaration (defined in tls_lib1.c) */
extern __thread int external_tls;

/* Common TLS variable (no initializer) */
__thread int common_tls;

/* DLL import simulation (for Windows/MinGW) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, just declare as external */
extern __thread int imported_tls;
#endif

/* Function prototypes from other files */
void init_tls_vars(int seed);
int compute_tls_sum(void);
void use_common_tls(int val);

/* Opaque function to prevent optimization */
static void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Take address to ensure TREE_USED */
static void take_addresses(void) {
    use_value((int)&public_tls);
    use_value((int)&static_tls);
    use_value((int)&weak_tls);
    use_value((int)&external_tls);
    use_value((int)&common_tls);
#ifdef _WIN32
    use_value((int)&imported_tls);
#endif
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize with non-constant values */
    public_tls = rand() % 100;
    static_tls = argc + (rand() % 50);
    weak_tls = seed % 77;
    common_tls = (public_tls + static_tls) % 99;
    
    /* Call functions from other compilation units */
    init_tls_vars(seed * 2);
    
    /* Use addresses to prevent optimization */
    take_addresses();
    
    /* Compute and print checksum */
    int sum = compute_tls_sum();
    sum += public_tls + static_tls + weak_tls + common_tls;
    
    /* Use printf with %p to force variable usage */
    printf("TLS addresses: public=%p, static=%p\n", 
           (void*)&public_tls, (void*)&static_tls);
    printf("Final checksum: %d\n", sum % 1000);
    
    /* Use common TLS variable */
    use_common_tls(sum);
    
    return sum % 256;
}
#else
int main(void) {
    printf("GCC __thread not supported on this compiler\n");
    return 0;
}
#endif
