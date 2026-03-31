/* TLS definitions with various attributes */

/* Public TLS with default visibility */
__thread int public_tls_var = 42;

/* Static TLS (internal linkage) */
static __thread int static_tls_var = 100;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* Hidden visibility TLS */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;

/* Protected visibility TLS */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* Internal visibility TLS */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 500;

/* DLL export simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 600;
#else
__thread int exported_tls_var __attribute__((visibility("default"))) = 600;
#endif

/* TLS in different contexts */
struct S {
    __thread int member_tls;
};

/* Function to force usage of TLS variables */
void use_tls_defs(void) {
    /* Take addresses to ensure TREE_USED is set */
    int *ptr1 = &public_tls_var;
    int *ptr2 = &static_tls_var;
    int *ptr3 = &weak_tls_var;
    int *ptr4 = &hidden_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4));
    
    /* Access to ensure they're marked used */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    
    /* Use struct member TLS */
    struct S s;
    s.member_tls = 700;
    
    /* Force preservation */
    asm volatile ("# TLS Vars: %0 %1 %2 %3 %4 %5 %6" 
                  : : "r"(public_tls_var), "r"(static_tls_var), 
                      "r"(weak_tls_var), "r"(hidden_tls_var),
                      "r"(protected_tls_var), "r"(internal_tls_var),
                      "r"(exported_tls_var));
}
