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
/* Simulate with external declaration */
extern __thread int imported_tls;
#endif

/* Function that uses TLS and returns address */
int* get_public_tls_addr(void) {
    return &public_tls;
}

/* Function that modifies weak TLS */
void init_weak_tls(void) {
    if (&weak_tls_var != NULL) {  /* Weak symbol may be NULL */
        weak_tls_var = 100;
    }
}

/* Function with static TLS inside - tests DECL_CONTEXT */
static void helper_function(void) {
    /* Static TLS inside function - has function as context */
    static __thread int local_func_tls;
    local_func_tls++;
}

void call_helper(void) {
    helper_function();
}
