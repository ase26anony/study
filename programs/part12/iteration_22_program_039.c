/* Define TLS variables with various attributes to be cloned during emulation */

/* Public TLS with default visibility and initializer */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) */
__thread int common_tls;

/* DLL import simulation (for attribute copying) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate similar concept with visibility */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* Function that uses static TLS with non-trivial context */
static int helper() {
    static __thread int local_func_tls = 100;
    local_func_tls++;
    return local_func_tls;
}

/* Function that returns address of TLS variable (forces address-taking) */
int* get_public_tls_addr() {
    /* Ensure TREE_USED is set */
    public_tls = public_tls + 1;
    return &public_tls;
}

/* Function using weak TLS */
int use_weak_tls() {
    if (&weak_tls_var != 0) {
        weak_tls_var = weak_tls_var + 10;
    }
    return weak_tls_var;
}

/* Function with static TLS in nested context */
void nested_context() {
    static int counter = 0;
    counter++;
    
    {
        /* Static TLS inside block scope */
        static __thread int block_tls = counter * 10;
        block_tls += 5;
    }
}

/* Initialize common TLS */
void init_common_tls(int value) {
    common_tls = value;
}

int get_common_tls() {
    return common_tls;
}
