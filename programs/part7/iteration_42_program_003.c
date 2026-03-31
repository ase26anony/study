#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Declare external TLS variables from other files */
extern __thread int external_tls;
extern __thread int common_tls;
extern __thread int weak_tls;

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable definition */
__thread __attribute__((weak)) int weak_tls = 0;

/* Function that uses TLS variables to prevent optimization */
int compute_checksum(int seed) {
    /* Use non-constant initialization */
    public_tls = seed * 2;
    static_tls = seed + 1;
    
    /* Take address to prevent optimization */
    int *ptr1 = &public_tls;
    int *ptr2 = &static_tls;
    
    /* Use external TLS variables */
    external_tls = seed * 3;
    common_tls = seed * 4;
    
    /* Complex enough to not be optimized away */
    return public_tls + static_tls + external_tls + common_tls + weak_tls;
}

/* Opaque function to prevent optimization */
void use_tls_pointers(void) {
    /* Take addresses and pass to printf */
    printf("Public TLS addr: %p\n", (void*)&public_tls);
    printf("Static TLS addr: %p\n", (void*)&static_tls);
    
    /* Volatile to prevent dead store elimination */
    volatile int *volatile_ptr = &public_tls;
    *volatile_ptr = *volatile_ptr + 1;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Initialize with non-constant values */
    int seed = argc > 1 ? atoi(argv[1]) : rand();
    
    /* First computation */
    int sum1 = compute_checksum(seed);
    
    /* Use the opaque function */
    use_tls_pointers();
    
    /* Modify and recompute */
    public_tls += 100;
    static_tls += 200;
    
    int sum2 = compute_checksum(seed + 1);
    
    /* Final result using all TLS variables */
    int final_result = public_tls + static_tls + external_tls + common_tls + weak_tls;
    
    printf("Checksum 1: %d\n", sum1);
    printf("Checksum 2: %d\n", sum2);
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}

#else
int main(void) {
    printf("GCC TLS not supported\n");
    return 0;
}
#endif
