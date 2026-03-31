/* TLS definitions with various attributes */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 10;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* Hidden visibility TLS */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 200;

/* Protected visibility TLS */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 300;

/* Internal visibility TLS */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 400;

/* DLL export simulation */
__thread int exported_tls_var __attribute__((dllexport)) = 500;

/* TLS in different contexts */
struct S {
    __thread int member_tls;
};

/* Function to ensure TLS variables are marked as used */
void mark_tls_used(void) {
    /* Take addresses to ensure TREE_USED is set */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    volatile int* ptr4 = &common_tls_var;
    volatile int* ptr5 = &hidden_tls_var;
    volatile int* ptr6 = &protected_tls_var;
    volatile int* ptr7 = &internal_tls_var;
    volatile int* ptr8 = &exported_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4),
                      "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8));
    
    /* Access the variables */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
}

/* C11 thread_local variables */
_Thread_local int c11_public_tls = 123;
static _Thread_local int c11_static_tls = 456;
_Thread_local int c11_weak_tls __attribute__((weak)) = 789;

/* TLS with preserve attribute (simulated via used) */
__thread int preserved_tls_var __attribute__((used)) = 999;

/* Complex TLS usage pattern */
__thread struct {
    int a;
    char b;
    double c;
} complex_tls_var = {1, 'x', 3.14};
