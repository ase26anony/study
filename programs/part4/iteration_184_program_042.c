/* This should trigger emulated TLS code generation */
/* Test case for TLS attribute copying in emulated TLS */

/* Force emulated TLS handling */
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif

/* Declare TLS variables with various attributes */

/* Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS with internal linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 5;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 6;

/* TLS with default visibility and used attribute */
__attribute__((visibility("default"), used)) __thread int tls_visible_used = 7;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can still test with dllimport-like attribute */
__attribute__((weak)) __thread int tls_dllimport = 8;
#endif

/* Common TLS (uninitialized, external linkage) */
__thread int tls_common;

/* Definition of previously declared extern TLS */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS if available */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    /* Hidden TLS access */
    tls_hidden = tls_default + tls_static;
    
    /* Ensure used attribute is honored */
    volatile int *ptr = &tls_visible_used;
    *ptr = 42;
}

/* Another helper to take addresses */
void take_tls_addresses(void) {
    /* Take addresses to prevent optimization */
    volatile int *addr1 = &tls_default;
    volatile int *addr2 = &tls_static;
    volatile int *addr3 = &tls_extern;
    volatile int *addr4 = &tls_hidden;
    volatile int *addr5 = &tls_visible_used;
    
    /* Use addresses to create side effects */
    (void)addr1;
    (void)addr2;
    (void)addr3;
    (void)addr4;
    (void)addr5;
}

int main(void) {
    int sum = 0;
    
    /* Initialize common TLS */
    tls_common = 9;
    
    /* Use all TLS variables */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    
    /* Access weak symbol */
    if (&tls_weak) {
        sum += tls_weak;
    }
    
    sum += tls_hidden;
    sum += tls_visible_used;
    
#ifdef _WIN32
    /* DLL imported TLS */
    if (&tls_dllimport) {
        sum += tls_dllimport;
    }
#else
    sum += tls_dllimport;
#endif
    
    sum += tls_common;
    
    /* Modify TLS in helper */
    modify_tls();
    
    /* Take addresses */
    take_tls_addresses();
    
    /* Compute final result using TLS */
    int result = tls_default + tls_static + tls_extern + tls_common;
    
    /* Prevent dead code elimination */
    volatile int output = result + sum;
    
    return output > 0 ? 0 : 1;
}

/* Additional TLS with different storage duration simulation */
__attribute__((section(".tdata"))) __thread int tls_section = 99;

/* Force reference to section TLS */
void __attribute__((constructor)) init_tls_section(void) {
    volatile int *p = &tls_section;
    *p = 100;
}
