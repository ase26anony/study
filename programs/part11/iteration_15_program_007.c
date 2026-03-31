/* Test for emulated TLS attribute copying - C version */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Force emulated TLS with compiler flag */
/* We'll compile with -ftls-model=emulated */

/* DECL_PRESERVE_P: Variables that should not be eliminated */
__thread int tls_preserve __attribute__((used)) = 42;
__thread int tls_not_preserve = 100;

/* TREE_USED: Variables that are referenced */
__thread int tls_used1 = 1;
__thread int tls_used2 = 2;
static __thread int tls_static_used = 3;

/* TREE_PUBLIC / DECL_EXTERNAL: Mix of public and static */
__thread int tls_public = 10;           /* Public */
static __thread int tls_file_static = 20; /* File static */
extern __thread int tls_extern;         /* External declaration */

/* DECL_COMMON: Tentative definitions */
__thread int tls_common;                /* Common symbol */

/* DECL_WEAK: Weak symbols */
__thread int tls_weak __attribute__((weak)) = 30;
extern __thread int tls_weak_undefined __attribute__((weak));

/* DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_default __attribute__((visibility("default"))) = 40;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 50;
__thread int tls_protected __attribute__((visibility("protected"))) = 60;
__thread int tls_internal __attribute__((visibility("internal"))) = 70;
__thread int tls_no_visibility = 80;    /* No explicit visibility */

/* DECL_DLLIMPORT_P: Target-specific */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* DECL_CONTEXT: Different scopes */
static void function_with_tls(void) {
    /* Function scope TLS */
    static __thread int tls_function_scope = 90;
    tls_function_scope++;
}

/* Pattern C: Use in complex expressions */
__thread int* tls_pointer;
__thread int tls_for_asm;

/* Helper function to ensure variables are used */
void use_tls_variables(void) {
    /* Ensure TREE_USED is set for these variables */
    tls_used1++;
    tls_used2 += 2;
    tls_static_used *= 3;
    
    /* Use public/static variables */
    tls_public = tls_file_static + 1;
    
    /* Initialize common variable */
    tls_common = 123;
    
    /* Use weak variables if defined */
    if (&tls_weak) {
        tls_weak++;
    }
    
    /* Use visibility-controlled variables */
    tls_default++;
    tls_hidden--;
    tls_protected *= 2;
    tls_internal /= 2;
    tls_no_visibility += 5;
    
    /* Complex usage patterns */
    tls_pointer = &tls_public;
    *tls_pointer = 999;
    
    /* Use in inline asm to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_for_asm));
    
    /* Call function with scope-specific TLS */
    function_with_tls();
}

/* Pattern D: Different TLS models */
#ifdef __GNUC__
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 200;
__thread int tls_emulated __attribute__((tls_model("emulated"))) = 300;
#endif

int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    use_tls_variables();
    
    /* Sum up values to ensure they're all used */
    sum += tls_preserve;
    sum += tls_not_preserve;
    sum += tls_used1;
    sum += tls_used2;
    sum += tls_static_used;
    sum += tls_public;
    sum += tls_file_static;
    sum += tls_common;
    
    if (&tls_weak) sum += tls_weak;
    
    sum += tls_default;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    sum += tls_no_visibility;
    
#ifdef __GNUC__
    sum += tls_global_dynamic;
    sum += tls_emulated;
#endif
    
    /* Use external variable if linked */
    extern __thread int tls_extern_defined;
    sum += tls_extern_defined;
    
    printf("TLS sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
