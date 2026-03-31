/* Main test file for EMUTLS attribute copying coverage */

/* Force EMUTLS transformation by targeting ARM without hardware TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used_attr __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable */
__thread int tls_public_var = 100;

/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static_var = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common_var;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak_var __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED with hidden visibility */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED with default visibility */
__thread int tls_default_var __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
void function_with_tls(void) {
    __thread int tls_local_func = 600;
    tls_local_func++;  /* Ensure TREE_USED */
}

/* DECL_EXTERNAL: External TLS variable declaration */
extern __thread int tls_external_var;

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* TREE_USED: Reference each variable */
    tls_used_attr++;
    tls_public_var++;
    tls_static_var++;
    tls_common_var = 123;
    tls_weak_var++;
    tls_hidden_var++;
    tls_default_var++;
    tls_external_var++;  /* External reference */
    
    function_with_tls();
}

int main(void) {
    reference_all_tls();
    
    /* Additional references to ensure variables are marked used */
    volatile int sum = 0;
    sum += tls_used_attr;
    sum += tls_public_var;
    sum += tls_static_var;
    sum += tls_common_var;
    sum += tls_weak_var;
    sum += tls_hidden_var;
    sum += tls_default_var;
    sum += tls_external_var;
    
    return sum > 0 ? 0 : 1;
}
