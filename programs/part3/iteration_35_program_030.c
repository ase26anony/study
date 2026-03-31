/* Test for EMUTLS attribute copying - C language file */

/* Force EMUTLS transformation by targeting non-TLS architecture */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated */

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

/* DECL_DLLIMPORT_P: Windows-specific attribute */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_EXTERNAL: External declaration (defined in another file) */
extern __thread int tls_external;

/* Function-scoped TLS variable (different DECL_CONTEXT) */
void use_function_tls(void) {
    /* DECL_CONTEXT: function scope */
    static __thread int tls_function_scope = 600;
    tls_function_scope++;
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* TREE_USED: Read/write operations */
    int val;
    
    val = tls_used;
    tls_used = val + 1;
    
    val = tls_public;
    tls_public = val * 2;
    
    val = tls_static;
    tls_static = val - 1;
    
    tls_common = 999;
    
    val = tls_weak;
    tls_weak = val / 2;
    
    val = tls_hidden;
    tls_hidden = val + 100;
    
    val = tls_default;
    tls_default = val - 50;
    
    tls_dllimport = 777;
    
    val = tls_external;
    tls_external = val + 111;
    
    use_function_tls();
}

/* Main function for C test */
int main_c(void) {
    reference_all_tls();
    return 0;
}
