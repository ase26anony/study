/* TLS definitions with various attributes */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 10;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* TLS with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 50;

/* TLS with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 60;

/* TLS with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 70;

/* DLL export simulation */
__thread int exported_tls_var __attribute__((dllexport)) = 300;

/* TLS in different contexts */
struct S {
    __thread int member_tls;
};

/* Function to force usage of TLS variables */
void use_tls_defs(void) {
    /* Take addresses to ensure TREE_USED is set */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &hidden_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4));
    
    /* Access variables */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var = 99;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    
    /* Use struct member TLS */
    struct S s;
    s.member_tls = 123;
}
