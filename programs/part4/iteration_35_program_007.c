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

/* DLL import simulation (declaration only, defined elsewhere) */
extern __thread int imported_tls_var __attribute__((dllimport));

/* TLS variable that will be preserved */
__thread int preserved_tls_var = 700;

/* TLS in different contexts */
struct S {
    __thread int member_tls;
};

/* Force usage to set TREE_USED flag */
void mark_tls_used(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    volatile int* ptr4 = &hidden_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile ("" : : "r"(&common_tls_var));
    asm volatile ("" : : "r"(&protected_tls_var));
    asm volatile ("" : : "r"(&internal_tls_var));
    
    /* Access to ensure they're not optimized away */
    (void)public_tls_var;
    (void)static_tls_var;
    
    /* Mark as preserved */
    asm volatile ("# %0" : : "r"(&preserved_tls_var));
}

/* Thread-local in function scope */
void func_with_tls(void) {
    static __thread int func_static_tls = 800;
    func_static_tls++;
}

/* C11 thread_local */
_Thread_local int c11_tls_var = 900;

/* Weak external reference */
extern __thread int weak_external_tls __attribute__((weak));
