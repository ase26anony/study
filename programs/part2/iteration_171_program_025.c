/* Test for TLS emulation attribute copying coverage */
/* This file contains main() and various TLS definitions */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Test 1: Basic TLS with used attribute - tests DECL_PRESERVE_P */
__thread int tls_used_var __attribute__((used)) = 42;

/* Test 2: Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int tls_public_var __attribute__((visibility("default"))) = 100;

/* Test 3: Weak TLS variable - tests DECL_WEAK */
__thread int tls_weak_var __attribute__((weak)) = 200;

/* Test 4: Hidden visibility TLS - tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 300;

/* Test 5: Protected visibility TLS */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 400;

/* Test 6: Common TLS (tentative definition) - tests DECL_COMMON */
__thread int tls_common_var;  /* Tentative definition */

/* Test 7: External TLS declaration (defined in aux file) - tests DECL_EXTERNAL */
extern __thread int tls_external_var;

/* Test 8: DLL import style (simulated with weak) for DECL_DLLIMPORT_P */
/* On MinGW/Cygwin targets, we'd use __declspec(dllimport) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#else
/* Simulate with weak external */
extern __thread int tls_dllimport_var __attribute__((weak));
#endif

/* Function to test TLS in function scope - gives DECL_CONTEXT */
static void test_function_scope(void) {
    /* Test 9: TLS with function context - tests DECL_CONTEXT */
    static __thread int tls_function_local = 999;
    
    /* Use it to prevent optimization */
    tls_function_local++;
    asm volatile("" : : "r"(&tls_function_local));
}

/* Test 10: TLS in global scope but with complex type */
struct ComplexType {
    int a;
    double b;
    char c[10];
};
__thread struct ComplexType tls_complex __attribute__((used));

/* Forward declaration for function in aux file */
void aux_function(void);

int main(void) {
    /* Take addresses and use all TLS variables to ensure they're processed */
    
    /* Test 1: Used variable */
    int *p1 = &tls_used_var;
    tls_used_var += 1;
    
    /* Test 2: Public variable */
    int *p2 = &tls_public_var;
    tls_public_var = tls_used_var * 2;
    
    /* Test 3: Weak variable */
    int *p3 = &tls_weak_var;
    tls_weak_var = 123;
    
    /* Test 4: Hidden variable */
    int *p4 = &tls_hidden_var;
    tls_hidden_var = tls_weak_var + 1;
    
    /* Test 5: Protected variable */
    int *p5 = &tls_protected_var;
    tls_protected_var = tls_hidden_var * 2;
    
    /* Test 6: Common variable */
    int *p6 = &tls_common_var;
    tls_common_var = 789;
    
    /* Test 7: External variable (defined in aux) */
    int *p7 = &tls_external_var;
    tls_external_var = 456;  /* Write to external */
    
    /* Test 8: DLL import style variable */
    int *p8 = &tls_dllimport_var;
    /* Just take address, don't write (might be weak undefined) */
    
    /* Test 9: Function scope TLS */
    test_function_scope();
    
    /* Test 10: Complex type TLS */
    struct ComplexType *p10 = &tls_complex;
    tls_complex.a = 42;
    tls_complex.b = 3.14;
    tls_complex.c[0] = 'X';
    
    /* Use asm to prevent optimization of address-taking */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5),
                       "r"(p6), "r"(p7), "r"(p8), "r"(p10));
    
    /* Call function from aux file */
    aux_function();
    
    /* Simple validation */
    int sum = tls_used_var + tls_public_var + tls_weak_var + 
              tls_hidden_var + tls_protected_var + tls_common_var +
              tls_external_var + tls_complex.a;
    
    return (sum > 0) ? 0 : 1;
}

#ifdef __cplusplus
}
#endif
