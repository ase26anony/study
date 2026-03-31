/* tls_defs.c - Definitions of TLS variables with diverse attributes */

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

/* Preserve this declaration - used in inline assembly */
__thread int preserved_tls_var = 700;

/* TLS in different contexts */
struct ThreadLocalStruct {
    __thread int member_tls;
};

/* Function that uses TLS variables to ensure they're marked as used */
void use_tls_variables(void) {
    /* Take addresses to ensure TREE_USED is set */
    int *ptr1 = &public_tls_var;
    int *ptr2 = &static_tls_var;
    int *ptr3 = &weak_tls_var;
    int *ptr4 = &hidden_tls_var;
    
    /* Use in volatile context to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4));
    
    /* Access preserved TLS variable */
    preserved_tls_var = 750;
    
    /* Use in another volatile context */
    asm volatile ("" : : "r"(&preserved_tls_var));
}

/* Define the external TLS variable */
__thread int external_tls_var = 800;
