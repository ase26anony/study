#ifdef __GNUC__
#define THREAD __thread
#else
#define THREAD /* empty if not GCC */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for TLS variables defined in other files */
extern THREAD int external_tls;
extern THREAD int weak_tls;
extern THREAD int imported_tls;

/* Public TLS variable with initialization */
THREAD int public_tls = 0;

/* Static (private) TLS variable */
static THREAD int static_tls = 0;

/* Function prototypes */
void init_tls_vars(int seed);
int use_tls_vars(void);
void process_from_other_file(void);

/* Opaque function to prevent optimization */
static void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function that uses all TLS variables */
int compute_tls_checksum(void) {
    int sum = 0;
    
    /* Access each TLS variable in a non-trivial way */
    sum += public_tls * 2;
    sum += static_tls * 3;
    
    /* External variables */
    sum += external_tls * 5;
    
    /* Weak symbol - may be zero if not defined elsewhere */
    if (&weak_tls != NULL) {
        sum += weak_tls * 7;
    }
    
    /* DLL imported variable simulation */
    sum += imported_tls * 11;
    
    /* Take addresses to ensure they're marked as used */
    use_value((int)(long)&public_tls);
    use_value((int)(long)&static_tls);
    use_value((int)(long)&external_tls);
    
    return sum % 1000;
}

/* Initialize TLS variables with non-constant values */
void init_tls_vars(int seed) {
    /* Use seed to make initialization non-constant */
    srand(seed);
    
    public_tls = rand() % 100;
    static_tls = seed % 50;
    
    /* Initialize external_tls through pointer if available */
    extern THREAD int external_tls;
    external_tls = (seed * 3) % 75;
}

int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    
    init_tls_vars(seed);
    process_from_other_file();
    
    int checksum = compute_tls_checksum();
    printf("TLS checksum: %d\n", checksum);
    
    return 0;
}
