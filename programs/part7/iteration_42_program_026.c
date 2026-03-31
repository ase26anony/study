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

/* Function prototypes from other files */
void init_tls_vars(int seed);
void modify_tls_vars(void);
int compute_tls_checksum(void);

/* Main TLS variable with public linkage */
#ifdef __GNUC__
__thread int main_public_tls;
#endif

/* Static TLS variable - should have different linkage */
#ifdef __GNUC__
static __thread int main_static_tls;
#endif

/* Weak TLS variable in main */
#ifdef __GNUC__
__thread __attribute__((weak)) int main_weak_tls;
#endif

/* Function that uses TLS variables in non-trivial ways */
void use_tls_vars(int base) {
#ifdef __GNUC__
    /* Initialize with non-constant values */
    main_public_tls = base + 1;
    main_static_tls = base + 2;
    main_weak_tls = base + 3;
    
    /* Take addresses to prevent optimization */
    int *ptr1 = &main_public_tls;
    int *ptr2 = &main_static_tls;
    int *ptr3 = &main_weak_tls;
    
    /* Perform arithmetic operations */
    main_public_tls = (*ptr1) * 2;
    main_static_tls = (*ptr2) + (*ptr3);
    main_weak_tls = (*ptr1) ^ (*ptr2);
    
    /* Opaque use through printf */
    printf("TLS addresses: %p, %p, %p\n", 
           (void*)ptr1, (void*)ptr2, (void*)ptr3);
#endif
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Use command line argument for non-constant initialization */
    int seed = (argc > 1) ? atoi(argv[1]) : rand();
    
#ifdef __GNUC__
    /* Initialize main's TLS variables */
    use_tls_vars(seed);
    
    /* Initialize TLS variables from other files */
    init_tls_vars(seed + 100);
    
    /* Modify them */
    modify_tls_vars();
    
    /* Compute and print checksum */
    int checksum = compute_tls_checksum();
    
    /* Add main's TLS variables to checksum */
    checksum += main_public_tls + main_static_tls + main_weak_tls;
    
    printf("Final TLS checksum: %d\n", checksum % 1000);
#else
    printf("TLS not supported on this compiler\n");
#endif
    
    return 0;
}
