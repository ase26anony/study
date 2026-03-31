/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS usage */
#pragma GCC optimize("O0")

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int c11_public_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local int c11_static_tls = 21;

/* Weak TLS symbols */
__thread int weak_tls_var __attribute__((weak)) = 100;
__thread int weak_undefined_tls __attribute__((weak));

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* Visibility attributes */
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
    static __thread int static_member_tls;
};

__thread int S::static_member_tls = 700;

/* Used to set DECL_PRESERVE_P */
__thread int preserved_tls_var __attribute__((used)) = 800;

/* Function to force TLS usage */
void use_tls_defs(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int* ptr;
    
    ptr = &public_tls_var;
    ptr = &static_tls_var;
    ptr = &weak_tls_var;
    ptr = &common_tls_var;
    ptr = &hidden_tls_var;
    ptr = &protected_tls_var;
    ptr = &internal_tls_var;
    ptr = &exported_tls_var;
    ptr = &imported_tls_var;
    ptr = &preserved_tls_var;
    
    /* Access to ensure TREE_USED is set */
    public_tls_var++;
    static_tls_var++;
    c11_public_tls++;
    c11_static_tls++;
    
    /* Use in asm to prevent optimization */
    asm volatile("" : : "r"(&public_tls_var), "r"(&static_tls_var));
    
    /* Use struct member TLS */
    struct S s;
    s.member_tls = 900;
    S::static_member_tls = 1000;
}
