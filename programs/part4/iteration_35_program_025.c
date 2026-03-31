/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS even if native TLS is available */
#pragma GCC target("tls")

/* Public TLS variable with external linkage */
__thread int public_tls_var = 42;

/* Static TLS variable with internal linkage */
static __thread int static_tls_var = 100;

/* Weak TLS variable that can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* Common TLS variable (tentative definition) */
__thread int common_tls_var __attribute__((common));

/* TLS variable with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;

/* TLS variable with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* TLS variable with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 500;

/* DLL export simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 600;
#else
__thread int exported_tls_var __attribute__((visibility("default"))) = 600;
#endif

/* TLS variable in different context (struct member) */
struct S {
    __thread int member_tls;
};

/* Global struct with TLS member */
struct S global_struct = {0};

/* Function that uses TLS variables to ensure they're marked as used */
void use_tls_vars_defs(void) {
    /* Take addresses to ensure TREE_USED is set */
    volatile int *ptr1 = &public_tls_var;
    volatile int *ptr2 = &static_tls_var;
    volatile int *ptr3 = &weak_tls_var;
    volatile int *ptr4 = &common_tls_var;
    volatile int *ptr5 = &hidden_tls_var;
    volatile int *ptr6 = &protected_tls_var;
    volatile int *ptr7 = &internal_tls_var;
    volatile int *ptr8 = &exported_tls_var;
    volatile int *ptr9 = &global_struct.member_tls;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), 
                   "r"(ptr4), "r"(ptr5), "r"(ptr6),
                   "r"(ptr7), "r"(ptr8), "r"(ptr9));
    
    /* Modify values */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    global_struct.member_tls++;
}

/* C11 thread_local variable */
_Thread_local int c11_tls_var = 700;

/* Thread-local with both weak and visibility attributes */
__thread int weak_hidden_tls_var __attribute__((weak, visibility("hidden"))) = 800;

/* Preserve attribute testing - use in noinline function */
__attribute__((noinline)) 
void preserve_tls_usage(void) {
    volatile int val = public_tls_var + static_tls_var;
    asm volatile("" : : "r"(val));
}
