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
__thread int main_public_tls;
static __thread int main_static_tls;
__thread __attribute__((weak)) int main_weak_tls;
#endif

/* Function that uses TLS variables in non-trivial ways */
void process_main_tls(int base) {
#ifdef __GNUC__
    /* Initialize with non-constant values */
    main_public_tls = base + 1;
    main_static_tls = base * 2;
    
    /* Use weak TLS variable if available */
    if (&main_weak_tls) {
        main_weak_tls = base % 100;
    }
    
    /* Take addresses to prevent optimization */
    volatile int *ptr1 = &main_public_tls;
    volatile int *ptr2 = &main_static_tls;
    
    /* Perform arithmetic operations */
    main_public_tls += (*ptr1) * 3;
    main_static_tls -= (*ptr2) / 2;
    
    /* Opaque use through printf */
    printf("Main TLS pointers: %p, %p\n", 
           (void*)ptr1, (void*)ptr2);
#endif
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
#ifdef __GNUC__
    /* Initialize with non-constant values */
    int init_val = rand() % 1000;
    
    /* Process TLS variables in main */
    process_main_tls(init_val);
    
    /* Call functions from other compilation units */
    init_tls_vars(init_val + 100);
    modify_tls_vars(init_val % 50);
    
    /* Compute and print checksum */
    int checksum = compute_tls_checksum();
    
    /* Additional use of TLS variables */
    checksum += main_public_tls;
    checksum += main_static_tls;
    
    printf("Final TLS checksum: %d (seed: %d)\n", checksum, seed);
    return checksum % 256;
#else
    printf("TLS not supported on this compiler\n");
    return 0;
#endif
}
