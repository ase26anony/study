/* Test for EMUTLS attribute copying - Main file */

/* Force EMUTLS transformation by using a target without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden */

/* TLS variable with used attribute - triggers DECL_PRESERVE_P */
__thread int tls_used __attribute__((used)) = 42;

/* Public TLS variable (non-static) - triggers TREE_PUBLIC */
__thread int tls_public = 100;

/* Static TLS variable (non-public) - for contrast */
static __thread int tls_static = 200;

/* TLS variable with weak attribute - triggers DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 300;

/* TLS variable with hidden visibility - triggers DECL_VISIBILITY */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* TLS variable with default visibility - triggers DECL_VISIBILITY_SPECIFIED */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* TLS variable without initializer (common linkage) - triggers DECL_COMMON */
__thread int tls_common;

/* External TLS variable declaration - triggers DECL_EXTERNAL */
extern __thread int tls_external;

/* Function-scoped TLS variable - tests DECL_CONTEXT for local scope */
void use_function_tls(void) {
    __thread int tls_function = 600;
    tls_function += 10;  /* Ensure TREE_USED */
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Read and write to each variable */
    tls_used += 1;
    tls_public += 2;
    tls_static += 3;
    tls_weak += 4;
    tls_hidden += 5;
    tls_default += 6;
    tls_common += 7;
    tls_external += 8;
    
    /* Call function with function-scoped TLS */
    use_function_tls();
}

/* Main function that uses all TLS variables */
int main(void) {
    reference_all_tls();
    
    /* Additional usage patterns */
    tls_used = tls_public + tls_static;
    tls_hidden = tls_default * 2;
    tls_common = tls_weak / 2;
    
    /* Ensure external TLS is used */
    if (tls_external > 0) {
        tls_external = 999;
    }
    
    return 0;
}
