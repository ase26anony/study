/* Define TLS variables with various attributes to test emutls_decl copying */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int c11_public_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local int c11_static_tls = 77;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;
__thread int weak_undefined_tls __attribute__((weak));

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* TLS with visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 200;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 300;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 400;

/* DLL import/export simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 500;
__declspec(dllimport) __thread int imported_tls_var;
#else
__thread int __attribute__((visibility("default"))) exported_tls_var = 500;
__thread int __attribute__((visibility("default"))) imported_tls_var = 600;
#endif

/* TLS in different contexts */
struct S {
    __thread int member_tls;
    __thread long member_tls2;
};

/* Force usage to set TREE_USED */
void use_tls_vars(void) {
    /* Take addresses to ensure variables are marked as used */
    volatile int* ptr1 = &public_tls_var;
    volatile int* ptr2 = &static_tls_var;
    volatile int* ptr3 = &weak_tls_var;
    volatile int* ptr4 = &common_tls_var;
    volatile int* ptr5 = &hidden_tls_var;
    volatile int* ptr6 = &protected_tls_var;
    volatile int* ptr7 = &internal_tls_var;
    volatile int* ptr8 = &exported_tls_var;
    volatile int* ptr9 = &imported_tls_var;
    
    /* Use in inline assembly to prevent optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3));
    asm volatile("" : : "r"(ptr4), "r"(ptr5), "r"(ptr6));
    asm volatile("" : : "r"(ptr7), "r"(ptr8), "r"(ptr9));
    
    /* Access variables */
    public_tls_var++;
    static_tls_var--;
    weak_tls_var *= 2;
    common_tls_var = 999;
}

/* TLS variable with complex initializer */
__thread int computed_tls_var = (sizeof(int) > 2) ? 1000 : 2000;

/* TLS array */
__thread int tls_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

/* Function to ensure DECL_PRESERVE_P might be set */
__attribute__((noinline)) void preserve_tls(void) {
    /* Complex usage pattern that might affect preservation */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = i * public_tls_var;
    }
    
    /* Reference all TLS variables in a way that's hard to optimize */
    volatile int sink = 0;
    sink += public_tls_var;
    sink += static_tls_var;
    sink += weak_tls_var;
    sink += common_tls_var;
    sink += hidden_tls_var;
    sink += protected_tls_var;
    sink += internal_tls_var;
    sink += exported_tls_var;
    sink += imported_tls_var;
    sink += computed_tls_var;
    
    asm volatile("" : : "r"(sink));
}
