#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* External TLS declaration (defined in test_lib1.c) */
extern __thread int external_tls;

/* Function prototypes from other files */
void init_tls_vars(int seed);
int compute_tls_sum(void);
void use_common_tls(void);

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    public_tls = rand() % 100;
    weak_tls = rand() % 100;
    
    /* Call functions from other compilation units */
    init_tls_vars(seed + 1);
    
    /* Use TLS variables in non-trivial ways */
    printf("Public TLS address: %p, value: %d\n", 
           (void*)&public_tls, public_tls);
    printf("Weak TLS address: %p, value: %d\n", 
           (void*)&weak_tls, weak_tls);
    
    /* Take addresses to ensure TREE_USED is set */
    int *ptr1 = &public_tls;
    int *ptr2 = &weak_tls;
    
    /* Perform arithmetic operations */
    public_tls = public_tls * 2 + 1;
    weak_tls = weak_tls / 2 + 5;
    
    /* Access external TLS variable */
    printf("External TLS value: %d\n", external_tls);
    external_tls += public_tls;
    
    /* Use common TLS variables */
    use_common_tls();
    
    /* Compute final checksum */
    int sum = compute_tls_sum();
    sum += public_tls + weak_tls + external_tls;
    
    printf("TLS checksum: %d\n", sum % 1000);
    return sum % 256;
}

#else
int main() {
    printf("GCC TLS not supported\n");
    return 0;
}
#endif
