/* tls_decl.c - Declaration of TLS variable with various attributes */

/* Force emulated TLS */
#pragma GCC tls_model emulated

#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

/* External declaration with multiple attributes */
extern DLL_IMPORT __thread int tls_var 
    __attribute__((weak, visibility("hidden"), common));

/* Another TLS variable for local static context */
void use_local_tls(void) {
    /* Local static TLS variable with function context */
    static __thread int local_tls_var __attribute__((used)) = 42;
    
    /* Use it to ensure TREE_USED is set */
    local_tls_var++;
}

/* Function to reference the external TLS variable */
int* get_tls_var_ptr(void) {
    return &tls_var;
}
