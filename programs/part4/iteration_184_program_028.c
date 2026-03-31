/* This should trigger emulated TLS code generation */
/* Test case for tree-emutls.cc attribute copying logic */

/* Force emulated TLS handling */
#pragma GCC target("tls-model=emulated")

/* Global TLS variables with different attributes */
/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static (internal linkage) TLS */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 5;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 6;

/* TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_visible = 7;

/* Used attribute ensures TREE_USED is set */
__attribute__((used)) __thread int tls_used = 8;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use dllimport attribute anyway - it should still set DECL_DLLIMPORT_P */
__attribute__((dllimport)) __thread int tls_dllimport = 9;
#endif

/* Common TLS (uninitialized, external linkage) */
__thread int tls_common;

/* Definition of previously declared extern TLS */
__thread int tls_extern = 3;

/* Weak external TLS */
extern __thread int tls_weak_extern __attribute__((weak));
__thread int tls_weak_extern = 10;

/* Function that modifies TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_static = tls_default * 2;
    tls_hidden = tls_visible + 1;
    tls_used = tls_weak + tls_common;
    
    /* Take address to inhibit optimizations */
    volatile int *addr = &tls_static;
    (void)addr;
}

/* Another function using TLS with different pattern */
int compute_tls_sum(void) {
    int sum = 0;
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible;
    sum += tls_used;
    sum += tls_common;
    
    /* Force use of dllimport variable if defined */
    if (tls_dllimport) {
        sum += 1;
    }
    
    return sum;
}

/* Main function demonstrating TLS usage */
int main(void) {
    int result = 0;
    
    /* Initialize uninitialized TLS */
    tls_common = 4;
    
    /* Use all TLS variables to prevent elimination */
    tls_default += 1;
    tls_static += 2;
    tls_extern += 3;
    
    /* Call function that modifies TLS */
    modify_tls();
    
    /* Compute using TLS variables */
    result = compute_tls_sum();
    
    /* Take addresses of TLS variables - important for emulation */
    volatile int *addr1 = &tls_default;
    volatile int *addr2 = &tls_hidden;
    volatile int *addr3 = &tls_visible;
    (void)addr1; (void)addr2; (void)addr3;
    
    /* Use weak TLS */
    if (tls_weak_extern) {
        result += tls_weak_extern;
    }
    
    /* Create side effect with result */
    if (result > 100) {
        tls_default = result;
    }
    
    /* Prevent optimization of entire program */
    return result & 0xFF;
}

/* Additional TLS in different scope for more coverage */
static void nested_function(void) {
    static __thread int nested_tls = 42;
    nested_tls += tls_default;
    volatile int *addr = &nested_tls;
    (void)addr;
}
