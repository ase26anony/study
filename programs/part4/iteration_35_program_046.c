/* Define TLS variables with various attributes to test emutls_decl logic */

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

/* TLS in different context - struct member */
struct ThreadLocalStruct {
    __thread int member_tls;
};

/* Force usage to set TREE_USED flag */
void use_tls_vars_in_defs(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int *ptr1 = &public_tls_var;
    volatile int *ptr2 = &static_tls_var;
    volatile int *ptr3 = &weak_tls_var;
    volatile int *ptr4 = &hidden_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile ("" : : "r"(&common_tls_var));
    asm volatile ("" : : "r"(&protected_tls_var));
    asm volatile ("" : : "r"(&internal_tls_var));
    
    /* Access to ensure usage */
    preserved_tls_var = public_tls_var + static_tls_var;
    
    /* Use struct member TLS */
    struct ThreadLocalStruct s;
    s.member_tls = 800;
}
