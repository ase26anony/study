/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS by including this in compilation flags */
#pragma GCC visibility push(default)

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int c11_public_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local int c11_static_tls = 14;

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

/* TLS that will be preserved (used in inline asm) */
__thread int preserved_tls_var = 700;

/* Function using TLS variables to ensure they're marked as used */
void use_tls_vars(void) {
    /* Take addresses - ensures TREE_USED is set */
    int *ptr1 = &public_tls_var;
    int *ptr2 = &static_tls_var;
    int *ptr3 = &weak_tls_var;
    int *ptr4 = &hidden_tls_var;
    int *ptr5 = &exported_tls_var;
    
    /* Use in volatile context to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    asm volatile ("" : : "r"(&preserved_tls_var));
    
    /* Access all TLS variables */
    public_tls_var++;
    static_tls_var++;
    c11_public_tls++;
    c11_static_tls++;
    weak_tls_var++;
    common_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    
    /* Use struct member TLS */
    struct S s;
    s.member_tls = 100;
    S::static_member_tls++;
}

/* Thread-local with register storage class hint */
register __thread int reg_tls_var asm("") = 800;

#pragma GCC visibility pop
