/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS variable with external linkage */
__thread int public_tls_var = 42;

/* Static TLS variable with internal linkage */
static __thread int static_tls_var = 10;

/* Weak TLS symbol that can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* TLS variable with common linkage (tentative definition) */
__thread int common_tls_var __attribute__((common));

/* TLS variables with different visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 50;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 60;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 70;

/* DLL import/export simulation (GCC equivalents) */
#ifdef _WIN32
__thread int __attribute__((dllimport)) imported_tls_var;
__thread int __attribute__((dllexport)) exported_tls_var = 300;
#else
/* On non-Windows, use visibility for similar effect */
__thread int imported_tls_var __attribute__((visibility("default")));
__thread int exported_tls_var __attribute__((visibility("default"))) = 300;
#endif

/* TLS variable that should be preserved */
__thread int preserve_tls_var __attribute__((used)) = 80;

/* C11 thread_local variable */
_Thread_local int c11_tls_var = 90;

/* TLS variable in a volatile context */
__thread volatile int volatile_tls_var = 110;

/* Function to ensure TLS variables are marked as used */
void mark_tls_used(void) {
    /* Take addresses to ensure TREE_USED is set */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &common_tls_var;
    int* ptr5 = &hidden_tls_var;
    int* ptr6 = &protected_tls_var;
    int* ptr7 = &internal_tls_var;
    int* ptr8 = &imported_tls_var;
    int* ptr9 = &exported_tls_var;
    int* ptr10 = &preserve_tls_var;
    int* ptr11 = &c11_tls_var;
    int* ptr12 = &volatile_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4),
                     "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8),
                     "r"(ptr9), "r"(ptr10), "r"(ptr11), "r"(ptr12));
}

/* TLS variable defined in a struct context */
struct ThreadData {
    __thread int member_tls;
    __thread long another_member __attribute__((visibility("hidden")));
};

/* Global struct with TLS members */
struct ThreadData global_thread_data;

/* Function-local static TLS */
void func_with_local_tls(void) {
    static __thread int func_static_tls = 123;
    func_static_tls++;
}
