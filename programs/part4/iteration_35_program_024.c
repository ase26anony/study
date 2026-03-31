/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int c11_public_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local int c11_static_tls = 21;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;
_Thread_local int c11_weak_tls __attribute__((weak)) = 200;

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));
_Thread_local int c11_common_tls __attribute__((common));

/* TLS with visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 1;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 2;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 3;

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

/* Force usage to set TREE_USED */
void use_tls_vars(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    volatile int* ptr4 = &common_tls_var;
    volatile int* ptr5 = &hidden_tls_var;
    volatile int* ptr6 = &exported_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), 
                   "r"(ptr4), "r"(ptr5), "r"(ptr6));
    
    /* Access variables */
    public_tls_var++;
    static_tls_var--;
    weak_tls_var *= 2;
    common_tls_var = 999;
    hidden_tls_var = 111;
    exported_tls_var = 222;
    
    /* TLS in struct context */
    static struct S s;
    s.member_tls = 50;
    s.c11_member_tls = 60;
}
