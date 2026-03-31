#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __GNUC__
/* Forward declarations for TLS variables defined in other files */
extern __thread int public_tls;
extern __thread int weak_tls;
extern __thread int external_tls;
extern __thread int common_tls;
#endif

/* Function prototypes from other compilation units */
void init_tls_vars(int seed);
void modify_tls_vars(int factor);
int compute_tls_checksum(void);

/* Main TLS variable with public linkage */
#ifdef __GNUC__
__thread int main_public_tls = 0;
#endif

/* Static TLS variable - should have different linkage */
#ifdef __GNUC__
static __thread int main_static_tls = 0;
#endif

/* Weak TLS variable in main */
#ifdef __GNUC__
__thread __attribute__((weak)) int main_weak_tls = 0;
#endif

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use command line or time for non-constant initialization */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Initialize main's TLS variables with non-constant values */
#ifdef __GNUC__
    main_public_tls = rand() % 100;
    main_static_tls = rand() % 100 + 100;
    main_weak_tls = rand() % 100 + 200;
    
    /* Force TREE_USED by taking addresses and using variables */
    printf("Main TLS addresses: %p, %p, %p\n", 
           (void*)&main_public_tls, 
           (void*)&main_static_tls,
           (void*)&main_weak_tls);
    
    /* Use the variables in computation */
    int local_sum = main_public_tls + main_static_tls + main_weak_tls;
    printf("Main TLS sum: %d\n", local_sum);
#endif
    
    /* Initialize TLS variables from other compilation units */
    init_tls_vars(seed + 1);
    
    /* Modify them */
    modify_tls_vars(2);
    
    /* Compute final checksum */
    int checksum = compute_tls_checksum();
    
#ifdef __GNUC__
    /* Include main's TLS variables in final checksum */
    checksum += main_public_tls + main_static_tls + main_weak_tls;
#endif
    
    printf("Final TLS checksum: %d (seed: %d)\n", checksum % 1000, seed);
    
    return checksum % 256;
}
