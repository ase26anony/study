/* Test program for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
__thread int tls_used_var __attribute__((used)) = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK */
__thread int tls_weak_var __attribute__((weak)) = 100;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 200;

/* Test 4: Protected visibility TLS */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 300;

/* Test 5: External declaration (defined in another file) */
/* Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external_var;

/* Test 6: Common TLS (tentative definition) */
/* Tests: DECL_COMMON */
__thread int tls_common_var;

/* Test 7: DLL import style (simulated with weak) */
/* Tests: DECL_DLLIMPORT_P (when configured appropriately) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#else
/* Use weak as approximation for non-Windows */
__thread int tls_dllimport_var __attribute__((weak));
#endif

/* Test 8: Static TLS with internal linkage */
/* Tests: !TREE_PUBLIC contrast case */
static __thread int tls_static_var = 500;

/* Test 9: TLS in function scope for DECL_CONTEXT */
static void test_function_scope(void) {
    /* Local TLS variable - will have function as DECL_CONTEXT */
    static __thread int tls_local_in_func = 600;
    
    /* Use it to prevent optimization */
    asm volatile("" : : "r"(&tls_local_in_func));
}

/* Test 10: TLS with noinit section attribute */
__thread int tls_noinit_var __attribute__((section(".tls_noinit")));

/* Forward declaration for function in auxiliary file */
void use_tls_variables(void);

int main(void) {
    int result = 0;
    
    /* Take addresses to ensure variables are referenced */
    int *ptr1 = &tls_used_var;
    int *ptr2 = &tls_weak_var;
    int *ptr3 = &tls_hidden_var;
    int *ptr4 = &tls_protected_var;
    int *ptr5 = &tls_external_var;
    int *ptr6 = &tls_common_var;
    int *ptr7 = &tls_dllimport_var;
    int *ptr8 = &tls_static_var;
    
    /* Use asm to prevent optimization but ensure variables are referenced */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4),
                       "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8));
    
    /* Read-modify-write operations */
    tls_used_var += 1;
    tls_weak_var *= 2;
    tls_hidden_var -= 10;
    tls_protected_var /= 3;
    
    /* Initialize common variable */
    tls_common_var = 999;
    
    /* Call function that uses TLS variables */
    use_tls_variables();
    
    /* Test function scope TLS */
    test_function_scope();
    
    /* Simple computation using TLS variables */
    result = tls_used_var + tls_weak_var + tls_hidden_var + 
             tls_protected_var + tls_external_var + tls_common_var;
    
    return result > 0 ? 0 : 1;
}

#ifdef __cplusplus
}
#endif
