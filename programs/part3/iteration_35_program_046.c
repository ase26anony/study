/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by using non-TLS-supporting target flags */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

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

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS variable inside a function (local scope) */
void function_with_tls(void) {
    __thread int tls_local = 600;
    tls_local++;  /* Ensure TREE_USED */
}

/* DECL_DLLIMPORT_P: Windows-specific dllimport */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_EXTERNAL: External declaration (defined in another file) */
extern __thread int tls_external;

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_tls_variables(void) {
    /* Read/write to mark as used */
    int val;
    
    val = tls_used;
    tls_used = val + 1;
    
    val = tls_public;
    tls_public = val + 2;
    
    val = tls_static;
    tls_static = val + 3;
    
    val = tls_common;
    tls_common = val + 4;
    
    val = tls_weak;
    tls_weak = val + 5;
    
    val = tls_hidden;
    tls_hidden = val + 6;
    
    val = tls_default;
    tls_default = val + 7;
    
    val = tls_dllimport;
    tls_dllimport = val + 8;
    
    val = tls_external;
    tls_external = val + 9;
    
    function_with_tls();
}
