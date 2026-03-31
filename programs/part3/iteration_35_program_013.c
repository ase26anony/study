/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by using non-TLS-supporting target flags */
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

/* DECL_CONTEXT: TLS variable inside a function */
void function_with_tls(void) {
    static __thread int tls_in_function = 600;
    tls_in_function++;  /* Ensure TREE_USED */
}

/* DECL_DLLIMPORT_P: Windows-specific - we'll use a macro to make it portable */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* For non-Windows, we'll use a different attribute to test the code path */
__thread int tls_dllimport __attribute__((visibility("protected"))) = 700;
#endif

/* External TLS variable declaration - defined in another file */
extern __thread int tls_external;

/* Ensure all variables are TREE_USED by referencing them */
int use_all_tls_variables(void) {
    int sum = 0;
    
    /* Reference each TLS variable to mark them as used */
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_dllimport;
    sum += tls_external;
    
    /* Modify some to ensure they're really used */
    tls_public++;
    tls_common = sum;
    tls_weak = tls_public * 2;
    
    function_with_tls();
    
    return sum;
}

/* Main function that uses all TLS variables */
int main(void) {
    int result = use_all_tls_variables();
    
    /* Additional uses to ensure EMUTLS sees the references */
    tls_hidden = result;
    tls_default = tls_hidden + 1;
    
    return 0;
}
