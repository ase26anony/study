#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __GNUC__
/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* External TLS declaration (defined in test_lib1.c) */
extern __thread int external_tls;

/* Common TLS variable (no initializer) */
__thread int common_tls;

/* DLL import simulation (for Windows/MinGW targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls = 0;
#endif

/* Function prototypes from other files */
void init_tls_vars(int seed);
int compute_tls_sum(void);
void modify_tls_from_lib2(void);
#endif

/* Opaque function to prevent optimization */
static void use_value(int value) {
    volatile int sink = value;
    (void)sink;
}

/* Function that takes address of TLS variables */
static void take_addresses(void) {
#ifdef __GNUC__
    use_value((int)&public_tls);
    use_value((int)&static_tls);
    use_value((int)&weak_tls);
    use_value((int)&external_tls);
    use_value((int)&common_tls);
    use_value((int)&imported_tls);
#endif
}

int main(int argc, char *argv[]) {
#ifdef __GNUC__
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    public_tls = rand() % 100;
    static_tls = rand() % 100 + 100;
    
    /* Weak variable might not be defined, check before using */
    if (&weak_tls) {
        weak_tls = rand() % 100 + 200;
    }
    
    /* Common variable gets a value */
    common_tls = rand() % 100 + 300;
    
    /* Imported variable */
    imported_tls = rand() % 100 + 400;
    
    /* Mark variables as used */
    TREE_USED: /* This label ensures the variables aren't optimized away */
    take_addresses();
    
    /* Call functions from other compilation units */
    init_tls_vars(seed + 1);
    modify_tls_from_lib2();
    
    /* Compute and print checksum */
    int sum = compute_tls_sum();
    printf("TLS checksum: %d\n", sum);
    
    /* Additional usage to ensure TREE_USED is set */
    printf("Addresses: public=%p, static=%p\n", 
           (void*)&public_tls, (void*)&static_tls);
    
    return sum % 256;
#else
    printf("TLS not supported on this compiler\n");
    return 0;
#endif
}
