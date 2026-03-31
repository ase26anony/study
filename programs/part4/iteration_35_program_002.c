/* TLS definitions with various attributes to test emutls_decl logic */

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

/* TLS with different visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 200;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 300;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 400;

/* DLL import/export simulation (GNU attributes) */
#ifdef _WIN32
    /* Windows-style attributes */
    __declspec(dllexport) __thread int exported_tls_var = 500;
    __declspec(dllimport) __thread int imported_tls_var;
#else
    /* GNU-style attributes */
    __thread int __attribute__((dllexport)) exported_tls_var = 500;
    __thread int __attribute__((dllimport)) imported_tls_var;
#endif

/* TLS in different contexts */
struct S {
    __thread int member_tls;
};

/* Function to force usage of TLS variables */
void use_tls_vars(void) {
    /* Take addresses to ensure TREE_USED is set */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &common_tls_var;
    int* ptr5 = &hidden_tls_var;
    int* ptr6 = &protected_tls_var;
    int* ptr7 = &internal_tls_var;
    int* ptr8 = &exported_tls_var;
    
    /* Use in volatile context */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    
    /* Modify values */
    public_tls_var++;
    static_tls_var *= 2;
    weak_tls_var -= 5;
    common_tls_var = 999;
    hidden_tls_var = hidden_tls_var * 2 + 1;
    
    /* Use struct member TLS */
    struct S s;
    s.member_tls = 1234;
    
    /* Force preservation by using in inline asm */
    asm volatile ("# TLS Vars: %0, %1, %2" 
                  : : "r"(public_tls_var), "r"(static_tls_var), "r"(hidden_tls_var));
}

/* TLS variable with preservation likely needed */
__thread int preserve_needed_tls = 0;

/* Force DECL_PRESERVE_P by taking address in separate function */
void* get_preserve_addr(void) {
    return &preserve_needed_tls;
}
