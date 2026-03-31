/* Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol - tests DECL_WEAK */
__thread int weak_tls_var __attribute__((weak));

/* Static TLS inside function - tests DECL_CONTEXT */
static void inner_function(void) {
    static __thread int local_tls = 100;  /* DECL_CONTEXT should be inner_function */
    local_tls++;
}

/* TLS with non-constant initializer - forces runtime initialization */
extern int get_random(void);
__thread int dynamic_tls = get_random();

/* Common TLS - tests DECL_COMMON */
__thread int common_tls;  /* Tentative definition */

/* DLL import simulation - tests DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with visibility hidden then external reference */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses TLS variables */
int* get_public_tls_addr(void) {
    return &public_tls;
}

void use_inner_tls(void) {
    inner_function();
}

int get_dynamic_tls(void) {
    return dynamic_tls;
}
