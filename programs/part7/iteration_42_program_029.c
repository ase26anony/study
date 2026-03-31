#ifdef __GNUC__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Declare external TLS variables that will be defined in other files */
extern __thread int external_tls;
extern __thread int common_tls;
extern __thread __attribute__((weak)) int weak_tls;

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* DLL import simulation (for Windows/MinGW targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, we'll just use a regular TLS variable */
__thread int imported_tls = 0;
#endif

/* Function that uses all TLS variables to prevent optimization */
int compute_checksum(int seed) {
    int sum = 0;
    
    /* Initialize with non-constant values */
    public_tls = seed * 2;
    static_tls = seed + 1;
    
    /* Use external variables if available */
    if (&external_tls != NULL) {
        sum += external_tls;
    }
    
    /* Use weak variable if linked */
    if (&weak_tls != NULL) {
        sum += weak_tls;
    }
    
    /* Use imported variable */
    sum += imported_tls;
    
    /* Use common variable */
    sum += common_tls;
    
    /* Use public and static variables */
    sum += public_tls;
    sum += static_tls;
    
    /* Take addresses to ensure variables are marked as used */
    volatile void *addr1 = &public_tls;
    volatile void *addr2 = &static_tls;
    (void)addr1;
    (void)addr2;
    
    return sum % 1000;
}

/* Function that modifies TLS variables */
void modify_tls_values(int multiplier) {
    public_tls *= multiplier;
    static_tls += multiplier;
    
    /* Access external variables */
    if (&external_tls != NULL) {
        external_tls = (external_tls * multiplier) % 100;
    }
    
    if (&weak_tls != NULL) {
        weak_tls = (weak_tls + multiplier) % 50;
    }
    
    common_tls = (common_tls * 2) % 30;
    imported_tls = multiplier;
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    /* Initialize with non-constant values */
    public_tls = rand() % 100;
    static_tls = rand() % 100;
    imported_tls = rand() % 100;
    
    printf("Initial checksum: %d\n", compute_checksum(seed));
    
    /* Modify values */
    modify_tls_values(3);
    
    printf("Modified checksum: %d\n", compute_checksum(seed + 1));
    
    /* Print addresses to ensure variables are used */
    printf("Addresses: public=%p, static=%p\n", 
           (void*)&public_tls, (void*)&static_tls);
    
    return 0;
}
#endif /* __GNUC__ */
