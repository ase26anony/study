/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS usage */
#ifdef __clang__
#define THREAD_LOCAL _Thread_local
#else
#define THREAD_LOCAL __thread
#endif

/* Public TLS with external linkage */
THREAD_LOCAL int public_tls_var = 42;

/* Static TLS with internal linkage */
static THREAD_LOCAL int static_tls_var = 100;

/* Weak TLS symbol */
THREAD_LOCAL int weak_tls_var __attribute__((weak)) = 200;

/* Common linkage TLS */
THREAD_LOCAL int common_tls_var __attribute__((common));

/* Hidden visibility TLS */
THREAD_LOCAL int hidden_tls_var __attribute__((visibility("hidden"))) = 300;

/* Protected visibility TLS */
THREAD_LOCAL int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* Internal visibility TLS */
THREAD_LOCAL int internal_tls_var __attribute__((visibility("internal"))) = 500;

/* DLL export simulation */
#ifdef _WIN32
THREAD_LOCAL int __declspec(dllexport) exported_tls_var = 600;
#else
THREAD_LOCAL int exported_tls_var __attribute__((visibility("default"))) = 600;
#endif

/* TLS in different contexts */
struct S {
    THREAD_LOCAL int member_tls;
};

/* Function to force usage of TLS variables */
void use_tls_variables(void) {
    /* Take addresses to ensure TREE_USED is set */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    volatile int* ptr4 = &common_tls_var;
    volatile int* ptr5 = &hidden_tls_var;
    volatile int* ptr6 = &protected_tls_var;
    volatile int* ptr7 = &internal_tls_var;
    volatile int* ptr8 = &exported_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4),
                   "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8));
    
    /* Modify values */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    
    /* Use struct member TLS */
    struct S s;
    s.member_tls = 999;
    volatile int* ptr9 = &s.member_tls;
    asm volatile("" : : "r"(ptr9));
}
