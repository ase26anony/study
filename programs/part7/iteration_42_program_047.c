#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations from other files */
extern __thread int public_tls;
extern __thread int weak_tls;
extern __thread int external_tls;
extern __thread int common_tls;

/* Static TLS variable - private linkage */
static __thread int static_tls;

/* DLL import simulation (for Windows/MinGW targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, just declare as regular TLS */
__thread int imported_tls;
#endif

/* Function prototypes from other files */
void init_tls_vars(int seed);
int compute_tls_sum(void);
void modify_tls_from_other_unit(void);

/* Opaque function to prevent optimization */
void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function that takes address of TLS variables */
void take_addresses(void) {
    int *ptr1 = &public_tls;
    int *ptr2 = &static_tls;
    int *ptr3 = &weak_tls;
    
    use_value((int)(long)ptr1);
    use_value((int)(long)ptr2);
    use_value((int)(long)ptr3);
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    public_tls = rand() % 100;
    weak_tls = rand() % 100;
    external_tls = rand() % 100;
    common_tls = rand() % 100;
    static_tls = rand() % 100;
    imported_tls = rand() % 100;
    
    /* Mark variables as used */
    TREE_USED(&public_tls);
    TREE_USED(&static_tls);
    TREE_USED(&weak_tls);
    
    /* Take addresses to ensure variables aren't optimized away */
    take_addresses();
    
    /* Call functions from other compilation units */
    init_tls_vars(seed + 1);
    modify_tls_from_other_unit();
    
    /* Compute checksum using all accessible TLS variables */
    int sum = compute_tls_sum();
    sum += public_tls + static_tls + weak_tls + 
           external_tls + common_tls + imported_tls;
    
    /* Use variables in arithmetic operations */
    public_tls = public_tls * 2 + 1;
    static_tls = static_tls / 2 + static_tls % 3;
    weak_tls ^= 0x55;
    
    /* Final aggregation */
    sum = (sum + public_tls + static_tls + weak_tls) % 1000;
    
    printf("TLS checksum: %d (seed: %d)\n", sum, seed);
    
    return sum % 256;
}

/* Dummy macro for compilation without GCC tree internals */
#ifndef TREE_USED
#define TREE_USED(x) (void)(x)
#endif

#else
int main(void) {
    printf("GCC __thread not supported on this compiler\n");
    return 0;
}
#endif
