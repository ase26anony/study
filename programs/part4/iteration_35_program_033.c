/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS usage */
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int public_c11_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local int static_c11_tls = 13;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;
_Thread_local int weak_c11_tls __attribute__((weak)) = 200;

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));
_Thread_local int common_c11_tls __attribute__((common));

/* TLS with visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 1;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 2;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 3;

/* DLL import/export simulation (GCC style) */
#ifdef _WIN32
__thread int __declspec(dllexport) exported_tls_var = 300;
__thread int __declspec(dllimport) imported_tls_var;
#else
__thread int __attribute__((visibility("default"))) exported_tls_var = 300;
/* Simulate dllimport with external declaration */
extern __thread int imported_tls_var;
#endif

/* TLS in different contexts */
struct S {
    __thread int member_tls;
    _Thread_local int member_c11_tls;
};

/* Function to force usage of TLS variables */
void use_tls_vars_defs(void) {
    /* Take addresses to force TREE_USED */
    volatile int *ptr1 = &public_tls_var;
    volatile int *ptr2 = &static_tls_var;
    volatile int *ptr3 = &weak_tls_var;
    volatile int *ptr4 = &common_tls_var;
    volatile int *ptr5 = &hidden_tls_var;
    volatile int *ptr6 = &protected_tls_var;
    volatile int *ptr7 = &internal_tls_var;
    volatile int *ptr8 = &exported_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), 
                  "r"(ptr4), "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8));
    
    /* Modify values */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var = 50;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    
    /* Use C11 TLS vars */
    public_c11_tls++;
    static_c11_tls++;
    weak_c11_tls++;
    common_c11_tls = 60;
}
