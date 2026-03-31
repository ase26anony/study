/* TLS definitions with various attributes to test emutls_decl copying */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 100;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* TLS with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;

/* TLS with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* TLS with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 500;

/* DLL export simulation */
__thread int exported_tls_var __attribute__((dllexport)) = 600;

/* TLS variable that will be preserved */
__thread int preserved_tls_var = 700;

/* TLS in different contexts */
struct ThreadLocalStruct {
    __thread int member_tls;
};

/* Function to force usage of TLS variables */
void use_tls_defs(void) {
    /* Take addresses to ensure TREE_USED is set */
    int *ptr1 = &public_tls_var;
    int *ptr2 = &static_tls_var;
    int *ptr3 = &weak_tls_var;
    int *ptr4 = &common_tls_var;
    int *ptr5 = &hidden_tls_var;
    int *ptr6 = &protected_tls_var;
    int *ptr7 = &internal_tls_var;
    int *ptr8 = &exported_tls_var;
    int *ptr9 = &preserved_tls_var;
    
    /* Use in volatile context */
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
    preserved_tls_var++;
}
