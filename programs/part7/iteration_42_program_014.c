#ifdef __GNUC__
/* Main file with TLS variable definitions and usage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable - may be overridden by another definition */
__thread __attribute__((weak)) int weak_tls = 0;

/* External TLS variable declaration (defined in tls_lib.c) */
extern __thread int external_tls;

/* Common TLS variable (no initializer, becomes common symbol with -fcommon) */
__thread int common_tls;

/* DLL import attribute simulation (for Windows/MinGW targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, simulate with external */
extern __thread int simulated_import_tls;
#endif

/* Function prototypes from other files */
void init_tls_variables(int seed);
int compute_tls_checksum(void);
void use_common_tls(int value);

/* Opaque function to prevent optimization */
static void use_value(int value) {
    volatile int sink = value;
    (void)sink;
}

/* Function that takes addresses of TLS variables */
static void take_addresses(void) {
    /* Taking addresses ensures TREE_USED is set */
    int *p1 = &public_tls;
    int *p2 = &static_tls;
    int *p3 = &weak_tls;
    
    use_value((int)(long)p1);
    use_value((int)(long)p2);
    use_value((int)(long)p3);
    
    /* Print addresses to ensure they're used */
    printf("Addresses: %p, %p, %p\n", 
           (void*)p1, (void*)p2, (void*)p3);
}

int main(int argc, char **argv) {
    int seed = 0;
    
    /* Use argv to get non-constant initialization */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    public_tls = rand() % 100;
    static_tls = public_tls + argc;
    weak_tls = seed % 50;
    
    /* Initialize common TLS variable */
    common_tls = (seed * 3) % 100;
    
    /* Take addresses to ensure TREE_USED */
    take_addresses();
    
    /* Call functions from other compilation units */
    init_tls_variables(seed + 1);
    
    /* Use the common TLS variable */
    use_common_tls(common_tls + 10);
    
    /* Compute final checksum */
    int checksum = compute_tls_checksum();
    
    /* Use all variables in computation */
    checksum += public_tls + static_tls + weak_tls + common_tls;
    
    /* Access external TLS variable */
    checksum += external_tls;
    
    printf("TLS checksum: %d (seed: %d)\n", checksum % 1000, seed);
    
    return checksum % 256;
}
#else
int main(void) {
    printf("GCC thread-local storage not supported\n");
    return 0;
}
#endif
