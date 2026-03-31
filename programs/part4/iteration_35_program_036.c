/* Define TLS variables with diverse attributes to test emutls_decl copying */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 100;

/* Weak TLS symbol that can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* Common linkage TLS (tentative definition) */
__thread int common_tls_var __attribute__((common));

/* TLS with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;

/* TLS with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* TLS with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 500;

/* DLL export simulation */
__thread int exported_tls_var __attribute__((dllexport)) = 600;

/* DLL import declaration (will be defined elsewhere) */
extern __thread int imported_tls_var __attribute__((dllimport));

/* TLS in different contexts */
struct ThreadData {
    __thread int member_tls;
};

/* Force usage to set TREE_USED flag */
void mark_used(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    volatile int* ptr4 = &common_tls_var;
    volatile int* ptr5 = &hidden_tls_var;
    volatile int* ptr6 = &protected_tls_var;
    volatile int* ptr7 = &internal_tls_var;
    volatile int* ptr8 = &exported_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), 
                  "r"(ptr4), "r"(ptr5), "r"(ptr6), 
                  "r"(ptr7), "r"(ptr8));
}

/* C11 thread_local variables */
_Thread_local int c11_tls_var = 700;
static _Thread_local int c11_static_tls_var = 800;

/* TLS with preserve attribute */
__thread int preserve_tls_var __attribute__((used)) = 900;

/* Complex TLS usage pattern */
__thread struct {
    int a;
    double b;
} complex_tls_var = {1, 2.0};
