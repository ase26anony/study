#ifdef __GNUC__
/* Main file with TLS variable definitions and usage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer to encourage common symbol behavior) */
__thread int common_tls;

/* Function prototypes from other files */
extern void use_external_tls(int val);
extern int get_external_tls(void);
extern void modify_common_tls(int val);

/* Opaque function to prevent optimization */
static void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function that uses TLS variables in non-trivial ways */
static void process_tls_values(int seed) {
    /* Initialize with non-constant values */
    public_tls = seed * 2;
    static_tls = seed + 1;
    
    /* Use weak variable if defined */
    if (&weak_tls != NULL) {
        weak_tls = seed * 3;
    }
    
    /* Use common variable */
    common_tls = seed * 4;
    
    /* Take addresses to ensure variables are marked used */
    int *ptrs[] = {
        &public_tls,
        &static_tls,
        &weak_tls,
        &common_tls
    };
    
    /* Perform arithmetic operations */
    int sum = public_tls + static_tls + common_tls;
    if (&weak_tls != NULL) {
        sum += weak_tls;
    }
    
    /* Pass to opaque function */
    use_value(sum);
    
    /* Complex usage pattern */
    for (int i = 0; i < 3; i++) {
        public_tls += i;
        static_tls -= i;
        if (&weak_tls != NULL) {
            weak_tls *= (i + 1);
        }
        common_tls ^= i;
    }
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize with runtime-dependent values */
    public_tls = rand() % 100;
    static_tls = rand() % 100 + 100;
    
    /* Use external TLS variable */
    use_external_tls(rand() % 50);
    int ext_val = get_external_tls();
    
    /* Modify common TLS variable */
    modify_common_tls(rand() % 75);
    
    /* Process all TLS values */
    process_tls_values(seed);
    
    /* Compute checksum from all accessible TLS variables */
    int checksum = public_tls + static_tls + common_tls + ext_val;
    
    if (&weak_tls != NULL) {
        checksum += weak_tls;
    }
    
    /* Print addresses to ensure variables are used */
    printf("TLS addresses: public=%p, static=%p, weak=%p, common=%p\n",
           (void*)&public_tls, (void*)&static_tls, 
           (void*)&weak_tls, (void*)&common_tls);
    
    printf("Checksum: %d\n", checksum % 1000);
    
    return 0;
}
#endif
