/* Test for EMUTLS attribute copying - Main file */

/* Force EMUTLS transformation by targeting ARM without hardware TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

/* TLS variable with used attribute (DECL_PRESERVE_P) */
__thread int tls_used __attribute__((used)) = 42;

/* Public TLS variable (TREE_PUBLIC) */
__thread int tls_public = 100;

/* Static (non-public) TLS variable */
static __thread int tls_static = 200;

/* Weak TLS variable (DECL_WEAK) */
__thread int tls_weak __attribute__((weak)) = 300;

/* TLS variable with hidden visibility (DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED) */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* TLS variable with default visibility */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* Common TLS variable without initializer (DECL_COMMON) */
__thread int tls_common;

/* External TLS variable declaration (DECL_EXTERNAL) */
extern __thread int tls_external;

/* Function-scoped TLS variable (tests DECL_CONTEXT) */
void use_function_tls(void) {
    __thread int tls_function = 600;
    tls_function++;  /* Ensure TREE_USED */
}

/* DLL import simulation - use appropriate attribute for platform */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with visibility or weak */
__thread int tls_dllimport __attribute__((weak));
#endif

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Reference each variable */
    int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_common;
    sum += tls_external;
    sum += tls_dllimport;
    
    /* Use sum to avoid optimization */
    tls_common = sum;
}

int main(void) {
    /* Ensure all TLS variables are marked as used */
    reference_all_tls();
    
    /* Use function-scoped TLS */
    use_function_tls();
    
    /* Simple operations on TLS variables */
    tls_public++;
    tls_weak--;
    tls_hidden = tls_default * 2;
    
    /* Call function from other compilation unit */
    use_external_tls();
    
    return 0;
}
