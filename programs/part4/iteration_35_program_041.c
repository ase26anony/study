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

/* TLS variable in a struct context */
struct ThreadData {
    __thread int member_tls;
    __thread char* member_ptr;
};

/* Function to force usage of TLS variables */
void use_tls_variables(void) {
    /* Take addresses to force TREE_USED flag */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &common_tls_var;
    int* ptr5 = &hidden_tls_var;
    int* ptr6 = &protected_tls_var;
    int* ptr7 = &internal_tls_var;
    int* ptr8 = &exported_tls_var;
    int* ptr9 = &c11_tls_var;
    
    /* Use in volatile assembly to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    asm volatile ("" : : "r"(ptr4), "r"(ptr5), "r"(ptr6));
    asm volatile ("" : : "r"(ptr7), "r"(ptr8), "r"(ptr9));
    
    /* Access struct member TLS */
    struct ThreadData data;
    data.member_tls = 123;
    data.member_ptr = (char*)&data.member_tls;
    
    /* Complex usage pattern */
    public_tls_var += static_tls_var;
    hidden_tls_var ^= protected_tls_var;
    internal_tls_var |= exported_tls_var;
}

/* Preserve attribute test - function that references TLS */
__attribute__((used)) 
void preserve_tls_references(void) {
    /* Force DECL_PRESERVE_P by taking address in used function */
    static int* preserve_ptr = &public_tls_var;
}
