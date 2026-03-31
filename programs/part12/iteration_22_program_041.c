/* Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int public_tls __attribute__((visibility("default"))) = 42;

/* Weak TLS symbol - tests DECL_WEAK */
__thread int weak_tls_var __attribute__((weak));

/* Common TLS (tentative definition) - tests DECL_COMMON */
__thread int common_tls;

/* Static TLS inside function - tests DECL_CONTEXT */
static int* get_local_tls_addr(void) {
    static __thread int local_func_tls = 100;
    return &local_func_tls;
}

/* DLL import simulation - tests DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with visibility hidden then accessed externally */
__thread int imported_tls __attribute__((visibility("hidden")));
#endif

/* External reference - tests DECL_EXTERNAL */
extern __thread int external_tls_ref;

/* Function that uses TLS addresses */
int* get_public_tls_addr(void) {
    /* Force TREE_USED to be set */
    public_tls += 1;
    weak_tls_var = public_tls * 2;
    common_tls = weak_tls_var + 1;
    
    /* Take address to force emutls runtime setup */
    int* addrs[3];
    addrs[0] = &public_tls;
    addrs[1] = &weak_tls_var;
    addrs[2] = &common_tls;
    
    return addrs[public_tls % 3];
}

/* Function with static TLS in nested scope */
void complex_context(void) {
    {
        static __thread int nested_tls = 999;
        nested_tls++;
    }
    
    /* Another with preservation flag simulation */
    static __thread int preserved_tls = 123;
    /* Simulate DECL_PRESERVE_P by taking address in complex way */
    volatile int* volatile_ptr = &preserved_tls;
    (void)volatile_ptr;
}
