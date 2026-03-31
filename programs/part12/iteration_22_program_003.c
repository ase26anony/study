/* Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol - tests DECL_WEAK */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS - tests DECL_COMMON */
__thread int common_tls;

/* DLL import simulation - tests DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with visibility hidden then accessed externally */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function returning address of TLS - forces address taking */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function using weak TLS */
int use_weak_tls(void) {
    weak_tls_var = 100;
    return weak_tls_var;
}

/* Function with static TLS in local scope - tests DECL_CONTEXT */
void func_with_local_tls(void) {
    static __thread int local_func_tls;  /* DECL_CONTEXT should be this function */
    local_func_tls++;
}

/* Force preservation flag - tests DECL_PRESERVE_P */
__thread int preserved_tls __attribute__((used)) = 99;
