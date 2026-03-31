/* test_emutls.c - Main test file for emulated TLS attribute coverage */

/* Pattern A: Explicit emulated TLS flag will be used during compilation */

#include <stdio.h>

/* 1. DECL_PRESERVE_P - Variables that should not be eliminated */
__thread int tls_preserve_used __attribute__((used));
__thread int tls_preserve_regular;

/* 2. TREE_USED - Variables that are referenced in code */
__thread int tls_used_var = 42;
static __thread int tls_static_used;

/* 3. TREE_PUBLIC / DECL_EXTERNAL - Mix of public and static */
__thread int tls_public = 100;
static __thread int tls_file_static = 200;
extern __thread int tls_extern_var;  /* Defined in another file */

/* 4. DECL_COMMON - Tentative definitions */
__thread int tls_common;  /* No initializer - common symbol */

/* 5. DECL_WEAK - Weak symbols */
__thread int tls_weak_var __attribute__((weak)) = 300;

/* 6. DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;
__thread int tls_protected __attribute__((visibility("protected"))) = 500;
__thread int tls_internal __attribute__((visibility("internal"))) = 600;
/* Default visibility is implicit */

/* 7. DECL_DLLIMPORT_P - Target-specific */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* 8. DECL_CONTEXT - Different scopes */
static void function_with_tls(void) {
    /* TLS in function scope */
    static __thread int tls_in_function = 700;
    tls_in_function++;
}

/* Helper function to ensure TLS variables are used */
void use_tls_variables(void) {
    /* Ensure TREE_USED is set for all variables */
    tls_preserve_used = 1;
    tls_preserve_regular = 2;
    
    tls_used_var += 1;
    tls_static_used = tls_used_var;
    
    tls_public *= 2;
    tls_file_static -= 1;
    
    tls_common = 999;
    
    if (&tls_weak_var) {
        tls_weak_var += 100;
    }
    
    tls_hidden++;
    tls_protected++;
    tls_internal++;
    
    function_with_tls();
    
    /* Take addresses to force more complex handling */
    void *addr1 = &tls_public;
    void *addr2 = &tls_hidden;
    (void)addr1;
    (void)addr2;
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_used_var));
}

/* Pattern C: Use in complex expressions */
int complex_tls_usage(int x) {
    static __thread int counter = 0;
    counter += x;
    
    /* Non-trivial expression with TLS */
    return tls_public + tls_hidden + counter + tls_weak_var;
}

int main(void) {
    int result = 0;
    
    /* Initialize and use all TLS variables */
    use_tls_variables();
    
    /* Pattern C continued: More complex usage */
    result += complex_tls_usage(10);
    result += complex_tls_usage(20);
    
    /* Use extern variable if available */
    result += tls_extern_var;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("tls_public: %d\n", tls_public);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_common: %d\n", tls_common);
    
    return 0;
}
