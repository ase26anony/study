/* TLS variables with various attributes to trigger emulated TLS attribute copying */

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__thread int tls_dllimport __attribute__((dllimport));
#else
/* Simulate with external declaration for non-Windows */
extern __thread int tls_dllimport;
#endif

/* Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 300;
    tls_func_static++;
    asm volatile("" : : "r"(&tls_func_static));
}

/* External TLS declarations (defined in another file) */
extern __thread int tls_external;
extern __thread int tls_external_weak __attribute__((weak));

/* Function that uses all TLS variables */
int use_tls_vars1(void) {
    int sum = 0;
    
    /* Access public TLS */
    sum += tls_public_default;
    tls_public_default++;
    
    /* Access hidden TLS */
    sum += tls_hidden;
    tls_hidden += 2;
    
    /* Access weak TLS */
    sum += tls_weak;
    tls_weak += 3;
    
    /* Access common TLS */
    sum += tls_common;
    tls_common = sum;
    
    /* Access DLL import TLS */
    sum += tls_dllimport;
    
    /* Access external TLS */
    sum += tls_external;
    
    /* Access weak external TLS */
    sum += tls_external_weak;
    
    /* Call function with static TLS */
    func_with_static_tls();
    
    return sum;
}

/* Force address-taking without optimization removal */
void force_address_taking1(void) {
    asm volatile("" : : 
        "r"(&tls_public_default),
        "r"(&tls_hidden),
        "r"(&tls_weak),
        "r"(&tls_common)
    );
}
