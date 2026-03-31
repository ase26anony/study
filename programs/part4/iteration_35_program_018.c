/* Define TLS variables with various attributes to test emutls_decl logic */

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

/* C11 thread_local variable */
_Thread_local int c11_tls_var = 700;

/* TLS in different contexts */
struct S {
    __thread int member_tls;
};

/* Global instance with TLS member */
struct S global_struct = {0};

/* Function to force usage of TLS variables */
void use_tls_vars(void) {
    /* Take addresses to force TREE_USED and DECL_PRESERVE_P */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &common_tls_var;
    int* ptr5 = &hidden_tls_var;
    int* ptr6 = &protected_tls_var;
    int* ptr7 = &internal_tls_var;
    int* ptr8 = &exported_tls_var;
    int* ptr9 = &c11_tls_var;
    int* ptr10 = &global_struct.member_tls;
    
    /* Use in volatile context to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4), 
                   "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8), "r"(ptr9), "r"(ptr10));
    
    /* Modify values */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    c11_tls_var++;
    global_struct.member_tls++;
}
