/* Test for TLS emulation attribute copying - Main file */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#include <stdio.h>

/* Guard for C++ compilation */
#ifdef __cplusplus
extern "C" {
#endif

/* Test 1: Public TLS with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */
__attribute__((used))
__thread int tls_public_used = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
__attribute__((weak))
__thread int tls_weak_var = 100;

/* Test 3: Hidden visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden")))
__thread int tls_hidden = 200;

/* Test 4: Protected visibility TLS */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("protected")))
__thread int tls_protected = 300;

/* Test 5: Common TLS (tentative definition) */
/* Tests: DECL_COMMON */
__thread int tls_common;  /* Tentative definition - becomes common */

/* Test 6: Static TLS with internal linkage */
/* Tests: !TREE_PUBLIC (contrast), DECL_CONTEXT (from function scope) */
static void test_function(void) {
    static __thread int tls_static_func = 500;  /* Has function as DECL_CONTEXT */
    (void)tls_static_func;  /* Prevent unused warning */
}

/* Test 7: External TLS declaration (defined in aux file) */
/* Tests: DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 8: DLL import style attribute (simulated for non-Windows) */
/* Tests: DECL_DLLIMPORT_P (when supported) */
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__GNUC__)
/* Simulate with alias on non-Windows */
__thread int tls_dllimport_target = 999;
__attribute__((weak, alias("tls_dllimport_target")))
__thread int tls_dllimport;
#endif

/* Function to ensure TLS variables are used */
void use_tls_variables(void) {
    /* Take addresses to prevent optimization */
    int *ptr1 = &tls_public_used;
    int *ptr2 = &tls_weak_var;
    int *ptr3 = &tls_hidden;
    int *ptr4 = &tls_protected;
    int *ptr5 = &tls_common;
    extern int *ptr6;  /* tls_external pointer from aux file */
    int *ptr7 = &tls_dllimport;
    
    /* Read-modify-write to ensure variables are live */
    tls_public_used += 1;
    tls_weak_var += 2;
    tls_hidden += 3;
    tls_protected += 4;
    tls_common += 5;
    
    /* Use inline asm to mark variables as used without optimization */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4), "r"(ptr5), "r"(ptr7));
    
    /* Call function with static TLS */
    test_function();
}

/* Main function */
int main(void) {
    printf("Testing TLS emulation attribute copying...\n");
    
    /* Initialize common TLS */
    tls_common = 50;
    
    /* Use all TLS variables */
    use_tls_variables();
    
    /* Call function from aux file that uses external TLS */
    use_external_tls();
    
    printf("tls_public_used = %d\n", tls_public_used);
    printf("tls_weak_var = %d\n", tls_weak_var);
    printf("tls_hidden = %d\n", tls_hidden);
    printf("tls_protected = %d\n", tls_protected);
    printf("tls_common = %d\n", tls_common);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
