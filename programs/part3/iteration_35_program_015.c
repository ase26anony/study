/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by using non-TLS-supporting target flags */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_preserved __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS variable with TREE_USED */
__thread int tls_public = 100;
/* TREE_PUBLIC: static (non-public) TLS variable */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS variable without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED: explicit visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_DLLIMPORT_P: Windows-specific attribute */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_imported __attribute__((dllimport));
#endif

/* Function-scoped TLS variable (different DECL_CONTEXT) */
void use_function_tls(void) {
    __thread int tls_function_scope = 600;
    tls_function_scope = tls_public + 1;  /* Ensure TREE_USED */
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_tls_vars(void) {
    /* Read and write to each variable */
    tls_public = tls_preserved + 1;
    tls_static = tls_public * 2;
    tls_common = tls_static - 50;
    tls_weak = tls_common / 2;
    tls_hidden = tls_weak + 100;
    tls_default = tls_hidden * 2;
    
    /* Use imported variable */
    extern __thread int tls_imported;
    tls_imported = tls_default;
    
    /* Call function with function-scoped TLS */
    use_function_tls();
}

/* External declaration for variable defined in another file */
extern __thread int tls_external;

void use_external_tls(void) {
    tls_external = 999;
    tls_public = tls_external;
}
