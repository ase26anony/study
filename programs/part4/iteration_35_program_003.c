/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS usage */
#pragma GCC optimize("O0")

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int public_c11_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local float static_c11_float = 3.14f;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;
__thread char weak_char_tls __attribute__((weak)) = 'A';

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));
__thread long common_long_tls __attribute__((common)) = 500;

/* Visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 200;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 300;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 400;

/* DLL import/export simulation */
#ifdef _WIN32
__thread int __declspec(dllexport) exported_tls_var = 500;
#else
__thread int __attribute__((visibility("default"))) exported_tls_var = 500;
#endif

/* TLS in different contexts */
struct ThreadLocalStruct {
    __thread int member_tls;
    __thread double member_double;
};

__thread struct ThreadLocalStruct tls_struct = {0, 0.0};

/* Function to ensure TLS variables are used */
void use_tls_variables_defs(void) {
    /* Take addresses to ensure variables are marked as used */
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
                   "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8) : "memory");
    
    /* Access TLS variables */
    public_tls_var++;
    static_tls_var *= 2;
    weak_tls_var -= 5;
    common_tls_var = 999;
    hidden_tls_var |= 0xFF;
    protected_tls_var &= 0x0F;
    internal_tls_var ^= 0x55;
    exported_tls_var /= 2;
    
    /* Use struct member TLS */
    tls_struct.member_tls = 123;
    tls_struct.member_double = 456.789;
}
