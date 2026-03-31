/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

/* Pattern A: Explicit emulated TLS flag will be used during compilation */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_preserve_used __attribute__((used)) = 42;
__thread int tls_preserve_not_used = 100;  /* Not marked with used attribute */

/* TREE_USED - ensure variables are referenced */
__thread int tls_used_var = 1;
static __thread int tls_static_used = 2;

/* TREE_PUBLIC / DECL_EXTERNAL mix */
__thread int tls_public = 10;           /* Public, non-static */
static __thread int tls_file_static = 20; /* File static */
extern __thread int tls_extern_var;     /* External declaration */

/* DECL_COMMON - tentative definitions */
__thread int tls_common;                /* Common symbol (tentative definition) */
__thread int tls_common_initialized = 30; /* Not common (has initializer) */

/* DECL_WEAK */
__thread int tls_weak_var __attribute__((weak)) = 50;
__thread int tls_strong_var = 60;       /* Strong definition */

/* DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 70;
__thread int tls_default __attribute__((visibility("default"))) = 80;
__thread int tls_protected __attribute__((visibility("protected"))) = 90;
/* No visibility attribute specified */
__thread int tls_no_visibility_specified = 100;

/* DECL_CONTEXT - different scopes */
static void function_with_tls(void) {
    /* Function scope TLS */
    static __thread int tls_function_scope = 200;
    tls_function_scope++;
}

/* Complex usage to ensure processing */
void use_tls_vars(void) {
    /* Ensure all TLS vars are marked TREE_USED */
    tls_used_var += tls_static_used;
    tls_public = tls_file_static * 2;
    tls_common = tls_common_initialized;
    tls_weak_var = tls_strong_var;
    tls_hidden++;
    tls_default--;
    tls_protected *= 2;
    tls_no_visibility_specified /= 2;
    
    /* Take addresses to force more complex handling */
    int *ptr1 = &tls_used_var;
    int *ptr2 = &tls_public;
    (void)ptr1;
    (void)ptr2;
    
    function_with_tls();
}

/* Pattern C: Use in non-trivial ways */
int compute_with_tls(void) {
    int sum = 0;
    
    /* Use in arithmetic */
    sum += tls_used_var;
    sum += tls_static_used;
    sum += tls_public;
    sum += tls_file_static;
    sum += tls_common;
    sum += tls_common_initialized;
    sum += tls_weak_var;
    sum += tls_strong_var;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_protected;
    sum += tls_no_visibility_specified;
    
    /* Use in conditional */
    if (tls_used_var > 0) {
        sum += 1000;
    }
    
    return sum;
}

/* Pattern D: Mix with other TLS models */
#ifdef __GNUC__
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 999;
#endif

int main(void) {
    use_tls_vars();
    int result = compute_with_tls();
    
    /* Reference the extern variable */
    tls_extern_var = result % 100;
    
    /* Ensure all variables are truly used */
    printf("Result: %d\n", result);
    
    return 0;
}
