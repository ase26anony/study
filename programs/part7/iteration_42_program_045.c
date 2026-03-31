#ifdef __GNUC__
#define THREAD_LOCAL __thread
#else
#define THREAD_LOCAL 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External declarations from other files */
extern THREAD_LOCAL int public_tls;
extern THREAD_LOCAL int weak_tls;
extern THREAD_LOCAL int imported_tls;
extern THREAD_LOCAL int common_tls;

/* Static TLS in main file */
static THREAD_LOCAL int static_tls;

/* Function declarations from other files */
void init_tls_values(int seed);
int compute_tls_sum(void);
void use_common_tls(int val);

/* Opaque function to prevent optimization */
static void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize static TLS with non-constant value */
    static_tls = seed * 2;
    use_value(static_tls);
    
    /* Initialize values in other compilation unit */
    init_tls_values(seed);
    
    /* Use external TLS variables */
    public_tls += 10;
    weak_tls -= 5;
    
    /* Take addresses to ensure TREE_USED */
    int *ptr1 = &public_tls;
    int *ptr2 = &weak_tls;
    use_value((int)(long)ptr1);
    use_value((int)(long)ptr2);
    
    /* Use common TLS variable */
    use_common_tls(seed * 3);
    
    /* Compute checksum */
    int sum = compute_tls_sum();
    sum += static_tls;
    
    printf("TLS checksum: %d\n", sum);
    
    return sum % 256;
}
