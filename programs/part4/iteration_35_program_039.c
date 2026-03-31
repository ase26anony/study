/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 10;

/* Weak TLS symbol that can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 100;

/* Common linkage TLS (tentative definition) */
__thread int common_tls_var __attribute__((common));

/* TLS with hidden visibility */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 50;

/* TLS with protected visibility */
__thread int protected_tls_var __attribute__((visibility("protected"))) = 60;

/* TLS with internal visibility */
__thread int internal_tls_var __attribute__((visibility("internal"))) = 70;

/* DLL exported TLS variable */
__thread int __attribute__((dllexport)) exported_tls_var = 300;

/* DLL imported TLS variable (declaration only) */
extern __thread int __attribute__((dllimport)) imported_tls_var;

/* TLS variable that will be preserved */
__thread int preserved_tls_var __attribute__((used)) = 80;

/* TLS in different contexts */
struct S {
    __thread int member_tls;
};

/* Function to ensure TLS variables are marked as used */
void mark_tls_used(void) {
    /* Take addresses to ensure TREE_USED is set */
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
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4),
                    "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8), "r"(ptr9));
    
    /* Access struct member TLS */
    struct S s;
    s.member_tls = 90;
    int* ptr10 = &s.member_tls;
    asm volatile ("" : : "r"(ptr10));
}
