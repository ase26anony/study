/* tls_main.c - Main test file for emulated TLS attribute coverage */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS for all variables */
#pragma GCC tls_model emulated

/* DECL_PRESERVE_P: Variable with 'used' attribute to prevent elimination */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_USED: Variable that will be referenced in code */
__thread int tls_referenced = 100;

/* TREE_PUBLIC: Public (non-static) TLS variable */
__thread int tls_public = 200;

/* Static TLS variable (not TREE_PUBLIC) */
static __thread int tls_static = 300;

/* DECL_COMMON: Tentative definition (common symbol) */
__thread int tls_common;

/* DECL_WEAK: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 400;

/* DECL_VISIBILITY: Different visibility attributes */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 500;
__thread int tls_protected __attribute__((visibility("protected"))) = 600;
__thread int tls_internal __attribute__((visibility("internal"))) = 700;

/* Default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 800;

/* DECL_VISIBILITY_SPECIFIED: Variable without explicit visibility */
__thread int tls_no_vis_specified = 900;

/* DECL_CONTEXT: File scope context */
__thread int tls_file_scope = 1000;

/* Function with static TLS variable (different DECL_CONTEXT) */
static void func_with_tls(void) {
    static __thread int tls_in_function = 1100;
    tls_in_function++;
}

/* DECL_EXTERNAL: External declaration (defined in another file) */
extern __thread int tls_external;

/* For DLL import testing on supported targets */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* Complex usage patterns to ensure processing */
__thread int* tls_pointer;
__thread struct {
    int a;
    int b;
} tls_struct = {1200, 1300};

/* C11 _Thread_local */
_Thread_local int tls_c11 = 1400;

/* Different TLS models for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 1500;

/* Address-taking and complex expressions */
static void use_tls_variables(void) {
    /* Ensure TREE_USED is set for all variables */
    int sum = 0;
    
    sum += tls_used;
    sum += tls_referenced;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_default;
    sum += tls_no_vis_specified;
    sum += tls_file_scope;
    sum += tls_external;
    
#ifdef _WIN32
    sum += tls_dllimport;
#elif defined(__CYGWIN__) || defined(__MINGW32__)
    sum += tls_dllimport;
#endif
    
    sum += *tls_pointer;
    sum += tls_struct.a + tls_struct.b;
    sum += tls_c11;
    sum += tls_global_dynamic;
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(sum));
    
    func_with_tls();
}

/* Weak alias test */
__thread int tls_original = 1600;
extern __thread int tls_alias __attribute__((weak, alias("tls_original")));

int main(void) {
    /* Initialize pointer */
    static int local_for_pointer = 1700;
    tls_pointer = &local_for_pointer;
    
    /* Initialize common variable */
    tls_common = 1800;
    
    /* Use all TLS variables */
    use_tls_variables();
    
    /* Modify and read back */
    tls_public++;
    tls_hidden++;
    tls_protected++;
    
    /* Complex expression with TLS */
    int result = tls_used + tls_referenced * 2 - tls_public;
    
    /* Ensure all variables are truly used */
    printf("Result: %d\n", result);
    printf("Common: %d\n", tls_common);
    printf("External: %d\n", tls_external);
    printf("Struct: %d %d\n", tls_struct.a, tls_struct.b);
    printf("C11 TLS: %d\n", tls_c11);
    printf("Global dynamic: %d\n", tls_global_dynamic);
    printf("Original/Alias: %d/%d\n", tls_original, tls_alias);
    
    return 0;
}
