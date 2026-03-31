#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for TLS variables from other files */
extern __thread int public_tls;
extern __thread int weak_tls;
extern __thread int external_tls;
extern __thread int common_tls;
#ifdef _WIN32
extern __thread int imported_tls;
#endif

/* Function prototypes from other files */
void init_tls_vars(int seed);
int compute_tls_sum(void);
void use_tls_from_other_unit(void);

/* Local static TLS variable */
static __thread int static_tls;

/* Function to prevent optimization */
static void use_var(int *ptr) {
    volatile int sink = *ptr;
    (void)sink;
}

int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize local TLS variables with non-constant values */
    static_tls = rand() % 100;
    
    /* Initialize TLS variables in other compilation unit */
    init_tls_vars(seed + 1);
    
    /* Use all TLS variables to ensure they're marked TREE_USED */
    use_var(&static_tls);
    use_var(&public_tls);
    use_var(&weak_tls);
    use_var(&external_tls);
    use_var(&common_tls);
    
    /* Call function from another compilation unit */
    use_tls_from_other_unit();
    
    /* Compute checksum using all accessible TLS variables */
    int sum = compute_tls_sum();
    sum += static_tls;
    
    printf("TLS checksum: %d (seed: %d)\n", sum % 1000, seed);
    
    /* Take addresses to ensure variables aren't optimized away */
    printf("Addresses: %p %p %p %p %p\n", 
           (void*)&static_tls,
           (void*)&public_tls,
           (void*)&weak_tls,
           (void*)&external_tls,
           (void*)&common_tls);
    
    return 0;
}
#endif
