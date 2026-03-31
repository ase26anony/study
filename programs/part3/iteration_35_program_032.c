/* Test for EMUTLS attribute copying - Main file */

/* Force EMUTLS transformation by using a target without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden */

/* TLS variable with used attribute - triggers DECL_PRESERVE_P */
__thread int tls_used __attribute__((used)) = 42;

/* Public TLS variable (non-static) - triggers TREE_PUBLIC */
__thread int tls_public = 100;

/* Static TLS variable (non-public) - for contrast */
static __thread int tls_static = 200;

/* TLS variable without initializer (common linkage) - triggers DECL_COMMON */
__thread int tls_common;

/* Weak TLS variable - triggers DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 300;

/* TLS variable with hidden visibility - triggers DECL_VISIBILITY */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* TLS variable with default visibility - triggers DECL_VISIBILITY_SPECIFIED */
__thread int tls_default_vis __attribute__((visibility("default"))) = 500;

/* External TLS variable declaration - will be defined in another file */
extern __thread int tls_external;

/* Function using TLS variables to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 50;
    tls_common = 999;
    tls_weak = tls_weak / 3;
    tls_hidden = tls_hidden + 1000;
    tls_default_vis = tls_default_vis | 0xFF;
    
    /* Use external TLS variable */
    tls_external = tls_external + 1234;
}

/* Function-scoped TLS variable - tests DECL_CONTEXT in function scope */
void function_with_tls(void) {
    static __thread int tls_in_function = 600;
    tls_in_function = tls_in_function + 1;
}

/* Main function that uses all TLS variables */
int main(void) {
    use_tls_variables();
    function_with_tls();
    
    /* Additional uses to ensure variables are marked used */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default_vis;
    sum += tls_external;
    
    return sum > 0 ? 0 : 1;
}
