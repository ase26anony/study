/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS variable with external linkage */
__thread int public_tls_var = 42;

/* Static TLS variable with internal linkage */
static __thread int static_tls_var = 10;

/* Weak TLS variable that can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Common TLS variable (tentative definition) */
__thread int common_tls_var __attribute__((common));

/* TLS variables with different visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 200;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 300;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 400;

/* DLL export simulation */
__thread int __attribute__((dllexport)) exported_tls_var = 500;

/* TLS variable that will be imported in another file */
__thread int imported_tls_var = 600;

/* TLS variable in a struct context */
struct S {
    __thread int member_tls;
};

/* Global struct with TLS member */
struct S global_struct = {0};

/* Function to ensure TLS variables are used */
void use_tls_vars_defs(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int *ptr1 = &public_tls_var;
    volatile int *ptr2 = &static_tls_var;
    volatile int *ptr3 = &weak_tls_var;
    volatile int *ptr4 = &common_tls_var;
    volatile int *ptr5 = &hidden_tls_var;
    volatile int *ptr6 = &protected_tls_var;
    volatile int *ptr7 = &internal_tls_var;
    volatile int *ptr8 = &exported_tls_var;
    volatile int *ptr9 = &imported_tls_var;
    volatile int *ptr10 = &global_struct.member_tls;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), 
                   "r"(ptr4), "r"(ptr5), "r"(ptr6),
                   "r"(ptr7), "r"(ptr8), "r"(ptr9), "r"(ptr10));
    
    /* Modify values */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    imported_tls_var++;
    global_struct.member_tls++;
}
