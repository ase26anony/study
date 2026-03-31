#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for TLS variables from other files */
extern __thread int public_tls;
extern __thread int weak_tls;
extern __thread int external_tls;
extern __thread int common_tls;

/* Function prototypes from other files */
void init_tls_vars(int seed);
int compute_tls_sum(void);
void use_dllimport_var(void);

/* Main file TLS variables with various attributes */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, simulate with visibility */
__thread __attribute__((visibility("default"))) int imported_tls;
#endif

static __thread int static_tls;

/* Function to ensure TLS variables are used non-trivially */
static int get_seed_value(int argc, char **argv) {
    if (argc > 1) return atoi(argv[1]);
    return time(NULL);
}

/* Opaque function to prevent optimization */
static void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

int main(int argc, char **argv) {
    int seed = get_seed_value(argc, argv);
    srand(seed);
    
    /* Initialize TLS variables with non-constant values */
    static_tls = rand() % 100;
    imported_tls = rand() % 100 + 100;
    
    /* Take addresses to ensure TREE_USED is set */
    int *static_ptr = &static_tls;
    int *imported_ptr = &imported_tls;
    use_value((int)(long)static_ptr);
    use_value((int)(long)imported_ptr);
    
    /* Initialize variables from other files */
    init_tls_vars(seed);
    
    /* Use DLL import variable if on Windows */
#ifdef _WIN32
    use_dllimport_var();
#endif
    
    /* Compute checksum using all TLS variables */
    int sum = compute_tls_sum();
    sum += static_tls + imported_tls;
    
    /* Print result to ensure side effects */
    printf("TLS checksum: %d (seed: %d)\n", sum % 1000, seed);
    
    return 0;
}
#endif /* __GNUC__ */
