/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS variable with external linkage */
__thread int public_tls_var = 42;

/* Static TLS variable with internal linkage */
static __thread int static_tls_var = 10;

/* Weak TLS symbol that can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Common linkage TLS variable (tentative definition) */
__thread int common_tls_var __attribute__((common));

/* TLS variable with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 50;

/* TLS variable with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 60;

/* TLS variable with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 70;

/* DLL export simulation */
__thread int exported_tls_var __attribute__((dllexport)) = 300;

/* TLS variable in different context - struct member */
struct S {
    __thread int member_tls;
};

/* Define the struct TLS variable */
struct S s_instance = {0};

/* Function to force usage of TLS variables */
void use_tls_vars_defs(void) {
    /* Take addresses to ensure TREE_USED is set */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &common_tls_var;
    int* ptr5 = &hidden_tls_var;
    int* ptr6 = &protected_tls_var;
    int* ptr7 = &internal_tls_var;
    int* ptr8 = &exported_tls_var;
    int* ptr9 = &s_instance.member_tls;
    
    /* Use in volatile context to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    asm volatile ("" : : "r"(ptr4), "r"(ptr5), "r"(ptr6));
    asm volatile ("" : : "r"(ptr7), "r"(ptr8), "r"(ptr9));
    
    /* Modify values */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    s_instance.member_tls++;
}
