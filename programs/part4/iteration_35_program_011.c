/* TLS definitions with various attributes */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int public_c11_tls = 123;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 99;
static _Thread_local int static_c11_tls = 456;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;
__thread int weak_uninit_tls __attribute__((weak));

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
struct ThreadLocalStruct {
    __thread int member_tls;
    __thread long member_tls2;
};

/* Force usage to set TREE_USED */
void use_tls_vars(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    volatile int* ptr4 = &hidden_tls_var;
    volatile int* ptr5 = &exported_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4), "r"(ptr5));
    
    /* Access variables */
    public_tls_var++;
    static_tls_var--;
    weak_tls_var *= 2;
    hidden_tls_var += 10;
    exported_tls_var /= 2;
    
    /* Use common variable */
    common_tls_var = 999;
    
    /* Use TLS in struct */
    struct ThreadLocalStruct s;
    s.member_tls = 111;
    s.member_tls2 = 222;
    
    /* Force usage of C11 TLS */
    public_c11_tls = public_c11_tls * 2 + 1;
    static_c11_tls = static_c11_tls / 2;
}
