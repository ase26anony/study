/* TLS variables with various attributes to trigger emulated TLS attribute copying */

/* Public TLS with explicit visibility */
__thread int tls_public_default __attribute__((visibility("default"), used)) = 42;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 200;

/* Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* External declaration - will be defined in another file */
extern __thread int tls_external;

/* DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with a weak external reference */
extern __thread int tls_dllimport __attribute__((weak));
#endif

/* Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_in_function = 999;
    tls_in_function++;
    /* Force address taking without side effects */
    asm volatile("" : : "r"(&tls_in_function));
}

/* Public TLS used in multiple functions */
__thread unsigned long tls_counter __attribute__((used)) = 0;

/* Function that uses various TLS variables */
void update_tls_vars(void) {
    tls_public_default++;
    tls_hidden *= 2;
    
    if (tls_weak) {
        tls_weak--;
    }
    
    tls_common = tls_public_default + tls_hidden;
    tls_counter++;
    
    /* Use the external TLS if available */
    if (&tls_external) {
        tls_common += tls_external;
    }
    
    func_with_static_tls();
}

/* Another function with different TLS usage pattern */
unsigned long compute_tls_checksum(void) {
    unsigned long sum = 0;
    
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_counter;
    
    /* Force compiler to keep all TLS variables */
    asm volatile("" : : "r"(&tls_public_default), "r"(&tls_hidden), 
                 "r"(&tls_weak), "r"(&tls_common), "r"(&tls_counter));
    
    return sum;
}
