/* tls_defs.c - TLS variable definitions with various attributes */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 10;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* TLS with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 200;

/* TLS with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 300;

/* TLS with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 400;

/* DLL export simulation */
__thread int exported_tls_var __attribute__((dllexport)) = 500;

/* TLS variable that will be used in assembly to ensure TREE_USED */
__thread int used_tls_var = 600;

/* TLS in different context - struct member */
struct S {
    __thread int member_tls;
};

/* Function to force preservation of TLS variables */
void mark_tls_used(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int *ptr1 = &public_tls_var;
    volatile int *ptr2 = &static_tls_var;
    volatile int *ptr3 = &weak_tls_var;
    volatile int *ptr4 = &common_tls_var;
    volatile int *ptr5 = &hidden_tls_var;
    volatile int *ptr6 = &protected_tls_var;
    volatile int *ptr7 = &internal_tls_var;
    volatile int *ptr8 = &exported_tls_var;
    volatile int *ptr9 = &used_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), 
                   "r"(ptr4), "r"(ptr5), "r"(ptr6),
                   "r"(ptr7), "r"(ptr8), "r"(ptr9));
    
    /* Access struct member TLS */
    struct S s;
    s.member_tls = 700;
    volatile int *ptr10 = &s.member_tls;
    asm volatile ("" : : "r"(ptr10));
}

/* C11 thread_local variables */
_Thread_local int c11_tls_var = 800;
_Thread_local static int c11_static_tls_var = 900;
_Thread_local int c11_weak_tls_var __attribute__((weak)) = 1000;
