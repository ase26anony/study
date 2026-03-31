#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer to potentially be common) */
__thread int common_tls;

/* DLL import attribute simulation (for Windows/MinGW) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, simulate with external declaration */
extern __thread int imported_tls;
#endif

/* Function prototypes */
int compute_checksum(void);
void use_tls_variables(int seed);
void modify_from_other_unit(void);

/* External TLS variable declaration (will be defined elsewhere) */
extern __thread int external_tls;

/* Function that uses all TLS variables to prevent optimization */
void use_tls_variables(int seed) {
    /* Initialize with non-constant values */
    public_tls = seed * 2;
    static_tls = seed + 1;
    weak_tls = seed - 5;
    common_tls = seed * 3;
    
    /* Use volatile to prevent constant propagation */
    volatile int* volatile_ptr;
    volatile_ptr = &public_tls;
    *volatile_ptr += 1;
    
    /* Take addresses to ensure variables are marked used */
    printf("Addresses: public=%p, static=%p, weak=%p, common=%p\n",
           (void*)&public_tls, (void*)&static_tls, 
           (void*)&weak_tls, (void*)&common_tls);
    
    /* Arithmetic operations */
    public_tls += static_tls * 2;
    weak_tls = public_tls % 100;
    common_tls ^= seed;
}

/* Compute checksum from all accessible TLS variables */
int compute_checksum(void) {
    int sum = 0;
    
    sum += public_tls;
    sum += static_tls;
    sum += weak_tls;
    sum += common_tls;
    
    /* Access external variable if available */
    extern __thread int external_tls;
    sum += external_tls;
    
    return sum & 0xFF; /* Return modulo 256 for consistent output */
}

int main(int argc, char** argv) {
    int seed;
    
    /* Use command line argument or time for non-constant seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL) & 0xFF;
    }
    
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    use_tls_variables(seed);
    
    /* Call function from another compilation unit */
    modify_from_other_unit();
    
    /* Re-initialize with different values */
    use_tls_variables(rand() % 100);
    
    /* Compute and print checksum */
    int checksum = compute_checksum();
    printf("TLS checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

#else
int main(void) {
    printf("GCC thread-local storage not supported\n");
    return 0;
}
#endif
