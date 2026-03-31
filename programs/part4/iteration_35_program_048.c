/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS by using appropriate attributes/declarations */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;

/* Static TLS with internal linkage */
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

/* Function to ensure TLS variables are marked as used */
void mark_tls_used(void) {
    /* Take addresses to ensure TREE_USED is set */
    volatile int *ptr1 = &public_tls_var;
    volatile int *ptr2 = &static_tls_var;
    volatile int *ptr3 = &weak_tls_var;
    volatile int *ptr4 = &common_tls_var;
    volatile int *ptr5 = &hidden_tls_var;
    volatile int *ptr6 = &protected_tls_var;
    volatile int *ptr7 = &internal_tls_var;
    volatile int *ptr8 = &exported_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4),
                   "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8));
    
    /* Access to ensure they're referenced */
    public_tls_var += 1;
    static_tls_var += 1;
    weak_tls_var += 1;
    common_tls_var += 1;
    hidden_tls_var += 1;
    protected_tls_var += 1;
    internal_tls_var += 1;
    exported_tls_var += 1;
}

/* C11 thread_local variables */
_Thread_local int c11_public_tls = 700;
static _Thread_local int c11_static_tls = 800;
_Thread_local int c11_weak_tls __attribute__((weak)) = 900;

/* Complex TLS usage pattern */
__thread int* tls_ptr_array[10];

/* Initialize some TLS pointers */
void init_tls_pointers(void) {
    for (int i = 0; i < 10; i++) {
        tls_ptr_array[i] = &public_tls_var + i;
    }
}
