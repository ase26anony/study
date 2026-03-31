/* This should trigger emulated TLS code generation */
/* Test case for tree-emutls.cc attribute copying (lines 295-304) */

/* Force emulated TLS even if target supports native TLS */
#pragma GCC target("tls-model=emulated")

/* Default visibility for contrast with hidden */
__attribute__((visibility("default"), used)) 
__thread int tls_default = 1;

/* Static TLS with internal linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Weak TLS symbol - may be overridden */
__attribute__((weak)) 
__thread int tls_weak = 5;

/* Hidden visibility TLS */
__attribute__((visibility("hidden")))
__thread int tls_hidden;

/* DLL import simulation (Windows-like) */
#ifdef _WIN32
__attribute__((dllimport)) 
#endif
__thread int tls_dll;

/* Common TLS (uninitialized with external linkage) */
__thread int tls_common;

/* Definition of previously declared extern */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_static = tls_default * 2;
    tls_hidden = tls_extern + tls_weak;
    
    /* Take address to force symbol usage */
    volatile int *addr = &tls_hidden;
    (void)addr; /* Suppress unused warning */
}

/* Another helper with different TLS usage pattern */
int compute_tls_sum(void) {
    int sum = 0;
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    
    /* Conditional based on TLS value */
    if (tls_common > 0) {
        sum += tls_common;
    }
    
    return sum;
}

int main(void) {
    int result = 0;
    
    /* Initialize uninitialized TLS */
    tls_hidden = 4;
    tls_common = 6;
    
    /* Use all TLS variables */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    
    /* Call helper functions */
    modify_tls();
    result = compute_tls_sum();
    
    /* Take addresses to inhibit optimizations */
    volatile int *addr1 = &tls_default;
    volatile int *addr2 = &tls_weak;
    volatile int *addr3 = &tls_extern;
    (void)addr1; (void)addr2; (void)addr3;
    
    /* Prevent dead code elimination */
    if (result > 100) {
        return 1;
    }
    
    return 0;
}

/* Force generation of TLS control structures */
__attribute__((constructor))
void init_tls_test(void) {
    /* Access TLS to ensure initialization */
    tls_default = tls_default;
}
