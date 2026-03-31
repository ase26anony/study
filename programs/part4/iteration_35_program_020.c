/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS variable with external linkage */
__thread int public_tls_var = 42;

/* Static TLS variable with internal linkage */
static __thread int static_tls_var = 100;

/* Weak TLS symbol that can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* Common TLS variable (tentative definition) */
__thread int common_tls_var __attribute__((common));

/* TLS variable with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;

/* TLS variable with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* TLS variable with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 500;

/* DLL exported TLS variable */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 600;
#else
__thread int exported_tls_var __attribute__((visibility("default"))) = 600;
#endif

/* TLS variable that will be imported in another file */
__thread int external_tls_var_def = 700;

/* TLS variable with preserve attribute (simulated via use in asm) */
__thread int preserve_tls_var = 800;

/* TLS variable in different context - inside a struct */
struct ThreadData {
    __thread int member_tls;
    __thread int another_member;
};

/* Global instance with TLS members */
struct ThreadData thread_data;

/* Function to force usage of TLS variables */
void use_tls_variables_defs(void) {
    /* Force TREE_USED to be set by using the variables */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    external_tls_var_def++;
    preserve_tls_var++;
    
    /* Use struct TLS members */
    thread_data.member_tls = 123;
    thread_data.another_member = 456;
    
    /* Take addresses to ensure variables are marked as used */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    
    /* Use in inline asm to prevent optimization and affect DECL_PRESERVE_P */
    asm volatile ("" : : "r"(&preserve_tls_var) : "memory");
    asm volatile ("" : : "r"(&hidden_tls_var) : "memory");
    
    /* Reference pointers to prevent unused variable warnings */
    (void)ptr1;
    (void)ptr2;
    (void)ptr3;
}

/* C11 thread_local variables */
_Thread_local int c11_tls_var = 900;
_Thread_local static int c11_static_tls_var = 1000;

/* Weak C11 TLS variable */
_Thread_local int c11_weak_tls_var __attribute__((weak)) = 1100;

/* Function using C11 TLS */
void use_c11_tls(void) {
    c11_tls_var++;
    c11_static_tls_var++;
    c11_weak_tls_var++;
}
