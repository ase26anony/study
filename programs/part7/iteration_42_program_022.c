#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __GNUC__
/* Forward declarations for TLS variables defined in other files */
extern __thread int public_tls;
extern __thread __attribute__((weak)) int weak_tls;
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

/* Static TLS variable (private linkage) */
#ifdef __GNUC__
static __thread int main_static_tls = 0;
#endif

/* Weak TLS variable in main */
#ifdef __GNUC__
__thread __attribute__((weak)) int main_weak_tls = 0;
#endif

/* Function that uses TLS variables in non-trivial ways */
void process_tls_values(int base) {
#ifdef __GNUC__
    /* Force TREE_USED to be set by taking addresses and performing operations */
    int *ptr1 = &main_public_tls;
    int *ptr2 = &main_static_tls;
    
    main_public_tls = base + rand() % 100;
    main_static_tls = base * 2 + (int)(ptr1 - ptr2);
    
    /* Use weak variable if available */
    if (&main_weak_tls) {
        main_weak_tls = base % 50;
    }
    
    /* Opaque use through printf */
    printf("TLS addresses: %p, %p\n", (void*)ptr1, (void*)ptr2);
#endif
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int seed = (argc > 1) ? atoi(argv[1]) : rand();
    
#ifdef __GNUC__
    /* Initialize with non-constant values */
    main_public_tls = seed;
    main_static_tls = seed * 2;
    
    /* Use external TLS variables */
    external_tls = seed % 100;
    common_tls = seed / 2;
    
    /* Initialize variables in other compilation unit */
    init_tls_vars(seed);
    
    /* Process TLS values */
    process_tls_values(seed);
    
    /* Modify TLS variables across compilation units */
    modify_tls_vars(seed % 10 + 1);
    
    /* Compute final checksum */
    int checksum = compute_tls_checksum();
    
    /* Add main's TLS variables to checksum */
    checksum += main_public_tls + main_static_tls;
    if (&main_weak_tls) {
        checksum += main_weak_tls;
    }
    
    printf("Final TLS checksum: %d\n", checksum % 1000);
    return checksum % 256;
#else
    printf("TLS not supported on this compiler\n");
    return 0;
#endif
}
