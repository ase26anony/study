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

/* Common TLS variable (no initializer, relies on -fcommon) */
__thread int common_tls;

/* DLL import simulation for Windows/MinGW targets */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls = 0;
#endif

/* Function prototypes from other files */
void init_tls_variables(int seed);
int compute_tls_checksum(void);
void modify_tls_from_lib(int value);

/* Opaque function to prevent optimization */
static void use_value(int value) {
    volatile int sink = value;
    (void)sink;
}

/* Function that takes addresses of TLS variables */
static void take_tls_addresses(void) {
    int *ptr1 = &public_tls;
    int *ptr2 = &static_tls;
    int *ptr3 = &weak_tls;
    int *ptr4 = &external_tls;
    int *ptr5 = &common_tls;
    int *ptr6 = &imported_tls;
    
    use_value((int)(long)ptr1);
    use_value((int)(long)ptr2);
    use_value((int)(long)ptr3);
    use_value((int)(long)ptr4);
    use_value((int)(long)ptr5);
    use_value((int)(long)ptr6);
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    public_tls = rand() % 100;
    static_tls = argc + rand() % 50;
    weak_tls = seed % 77;
    common_tls = public_tls + static_tls;
    imported_tls = weak_tls * 2;
    
    /* Call function from another compilation unit */
    init_tls_variables(seed + 1);
    
    /* Modify TLS variables through another file */
    modify_tls_from_lib(seed % 10);
    
    /* Take addresses to ensure TREE_USED is set */
    take_tls_addresses();
    
    /* Compute and print checksum */
    int checksum = compute_tls_checksum();
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    /* Use all TLS variables in arithmetic */
    int result = public_tls + static_tls + weak_tls + external_tls + 
                 common_tls + imported_tls;
    printf("TLS sum: %d\n", result % 1000);
    
    return 0;
}
#endif /* __GNUC__ */
