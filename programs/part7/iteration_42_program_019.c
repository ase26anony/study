#ifdef __GNUC__
#define TLS __thread
#else
#define TLS /* empty if not GCC */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations from other files */
extern TLS int public_tls;
extern TLS int weak_tls;
extern TLS int imported_tls;
extern TLS int common_tls;

/* Function prototypes from other files */
void init_tls_vars(int seed);
void modify_tls_vars(void);
int compute_tls_checksum(void);

/* Local TLS variables with different attributes */
static TLS int static_tls = 0;  /* Static linkage */

/* Weak TLS variable defined here as fallback */
#ifdef __GNUC__
TLS __attribute__((weak)) int weak_tls = 0;
#endif

/* Common TLS variable (no initializer to potentially be common) */
TLS int common_tls;

/* Function to ensure TLS variables are used non-trivially */
static void use_tls_addresses(void) {
    /* Take addresses to prevent optimization */
    static volatile void* addrs[4];
    
    addrs[0] = (void*)&static_tls;
    addrs[1] = (void*)&weak_tls;
    addrs[2] = (void*)&common_tls;
    
    /* Opaque use of addresses */
    printf("TLS addresses: %p %p %p\n", 
           addrs[0], addrs[1], addrs[2]);
}

int main(int argc, char** argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize with non-constant values */
    static_tls = rand() % 100;
    weak_tls = rand() % 100;
    common_tls = rand() % 100;
    
    /* Use addresses to ensure TREE_USED is set */
    use_tls_addresses();
    
    /* Call functions from other compilation units */
    init_tls_vars(rand());
    modify_tls_vars();
    
    /* Compute and print checksum */
    int checksum = compute_tls_checksum();
    checksum += static_tls + weak_tls + common_tls;
    
    printf("Final checksum: %d\n", checksum % 1000);
    return checksum % 256;
}
