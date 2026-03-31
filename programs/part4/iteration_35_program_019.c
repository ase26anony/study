/* Define TLS variables with various attributes to test emulated TLS attribute copying */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 100;

/* Weak TLS symbol that can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* TLS with common linkage (tentative definition) */
__thread int common_tls_var __attribute__((common));

/* TLS with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;

/* TLS with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* TLS with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 500;

/* DLL export simulation */
__thread int exported_tls_var __attribute__((dllexport)) = 600;

/* DLL import will be declared in another file */
/* This one is defined here for export */
__thread int imported_tls_var_def __attribute__((dllexport)) = 700;

/* TLS variable that will be preserved */
__thread int preserve_tls_var __attribute__((used)) = 800;

/* TLS in different context - inside a struct */
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
    volatile int *ptr9 = &imported_tls_var_def;
    volatile int *ptr10 = &preserve_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4), 
                   "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8), 
                   "r"(ptr9), "r"(ptr10));
    
    /* Access the variables */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var++;
    hidden_tls_var++;
    protected_tls_var++;
    internal_tls_var++;
    exported_tls_var++;
    imported_tls_var_def++;
    preserve_tls_var++;
    
    /* Use struct member TLS */
    struct ThreadData td;
    td.member_tls = 123;
    td.member_tls2 = 456;
    
    volatile int *ptr11 = &td.member_tls;
    volatile long *ptr12 = &td.member_tls2;
    asm volatile ("" : : "r"(ptr11), "r"(ptr12));
}

/* C11 thread_local variables */
_Thread_local int c11_tls_var = 900;
_Thread_local static int c11_static_tls_var = 1000;

/* TLS with both weak and visibility attributes */
__thread int weak_hidden_tls_var __attribute__((weak, visibility("hidden"))) = 1100;

/* Extern common declaration - will be defined elsewhere */
extern __thread int external_common_tls_var __attribute__((common));
