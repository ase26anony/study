#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable - can be overridden */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common symbol behavior (no initializer) */
__thread int common_tls;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__thread __attribute__((dllimport)) int imported_tls;
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls = 0;
#endif

/* Function to initialize and use TLS variables */
void init_tls_vars(int seed) {
    /* Initialize with non-constant values */
    public_tls = seed * 2;
    static_tls = seed + 1;
    weak_tls = seed * 3;
    common_tls = seed - 1;
    
    /* Use volatile to prevent optimization */
    volatile int* volatile_ptr = &imported_tls;
    *volatile_ptr = seed * 4;
    
    /* Take addresses to ensure TREE_USED is set */
    printf("Addresses in init_tls_vars:\n");
    printf("  public_tls: %p\n", (void*)&public_tls);
    printf("  static_tls: %p\n", (void*)&static_tls);
    printf("  weak_tls: %p\n", (void*)&weak_tls);
    printf("  common_tls: %p\n", (void*)&common_tls);
    printf("  imported_tls: %p\n", (void*)&imported_tls);
}

/* Function that modifies TLS variables */
int modify_tls_vars(int multiplier) {
    int sum = 0;
    
    public_tls *= multiplier;
    static_tls += multiplier;
    weak_tls -= multiplier;
    common_tls = common_tls * multiplier + 1;
    
    /* Complex enough to prevent optimization */
    sum = public_tls + static_tls + weak_tls + common_tls;
    
    /* Opaque function call with TLS addresses */
    printf("Modified TLS values:\n");
    printf("  public_tls: %d\n", public_tls);
    printf("  static_tls: %d\n", static_tls);
    
    return sum;
}

#endif /* __GNUC__ */
