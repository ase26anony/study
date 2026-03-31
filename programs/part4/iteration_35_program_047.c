/* Define TLS variables with various attributes to test emutls_decl logic */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int c11_public_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local int c11_static_tls = 21;

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
/* Simulate dllimport with extern + visibility */
extern __thread int imported_tls_var __attribute__((visibility("default")));
#endif

/* TLS in different contexts */
struct S {
    __thread int member_tls;
    _Thread_local long member_tls2;
};

/* Force usage to set TREE_USED */
void use_tls_vars(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    volatile int* ptr4 = &hidden_tls_var;
    volatile int* ptr5 = &protected_tls_var;
    volatile int* ptr6 = &common_tls_var;
    volatile int* ptr7 = &exported_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    asm volatile("" : : "r"(ptr4), "r"(ptr5), "r"(ptr6));
    
    /* Modify values */
    public_tls_var++;
    static_tls_var *= 2;
    hidden_tls_var -= 5;
    protected_tls_var |= 0xFF;
    
    /* Use C11 TLS variables */
    c11_public_tls = c11_static_tls + 1;
    
    /* Use struct member TLS */
    struct S s;
    s.member_tls = 999;
    s.member_tls2 = 888;
    
    /* Use volatile to ensure preservation */
    volatile int tmp = common_tls_var;
    common_tls_var = tmp + 1;
}
