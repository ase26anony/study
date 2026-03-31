/* Test for EMUTLS attribute copying - C file with various TLS attributes */

/* Force EMUTLS by using a target without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_preserved __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;  /* No initializer */

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED with default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS variable inside a function (block scope) */
void function_with_tls(void) {
    /* TLS variable with function scope */
    static __thread int tls_function_scope = 600;
    
    /* Mark as used by referencing it */
    tls_function_scope++;
}

/* DECL_DLLIMPORT_P: Simulate Windows DLL import */
/* Note: This typically requires Windows target, but we'll include it for completeness */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* For non-Windows, use a regular TLS variable */
__thread int tls_dllimport = 700;
#endif

/* External TLS declaration (will be defined in another file) */
extern __thread int tls_external;

/* Function to use all TLS variables to ensure TREE_USED is set */
void use_all_tls_variables(void) {
    /* Reference each TLS variable to mark them as TREE_USED */
    tls_preserved += 1;
    tls_public += 2;
    tls_static += 3;
    tls_common = 4;
    tls_weak += 5;
    tls_hidden += 6;
    tls_default += 7;
    tls_dllimport += 8;
    tls_external += 9;
    
    /* Call function with TLS variable */
    function_with_tls();
}

/* Main function for C test */
int main_c(void) {
    use_all_tls_variables();
    
    /* Additional usage to ensure variables are marked used */
    int sum = tls_preserved + tls_public + tls_static + tls_common +
              tls_weak + tls_hidden + tls_default + tls_dllimport;
    
    return sum > 0 ? 0 : 1;
}
