/* First translation unit with various TLS attributes */

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 100;

/* Common linkage (tentative definition) */
__thread int tls_common;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* External declaration (defined in File2.c) */
extern __thread int tls_external;

/* DLL import simulation for Windows targets */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Static TLS inside function context */
static void use_static_tls(void) {
    static __thread int tls_static_func = 300;
    tls_static_func++;
    /* Force address taking without optimization removal */
    asm volatile("" : : "r"(&tls_static_func));
}

/* Function that uses all TLS variables */
int use_tls_file1(void) {
    int sum = 0;
    
    /* Access all TLS variables */
    sum += tls_public_default;
    tls_public_default++;
    
    sum += tls_weak;
    tls_weak += 2;
    
    sum += tls_common;
    tls_common = sum;  /* Store sum in common TLS */
    
    sum += tls_hidden;
    tls_hidden--;
    
    sum += tls_external;  /* External from File2 */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(&tls_dllimport));
    
    use_static_tls();
    
    return sum;
}

/* Another function with different access pattern */
void modify_tls_file1(void) {
    tls_public_default *= 2;
    tls_weak /= 2;
    tls_hidden += tls_common;
    
    /* Complex enough to prevent dead code elimination */
    if (tls_public_default > 1000) {
        tls_common = 0;
    }
}
