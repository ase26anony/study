#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations for TLS variables defined in other files */
extern __thread int public_tls;
extern __thread int weak_tls;
extern __thread int imported_tls;
extern __thread int common_tls;

/* Static TLS variable - tests DECL_PRESERVE_P, DECL_CONTEXT */
static __thread int static_tls;

/* External TLS variable - tests DECL_EXTERNAL */
extern __thread int external_tls;

/* Define external_tls here to test DECL_EXTERNAL path */
__thread int external_tls = 0;

/* Function prototypes */
void init_tls_vars(int seed);
int compute_checksum(void);
void use_tls_from_other_file(void);

/* Opaque function to prevent optimization */
void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

void use_address(void* addr) {
    volatile void* sink = addr;
    (void)sink;
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    init_tls_vars(rand());
    
    /* Use all TLS variables to ensure TREE_USED is set */
    public_tls += argc;
    static_tls = rand() % 100;
    weak_tls = public_tls * 2;
    external_tls = static_tls + weak_tls;
    
    /* Take addresses to ensure variables aren't optimized away */
    use_address(&public_tls);
    use_address(&static_tls);
    use_address(&weak_tls);
    use_address(&external_tls);
    
    /* Call function from another compilation unit */
    use_tls_from_other_file();
    
    /* Compute and print checksum */
    int checksum = compute_checksum();
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    return checksum % 256;
}

void init_tls_vars(int seed) {
    /* Initialize with non-constant values */
    public_tls = seed % 1000;
    static_tls = (seed * 13) % 1000;
    weak_tls = (seed * 17) % 1000;
    external_tls = (seed * 19) % 1000;
    
    /* Use volatile to prevent constant propagation */
    volatile int vs = seed;
    common_tls = vs % 500;
    imported_tls = vs % 300;
}

int compute_checksum(void) {
    int sum = 0;
    
    /* Access all TLS variables */
    sum += public_tls;
    sum += static_tls;
    sum += weak_tls;
    sum += external_tls;
    
    /* Use opaque function call */
    use_value(sum);
    
    return sum & 0xFFFF;
}
#endif /* __GNUC__ */
