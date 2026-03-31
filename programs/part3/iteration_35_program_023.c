/* Test for EMUTLS attribute copying - Main file */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden */

/* Force EMUTLS transformation by using ARMv5 which lacks hardware TLS support */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used_attr __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED: explicit visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;
__thread int tls_default_vis __attribute__((visibility("default"))) = 500;

/* DECL_EXTERNAL: will be defined in another file */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: Windows-specific attribute */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function to ensure TREE_USED is set for all variables */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as TREE_USED */
    tls_used_attr += 1;
    tls_public += 2;
    tls_static += 3;
    tls_common = 600;
    tls_weak += 4;
    tls_hidden += 5;
    tls_default_vis += 6;
    tls_external += 7;
    tls_dllimport += 8;
    
    /* Use them in expressions to prevent optimization */
    volatile int sum = tls_used_attr + tls_public + tls_static + tls_common +
                      tls_weak + tls_hidden + tls_default_vis + tls_external +
                      tls_dllimport;
    (void)sum;
}

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
void function_with_local_tls(void) {
    static __thread int tls_in_function = 700;
    tls_in_function += 9;
}

int main(void) {
    use_tls_variables();
    function_with_local_tls();
    
    /* Additional uses to ensure coverage */
    tls_common = 123;
    tls_external = 456;
    
    return 0;
}
