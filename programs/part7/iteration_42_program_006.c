#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations from other files */
extern void use_tls_variables(int seed);
extern void modify_tls_from_other_unit(void);
extern int compute_tls_checksum(void);

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (file-local) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable - may be overridden by another definition */
__thread __attribute__((weak)) int weak_tls = 0;

/* External TLS variable - defined in another file */
extern __thread int external_tls;

/* Common TLS variable (when compiled with -fcommon) */
__thread int common_tls;

/* Function to initialize TLS variables with non-constant values */
void init_tls_variables(int argc, char **argv) {
    /* Use argc and argv to create non-constant initializers */
    public_tls = argc * 100;
    static_tls = (argv[0] != NULL) ? 1 : 0;
    
    /* Use rand() for non-constant initialization */
    weak_tls = rand() % 1000;
    
    /* Initialize common variable with non-constant value */
    common_tls = public_tls + weak_tls;
    
    /* Take addresses to ensure variables are marked as used */
    volatile int *ptr1 = &public_tls;
    volatile int *ptr2 = &static_tls;
    volatile int *ptr3 = &weak_tls;
    volatile int *ptr4 = &common_tls;
    
    /* Opaque use of addresses */
    printf("Addresses: %p, %p, %p, %p\n", 
           (void*)ptr1, (void*)ptr2, (void*)ptr3, (void*)ptr4);
}

/* Function that uses TLS variables in non-trivial ways */
int compute_checksum(void) {
    int sum = 0;
    
    /* Perform arithmetic operations */
    sum += public_tls * 2;
    sum -= static_tls;
    sum += weak_tls / 3;
    sum += common_tls % 100;
    
    /* Complex expression to prevent optimization */
    sum = (sum * 1103515245 + 12345) & 0x7fffffff;
    
    return sum;
}

int main(int argc, char **argv) {
    srand(time(NULL));
    
    /* Initialize TLS variables */
    init_tls_variables(argc, argv);
    
    /* Call function from another compilation unit */
    use_tls_variables(argc);
    
    /* Modify TLS from another unit */
    modify_tls_from_other_unit();
    
    /* Compute final checksum using all TLS variables */
    int checksum = compute_checksum();
    checksum += compute_tls_checksum();  /* From other file */
    
    /* Use external TLS variable if available */
    #ifdef EXTERNAL_TLS_DEFINED
    checksum += external_tls;
    #endif
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
#endif /* __GNUC__ */
