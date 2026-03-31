/* Test file for EMUTLS attribute copying - C language */

/* Force EMUTLS by using a target without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable with initializer */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED: default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
void use_local_tls(void) {
    __thread int tls_local = 600;
    tls_local++;  /* Ensure TREE_USED */
}

/* External declaration (will be defined in another file) */
extern __thread int tls_external;

/* Function to ensure all TLS variables are marked TREE_USED */
void use_all_tls_vars(void) {
    /* Reference each TLS variable to ensure TREE_USED is set */
    tls_used++;
    tls_public++;
    tls_static++;
    tls_common = 700;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    
    /* Use external TLS variable */
    tls_external++;
    
    /* Call function with local TLS */
    use_local_tls();
}

/* Main function for C test */
int main_c(void) {
    use_all_tls_vars();
    return 0;
}
