#ifdef __GNUC__
#define TLS __thread
#else
#define TLS /* empty if not GCC */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for TLS variables defined in other files */
#ifdef __GNUC__
extern TLS int public_tls;
extern TLS int weak_tls;
extern TLS int imported_tls;
extern TLS int common_tls;
#endif

/* Static TLS variable in main file */
#ifdef __GNUC__
static TLS int static_tls;
#endif

/* External TLS variable - declared here, defined in test_lib1.c */
#ifdef __GNUC__
extern TLS int external_tls;
#endif

/* Functions from other files */
void init_tls_vars(int seed);
int compute_tls_checksum(void);
void use_common_tls(int value);

int main(int argc, char *argv[]) {
    int i;
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
#ifdef __GNUC__
    /* Initialize static TLS variable with non-constant value */
    static_tls = rand() % 1000;
    
    /* Use all TLS variables to ensure they're marked TREE_USED */
    printf("Static TLS: %d\n", static_tls);
    
    /* Take address to prevent optimization */
    int *static_ptr = &static_tls;
    *static_ptr += argc;
#endif
    
    /* Initialize TLS variables in other compilation units */
    init_tls_vars(seed);
    
    /* Use functions that access TLS variables from other files */
    int checksum = compute_tls_checksum();
    
    /* Test common TLS behavior */
    use_common_tls(checksum);
    
    /* Final computation using all accessible TLS variables */
    int final_result = 0;
    
#ifdef __GNUC__
    final_result += static_tls;
    
    /* Access external TLS variables - these will trigger attribute copying */
    if (&public_tls) final_result += 1;
    if (&weak_tls) final_result += 2;
    if (&imported_tls) final_result += 3;
    if (&common_tls) final_result += 4;
    if (&external_tls) final_result += 5;
#endif
    
    final_result += checksum;
    
    printf("Final result: %d (seed: %d)\n", final_result % 1000, seed);
    return final_result % 100;
}
