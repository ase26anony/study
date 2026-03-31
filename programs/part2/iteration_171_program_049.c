/* Test for TLS emulation attribute copying - main file */
/* Compile with: gcc -O2 -femulated-tls -fprofile-arcs -ftest-coverage emutls_test.c emutls_aux.c -o emutls_test */

#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Test 1: Public TLS variable with default visibility and used attribute */
/* Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */
__attribute__((used)) __thread int tls_public_used = 42;

/* Test 2: Weak TLS variable */
/* Tests: DECL_WEAK, TREE_PUBLIC */
__attribute__((weak)) __thread int tls_weak_var;

/* Test 3: Hidden visibility TLS variable */
/* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 100;

/* Test 4: Protected visibility TLS variable */
__attribute__((visibility("protected"))) __thread int tls_protected;

/* Test 5: Static TLS variable (not public) */
/* Tests: TREE_PUBLIC = 0, has DECL_CONTEXT */
static __thread int tls_static_local;

/* Test 6: External declaration (defined in aux file) */
/* Tests: DECL_EXTERNAL */
extern __thread int tls_external_def;

/* Test 7: Common TLS variable (tentative definition) */
/* Tests: DECL_COMMON */
__thread int tls_common_var;

/* Function to give context to some variables */
static void func_with_tls(void) {
    /* Test 8: TLS variable with function context */
    /* Tests: DECL_CONTEXT (non-NULL) */
    static __thread int tls_in_function;
    tls_in_function++;
    KEEP_ALIVE(tls_in_function);
}

/* Test 9: DLL import style attribute (for MinGW/Cygwin) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__GNUC__)
/* Simulate similar behavior with visibility */
__attribute__((visibility("default"))) __thread int tls_dllimport_sim;
#endif

/* Declaration from aux file */
void use_tls_variables(void);

int main(void) {
    /* Ensure all TLS variables are referenced to prevent elimination */
    
    /* Test 1: Public used variable */
    tls_public_used += 1;
    KEEP_ALIVE(tls_public_used);
    
    /* Test 2: Weak variable */
    if (&tls_weak_var != NULL) {
        tls_weak_var = 10;
    }
    KEEP_ALIVE(tls_weak_var);
    
    /* Test 3: Hidden variable */
    tls_hidden = 50;
    KEEP_ALIVE(tls_hidden);
    
    /* Test 4: Protected variable */
    tls_protected = 75;
    KEEP_ALIVE(tls_protected);
    
    /* Test 5: Static local */
    tls_static_local = 20;
    KEEP_ALIVE(tls_static_local);
    
    /* Test 6: External variable */
    tls_external_def = 30;
    KEEP_ALIVE(tls_external_def);
    
    /* Test 7: Common variable */
    tls_common_var = 40;
    KEEP_ALIVE(tls_common_var);
    
    /* Test 8: Variable in function */
    func_with_tls();
    
    /* Test 9: DLL import style */
#ifdef _WIN32
    KEEP_ALIVE(tls_dllimport);
#elif defined(__GNUC__)
    tls_dllimport_sim = 60;
    KEEP_ALIVE(tls_dllimport_sim);
#endif
    
    /* Use variables from aux file */
    use_tls_variables();
    
    return 0;
}
