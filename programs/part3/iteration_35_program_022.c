/* Test TLS variables with various attributes to trigger EMUTLS attribute copying */

/* Force EMUTLS transformation by using a target without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

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

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_VISIBILITY_SPECIFIED: will be set by the visibility attribute */
/* Already covered by tls_hidden and tls_default */

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: Windows-specific */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* For non-Windows, we'll simulate with attribute */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function using TLS variables to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as TREE_USED */
    tls_used = tls_used + 1;
    tls_public = tls_public * 2;
    tls_static = tls_static - 50;
    tls_common = 999;
    tls_weak = tls_weak / 3;
    tls_hidden = tls_hidden + 1000;
    tls_default = tls_default - 100;
    tls_external = tls_external + 1;
    tls_dllimport = 1234;
}

/* DECL_CONTEXT: TLS variable inside a function (block scope) */
void function_with_tls(void) {
    static __thread int tls_in_function = 600;
    tls_in_function = tls_in_function + 10;
}

/* Main function that uses all TLS variables */
int main(void) {
    use_tls_variables();
    function_with_tls();
    
    /* Ensure all variables are used */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_external;
    sum += tls_dllimport;
    
    return sum > 0 ? 0 : 1;
}
