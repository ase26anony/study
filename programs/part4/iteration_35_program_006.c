/* Define TLS variables with various attributes to trigger emutls_decl logic */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int c11_public_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local int c11_static_tls = 13;

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
__thread int imported_tls_var __attribute__((weak));
#endif

/* TLS in different contexts */
struct S {
    __thread int member_tls;
    static __thread int static_member_tls;
};

/* Definition of static member TLS */
__thread int S::static_member_tls = 600;

/* Force usage to set TREE_USED */
void force_usage_defs(void) {
    /* Take addresses */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &hidden_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile ("" : : "r"(&common_tls_var));
    asm volatile ("" : : "r"(&protected_tls_var));
    
    /* Access to ensure they're marked used */
    public_tls_var += 1;
    static_tls_var *= 2;
    hidden_tls_var -= 3;
    
    /* Use struct member TLS */
    struct S s;
    s.member_tls = 700;
    S::static_member_tls = 800;
}
