/* Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol - tests DECL_WEAK */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS - tests DECL_COMMON */
__thread int common_tls;

/* DLL import style attribute - tests DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate similar concept with visibility */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-global context - tests DECL_CONTEXT */
static void helper_function(void) {
    /* Static TLS inside function - has function as DECL_CONTEXT */
    static __thread int local_func_tls = 100;
    local_func_tls++;
}

/* Force preservation flag - tests DECL_PRESERVE_P */
__thread int preserved_tls __attribute__((used)) = 999;

/* External declaration that will be defined elsewhere */
extern __thread int external_tls;

/* Get address of public TLS - forces address taking */
int* get_public_tls_addr(void) {
    helper_function();  /* Ensure static TLS is referenced */
    return &public_tls;
}

/* Modify weak TLS */
void init_weak_tls(void) {
    if (&weak_tls_var != NULL) {  /* Reference weak symbol */
        weak_tls_var = 77;
    }
}

/* Use common TLS */
void use_common_tls(void) {
    common_tls = common_tls + 1;
}

/* Reference imported TLS */
int* get_imported_addr(void) {
    return &imported_tls;
}
