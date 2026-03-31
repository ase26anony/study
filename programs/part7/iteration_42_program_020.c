#ifdef __GNUC__
#define THREAD __thread
#else
#define THREAD /* nothing - for non-GCC compilers */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations from other files */
extern THREAD int public_tls;
extern THREAD int weak_tls;
extern THREAD int imported_tls;
extern THREAD int common_tls;

/* Static TLS in main file */
static THREAD int static_tls;

/* Function prototypes from other files */
void init_tls_values(int base);
void modify_tls_values(void);
int compute_tls_checksum(void);

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Initialize with non-constant values */
    int base_value = (argc > 1) ? atoi(argv[1]) : rand() % 100;
    
    /* Initialize static TLS in this file */
    static_tls = base_value + 100;
    
    /* Initialize and use TLS variables */
    init_tls_values(base_value);
    modify_tls_values();
    
    /* Compute checksum using all TLS variables */
    int checksum = compute_tls_checksum();
    
    /* Use addresses to prevent optimization */
    printf("TLS addresses:\n");
    printf("  static_tls: %p\n", (void*)&static_tls);
    
    /* Final checksum */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum % 256;
}
