/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS variable with external linkage */
__thread int public_tls_var = 42;

/* Static TLS variable with internal linkage */
static __thread int static_tls_var = 100;

/* Weak TLS variable that can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* Common TLS variable (tentative definition) */
__thread int common_tls_var __attribute__((common));

/* TLS variables with different visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 500;

/* DLL import/export simulation (GCC style) */
#ifdef _WIN32
__thread int __declspec(dllexport) exported_tls_var = 600;
#else
__thread int __attribute__((visibility("default"))) exported_tls_var = 600;
#endif

/* TLS variable in a struct to test DECL_CONTEXT */
struct ThreadData {
    __thread int member_tls;
};

/* Force usage to set TREE_USED flag */
void use_tls_vars_defs(void) {
    /* Take addresses to ensure variables are marked as used */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &common_tls_var;
    int* ptr5 = &hidden_tls_var;
    int* ptr6 = &protected_tls_var;
    int* ptr7 = &internal_tls_var;
    int* ptr8 = &exported_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4),
                   "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8));
    
    /* Access struct member TLS */
    struct ThreadData td;
    td.member_tls = 123;
    
    /* Use C11 _Thread_local for variety */
    _Thread_local int c11_tls = 789;
    int* ptr9 = &c11_tls;
    asm volatile("" : : "r"(ptr9));
}
