/* TLS definitions with various attributes to test emutls_decl logic */

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
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 600;
#else
__thread int exported_tls_var __attribute__((visibility("default"))) = 600;
#endif

/* TLS variable that will be declared extern in another file */
__thread int external_tls_var_def = 700;

/* TLS in different contexts */
struct S {
    __thread int member_tls;
};

/* Function to force usage of TLS variables */
void use_tls_vars_defs(void) {
    /* Take addresses to ensure TREE_USED is set */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &common_tls_var;
    int* ptr5 = &hidden_tls_var;
    int* ptr6 = &protected_tls_var;
    int* ptr7 = &internal_tls_var;
    int* ptr8 = &exported_tls_var;
    int* ptr9 = &external_tls_var_def;
    
    /* Use in volatile context to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), 
                   "r"(ptr4), "r"(ptr5), "r"(ptr6),
                   "r"(ptr7), "r"(ptr8), "r"(ptr9));
    
    /* Access variables to ensure they're marked used */
    public_tls_var += 1;
    static_tls_var += 2;
    weak_tls_var += 3;
    common_tls_var += 4;
    hidden_tls_var += 5;
    protected_tls_var += 6;
    internal_tls_var += 7;
    exported_tls_var += 8;
    external_tls_var_def += 9;
    
    /* TLS in struct context */
    struct S s;
    s.member_tls = 1000;
    int* ptr10 = &s.member_tls;
    asm volatile ("" : : "r"(ptr10));
}
