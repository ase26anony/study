/* TLS definitions with various attributes to test emutls_decl logic */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 100;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* TLS with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;

/* TLS with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* TLS with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 500;

/* DLL export simulation */
__thread int exported_tls_var __attribute__((dllexport)) = 600;

/* TLS variable that will be preserved */
__thread int preserved_tls_var __attribute__((used)) = 700;

/* TLS in different context - struct member */
struct ThreadData {
    __thread int member_tls;
    __thread long member_tls2;
};

/* Force usage to set TREE_USED */
void use_tls_vars(void) {
    /* Take addresses to ensure variables are marked as used */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &common_tls_var;
    int* ptr5 = &hidden_tls_var;
    int* ptr6 = &protected_tls_var;
    int* ptr7 = &internal_tls_var;
    int* ptr8 = &exported_tls_var;
    int* ptr9 = &preserved_tls_var;
    
    /* Use in volatile context to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    asm volatile ("" : : "r"(ptr4), "r"(ptr5), "r"(ptr6));
    asm volatile ("" : : "r"(ptr7), "r"(ptr8), "r"(ptr9));
    
    /* Access struct member TLS */
    struct ThreadData data;
    data.member_tls = 123;
    data.member_tls2 = 456;
    
    /* Force compiler to consider all TLS variables as used */
    volatile int dummy = 
        public_tls_var + static_tls_var + weak_tls_var + 
        common_tls_var + hidden_tls_var + protected_tls_var +
        internal_tls_var + exported_tls_var + preserved_tls_var +
        data.member_tls + (int)data.member_tls2;
    
    (void)dummy; /* Suppress unused variable warning */
}
