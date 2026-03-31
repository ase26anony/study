#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common TLS variable (no initializer) */
__thread int common_tls;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls = 0;
#endif

/* External declarations for variables defined elsewhere */
extern __thread int external_tls;
extern __thread int another_common_tls;

/* Function prototypes from other files */
void use_tls_variables(int seed);
void modify_external_tls(int value);

/* Opaque function to prevent optimization */
static void use_value(int value) {
    volatile int sink = value;
    (void)sink;
}

/* Take address to ensure TREE_USED is set */
static void take_addresses(void) {
    static volatile void* addrs[6];
    addrs[0] = &public_tls;
    addrs[1] = &static_tls;
    addrs[2] = &weak_tls;
    addrs[3] = &common_tls;
    addrs[4] = &imported_tls;
    addrs[5] = &external_tls;
    (void)addrs;
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize with non-constant values */
    public_tls = rand() % 100;
    static_tls = rand() % 100 + 100;
    weak_tls = rand() % 100 + 200;
    common_tls = rand() % 100 + 300;
    imported_tls = rand() % 100 + 400;
    
    /* Use the variables */
    take_addresses();
    
    /* Call functions from other compilation units */
    use_tls_variables(seed);
    modify_external_tls(seed * 2);
    
    /* Compute checksum using all TLS variables */
    int checksum = public_tls + static_tls + weak_tls + 
                   common_tls + imported_tls + external_tls;
    
    /* Use printf with %p to force address taking */
    printf("TLS addresses: %p %p %p\n", 
           (void*)&public_tls, 
           (void*)&weak_tls,
           (void*)&external_tls);
    
    printf("Checksum: %d\n", checksum % 1000);
    
    return checksum % 256;
}
#else
int main(void) {
    printf("GCC thread-local storage not supported\n");
    return 0;
}
#endif
