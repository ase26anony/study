/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS usage */
#pragma GCC optimize("O0")

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

/* DLL export simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 600;
#else
__thread int exported_tls_var __attribute__((visibility("default"))) = 600;
#endif

/* TLS variable that will be imported in another file */
__thread int external_tls_var_def = 700;

/* TLS variable in a struct to test DECL_CONTEXT */
struct ThreadData {
    __thread int member_tls;
    __thread long member_tls2;
};

/* Define and use struct TLS */
struct ThreadData thread_data;

/* Function that uses TLS variables to ensure they're marked as used */
void use_tls_variables(void) {
    /* Take addresses to ensure TREE_USED is set */
    int *ptr1 = &public_tls_var;
    int *ptr2 = &static_tls_var;
    int *ptr3 = &weak_tls_var;
    int *ptr4 = &common_tls_var;
    int *ptr5 = &hidden_tls_var;
    int *ptr6 = &protected_tls_var;
    int *ptr7 = &internal_tls_var;
    int *ptr8 = &exported_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4),
                     "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8));
    
    /* Access struct member TLS */
    thread_data.member_tls = 123;
    thread_data.member_tls2 = 456;
    
    /* Use volatile access pattern */
    volatile int temp = public_tls_var;
    temp += static_tls_var;
    (void)temp;
}

/* Another TLS variable defined in a function scope */
void define_function_local_tls(void) {
    static __thread int func_static_tls = 800;
    __thread int func_auto_tls = 900;
    
    /* Use them to ensure they're preserved */
    func_static_tls++;
    func_auto_tls++;
    
    /* Take address */
    int *ptr = &func_static_tls;
    asm volatile("" : : "r"(ptr));
}
