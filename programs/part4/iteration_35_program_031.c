/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int c11_public_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 10;
static _Thread_local int c11_static_tls = 20;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;
_Thread_local int c11_weak_tls __attribute__((weak)) = 200;

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));
_Thread_local int c11_common_tls __attribute__((common));

/* TLS with visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 50;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 60;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 70;

/* DLL import/export simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 300;
__declspec(dllimport) __thread int imported_tls_var;
#else
__thread int __attribute__((visibility("default"))) exported_tls_var = 300;
__thread int __attribute__((visibility("default"))) imported_tls_var = 400;
#endif

/* TLS in different contexts */
struct S {
    __thread int member_tls;
    _Thread_local int c11_member_tls;
};

/* Function to force usage of TLS variables */
void use_tls_vars(void) {
    /* Take addresses to ensure TREE_USED is set */
    int *ptr1 = &public_tls_var;
    int *ptr2 = &static_tls_var;
    int *ptr3 = &weak_tls_var;
    int *ptr4 = &common_tls_var;
    int *ptr5 = &hidden_tls_var;
    int *ptr6 = &protected_tls_var;
    int *ptr7 = &exported_tls_var;
    
    /* Use in volatile context to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    asm volatile("" : : "r"(ptr4), "r"(ptr5), "r"(ptr6), "r"(ptr7));
    
    /* Access variables */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var = 1;
    hidden_tls_var++;
    protected_tls_var++;
    exported_tls_var++;
    
    /* Use C11 TLS variables */
    c11_public_tls++;
    c11_static_tls++;
    c11_weak_tls++;
    c11_common_tls = 2;
}
