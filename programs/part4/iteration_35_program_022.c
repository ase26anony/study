/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int c11_public_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 10;
static _Thread_local int c11_static_tls = 20;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;
__thread int weak_undefined_tls __attribute__((weak));

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* TLS with visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 200;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 300;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 400;

/* DLL import/export simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 500;
__declspec(dllimport) __thread int imported_tls_var;
#else
__thread int __attribute__((visibility("default"))) exported_tls_var = 500;
__thread int __attribute__((visibility("default"))) imported_tls_var = 600;
#endif

/* TLS in different contexts */
struct S {
    __thread int member_tls;
    __thread long member_tls2;
};

/* Force usage to set TREE_USED */
void use_tls_vars(void) {
    /* Take addresses to ensure variables are marked as used */
    int *ptr1 = &public_tls_var;
    int *ptr2 = &static_tls_var;
    int *ptr3 = &weak_tls_var;
    int *ptr4 = &common_tls_var;
    int *ptr5 = &hidden_tls_var;
    int *ptr6 = &protected_tls_var;
    int *ptr7 = &internal_tls_var;
    int *ptr8 = &exported_tls_var;
    int *ptr9 = &imported_tls_var;
    
    /* Use in volatile context */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    asm volatile ("" : : "r"(ptr4), "r"(ptr5), "r"(ptr6));
    
    /* Access variables */
    public_tls_var++;
    static_tls_var += 2;
    weak_tls_var *= 3;
    common_tls_var = public_tls_var + static_tls_var;
    
    /* Use struct with TLS members */
    struct S s;
    s.member_tls = 1000;
    s.member_tls2 = 2000;
    
    /* Use C11 TLS variables */
    c11_public_tls++;
    c11_static_tls--;
}
