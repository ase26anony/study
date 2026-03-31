/* C file with TLS definitions to exercise various DECL attributes */

/* For DECL_PRESERVE_P - used attribute */
__thread int tls_preserved __attribute__((used)) = 42;

/* For TREE_PUBLIC - non-static (public) TLS */
__thread int tls_public = 100;

/* For TREE_PUBLIC - static (non-public) TLS */
static __thread int tls_static = 200;

/* For DECL_COMMON - no initializer (common linkage) */
__thread int tls_common;

/* For DECL_WEAK - weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* For DECL_VISIBILITY - hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* For DECL_VISIBILITY - default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* Function-scoped TLS for DECL_CONTEXT */
void function_with_tls(void) {
    /* TLS inside function - different DECL_CONTEXT */
    static __thread int tls_in_function = 600;
    tls_in_function++;  /* Ensure TREE_USED */
}

/* External declaration that will be defined in another file */
extern __thread int tls_external;

/* Ensure all TLS variables are marked TREE_USED */
void use_all_tls(void) {
    tls_preserved++;
    tls_public++;
    tls_static++;
    tls_common = 700;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    
    /* Reference external TLS */
    tls_external = 800;
    
    function_with_tls();
}
