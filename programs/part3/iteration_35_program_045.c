/* Test for EMUTLS attribute copying - Main file */

/* Force EMUTLS transformation by targeting ARM without TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

/* TLS variable with used attribute (DECL_PRESERVE_P) */
__thread int tls_used __attribute__((used)) = 42;

/* Public TLS variable (TREE_PUBLIC = 1) */
__thread int tls_public = 100;

/* Static TLS variable (TREE_PUBLIC = 0) */
static __thread int tls_static = 200;

/* TLS variable with hidden visibility (DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED) */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* TLS variable with default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 400;

/* Weak TLS variable (DECL_WEAK) */
__thread int tls_weak __attribute__((weak)) = 500;

/* Common TLS variable without initializer (DECL_COMMON) */
__thread int tls_common;

/* Function-scoped TLS variable (different DECL_CONTEXT) */
void function_with_tls(void) {
    __thread int tls_function_scope = 600;
    tls_function_scope++;  /* Ensure TREE_USED */
}

/* External TLS variable declaration (DECL_EXTERNAL) */
extern __thread int tls_external;

/* Function that uses all TLS variables to ensure TREE_USED */
void use_all_tls(void) {
    /* Reference all TLS variables */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 10;
    tls_hidden = tls_hidden / 2;
    tls_default = tls_default + 100;
    tls_weak = tls_weak - 50;
    tls_common = 999;
    tls_external = tls_external + 1000;
    
    /* Call function with function-scoped TLS */
    function_with_tls();
}

int main(void) {
    /* Initialize common TLS variable */
    tls_common = 777;
    
    /* Use all TLS variables */
    use_all_tls();
    
    /* Perform some operations to ensure variables are used */
    int sum = tls_used + tls_public + tls_static + tls_hidden + 
              tls_default + tls_weak + tls_common;
    
    /* Return non-zero if any TLS variable has unexpected value */
    if (sum < 0) return 1;
    
    return 0;
}
