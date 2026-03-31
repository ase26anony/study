/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by targeting ARM without hardware TLS */
#ifdef __ARM_ARCH
#undef __ARM_ARCH
#endif
#define __ARM_ARCH 5

/* TLS variable with used attribute (DECL_PRESERVE_P) */
__thread int tls_used __attribute__((used)) = 42;

/* Public TLS variable (TREE_PUBLIC) */
__thread int tls_public = 100;

/* Static (non-public) TLS variable */
static __thread int tls_static = 200;

/* TLS variable without initializer (DECL_COMMON) */
__thread int tls_common;

/* Weak TLS variable (DECL_WEAK) */
__thread int tls_weak __attribute__((weak)) = 300;

/* TLS variable with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* TLS variable with default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* Function-scoped TLS variable (different DECL_CONTEXT) */
void function_with_tls(void) {
    __thread int tls_function_local = 600;
    tls_function_local++;  /* Ensure TREE_USED */
}

/* External TLS declaration (will be defined in another file) */
extern __thread int tls_external;

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Read and write to each variable */
    int sum = 0;
    
    sum += tls_used;
    tls_used = sum;
    
    sum += tls_public;
    tls_public = sum;
    
    sum += tls_static;
    tls_static = sum;
    
    sum += tls_common;
    tls_common = sum;
    
    sum += tls_weak;
    tls_weak = sum;
    
    sum += tls_hidden;
    tls_hidden = sum;
    
    sum += tls_default;
    tls_default = sum;
    
    sum += tls_external;
    /* Can't write to external here - will be done in main */
    
    function_with_tls();
}

/* Main function for C test */
int main_c(void) {
    reference_all_tls();
    
    /* Additional references to ensure usage */
    tls_external = 999;
    int val = tls_external;
    
    return 0;
}
