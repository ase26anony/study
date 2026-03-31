/* Test TLS variables with various attributes to trigger EMUTLS attribute copying */

/* Force EMUTLS transformation by using non-TLS-supporting target flags */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

/* Public TLS variable with used attribute - covers DECL_PRESERVE_P */
__thread int tls_used __attribute__((used)) = 42;

/* Static (non-public) TLS variable - covers TREE_PUBLIC */
static __thread int tls_static = 100;

/* TLS variable without initializer (common linkage) - covers DECL_COMMON */
__thread int tls_common;

/* Weak TLS variable - covers DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 200;

/* TLS variable with hidden visibility - covers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* TLS variable with default visibility - covers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_default __attribute__((visibility("default"))) = 400;

/* Function-scope TLS variable - covers DECL_CONTEXT */
void function_with_tls(void) {
    __thread int tls_function_local = 500;
    tls_function_local++;  /* Ensure TREE_USED is set */
}

/* External TLS declaration - will be defined in another file */
extern __thread int tls_external;

/* DLL import simulation for Windows targets - covers DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* For non-Windows, use attribute to simulate */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Read and write to each variable */
    int val;
    
    val = tls_used;
    tls_used = val + 1;
    
    val = tls_static;
    tls_static = val + 1;
    
    val = tls_common;
    tls_common = val + 1;
    
    val = tls_weak;
    tls_weak = val + 1;
    
    val = tls_hidden;
    tls_hidden = val + 1;
    
    val = tls_default;
    tls_default = val + 1;
    
    val = tls_external;
    tls_external = val + 1;
    
    val = tls_dllimport;
    tls_dllimport = val + 1;
    
    function_with_tls();
}

/* Main function for C test */
int main_c(void) {
    reference_all_tls();
    return 0;
}
