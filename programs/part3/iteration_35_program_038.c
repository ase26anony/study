/* Test for EMUTLS attribute copying - C file with various TLS attributes */

/* Force EMUTLS transformation by using non-TLS-supporting target flags:
   -march=armv5te -ftls-model=emulated */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED with default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
void function_with_tls(void) {
    __thread int tls_local = 600;
    tls_local++;  /* Ensure TREE_USED */
}

/* External TLS declaration (will be defined in another file) */
extern __thread int tls_external;

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_tls_variables(void) {
    /* Reference each variable to mark them as used */
    tls_used++;
    tls_public++;
    tls_static++;
    tls_common = 700;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    
    /* Reference external TLS */
    tls_external++;
    
    /* Call function with local TLS */
    function_with_tls();
}

/* Main function for C test */
int main_c(void) {
    reference_tls_variables();
    
    /* Additional references to ensure coverage */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    
    return sum > 0 ? 0 : 1;
}
