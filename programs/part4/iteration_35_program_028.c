/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS by using -femulated-tls flag during compilation */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int public_c11_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local long static_c11_tls = 77;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;
__thread int weak_undefined_tls __attribute__((weak));

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* TLS with visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 200;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 300;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 400;

/* DLL import/export simulation (GCC equivalents) */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 500;
__declspec(dllimport) __thread int imported_tls_var;
#else
/* GCC attributes for shared library visibility */
__thread int __attribute__((visibility("default"))) exported_tls_var = 500;
/* Simulate imported variable by declaring it extern */
extern __thread int imported_tls_var;
#endif

/* TLS in different contexts */
struct S {
    __thread int member_tls;
    __thread long another_member;
};

/* Function to ensure TLS variables are marked as used */
void mark_tls_used(void) {
    /* Take addresses to ensure TREE_USED is set */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    volatile int* ptr4 = &common_tls_var;
    volatile int* ptr5 = &hidden_tls_var;
    volatile int* ptr6 = &protected_tls_var;
    volatile int* ptr7 = &internal_tls_var;
    volatile int* ptr8 = &exported_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), 
                  "r"(ptr4), "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8));
    
    /* Access the variables */
    public_tls_var++;
    static_tls_var += 2;
    weak_tls_var *= 3;
    common_tls_var = public_tls_var + static_tls_var;
    hidden_tls_var ^= 0x55;
    protected_tls_var |= 0xAA;
    internal_tls_var &= 0xFF;
    exported_tls_var /= 2;
}

/* Thread-local variable with complex initializer */
__thread int computed_tls = sizeof(struct S);

/* TLS variable that might be preserved */
__thread int preserved_tls __attribute__((used)) = 999;

/* Make sure DECL_PRESERVE_P might be set */
void* get_preserved_tls_addr(void) {
    return &preserved_tls;
}
