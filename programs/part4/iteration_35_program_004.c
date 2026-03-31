/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS variable with external linkage */
__thread int public_tls_var = 42;

/* Static TLS variable with internal linkage */
static __thread int static_tls_var = 100;

/* Weak TLS variable that can be overridden */
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
__thread int exported_tls_var __attribute__((dllexport)) = 600;

/* C11 thread_local variable */
_Thread_local int c11_tls_var = 700;

/* TLS variable in different context - inside a struct */
struct ThreadData {
    __thread int member_tls;
    __thread long member_tls2;
};

/* Force usage to set TREE_USED flag */
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
    volatile int *ptr9 = &c11_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), 
                   "r"(ptr4), "r"(ptr5), "r"(ptr6),
                   "r"(ptr7), "r"(ptr8), "r"(ptr9));
    
    /* Access struct member TLS */
    struct ThreadData data;
    data.member_tls = 123;
    data.member_tls2 = 456;
    
    /* Use the values */
    public_tls_var += 1;
    static_tls_var += 2;
    weak_tls_var += 3;
}
