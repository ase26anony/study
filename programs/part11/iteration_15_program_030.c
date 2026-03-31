/* Test for GCC emulated TLS attribute copying - C version */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Explicit emulated TLS model */
#ifdef __GNUC__
#define EMU_TLS __thread __attribute__((tls_model("emulated")))
#else
#define EMU_TLS __thread
#endif

/* Pattern D: Contrast with other TLS model */
#define GLOBAL_DYNAMIC_TLS __thread __attribute__((tls_model("global-dynamic")))

/* DECL_PRESERVE_P: Variables that won't be eliminated */
EMU_TLS int tls_preserve_used __attribute__((used)) = 42;
EMU_TLS int tls_preserve_normal = 100;

/* TREE_PUBLIC/DECL_EXTERNAL: Mix of public and static */
EMU_TLS int tls_public = 1;                    /* TREE_PUBLIC = 1 */
static EMU_TLS int tls_static = 2;             /* TREE_PUBLIC = 0 */

/* DECL_COMMON: Tentative definitions */
EMU_TLS int tls_common;                        /* Should be DECL_COMMON */
EMU_TLS int tls_common_initialized = 0;        /* Not common */

/* DECL_WEAK: Weak symbols */
EMU_TLS int tls_weak __attribute__((weak)) = 3;

/* DECL_VISIBILITY: Different visibility attributes */
EMU_TLS int tls_visible_default __attribute__((visibility("default"))) = 4;
EMU_TLS int tls_visible_hidden __attribute__((visibility("hidden"))) = 5;
EMU_TLS int tls_visible_protected __attribute__((visibility("protected"))) = 6;
#ifdef __linux__
EMU_TLS int tls_visible_internal __attribute__((visibility("internal"))) = 7;
#endif

/* DECL_CONTEXT: Different scopes */
static void test_function_scope(void) {
    /* Function scope TLS */
    static EMU_TLS int tls_function_scope = 8;
    tls_function_scope++;
}

/* Complex usage for TREE_USED */
EMU_TLS volatile int tls_used_complex = 0;

/* For DECL_DLLIMPORT_P - Windows specific */
#ifdef _WIN32
__declspec(dllimport) extern EMU_TLS int tls_imported;
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern EMU_TLS int tls_imported __attribute__((dllimport));
#endif

/* Pattern C: Complex expressions with TLS */
EMU_TLS int tls_for_address;
EMU_TLS int tls_for_asm;

/* Pattern D: Contrast variable */
GLOBAL_DYNAMIC_TLS int tls_global_dynamic = 9;

/* Helper function that uses TLS variables to ensure TREE_USED is set */
void use_tls_variables(void) {
    /* Reference all TLS variables to mark them as used */
    volatile int sum = 0;
    
    sum += tls_preserve_used;
    sum += tls_preserve_normal;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_common_initialized;
    sum += tls_weak;
    sum += tls_visible_default;
    sum += tls_visible_hidden;
    sum += tls_visible_protected;
#ifdef __linux__
    sum += tls_visible_internal;
#endif
    
    /* Complex usage patterns */
    int *ptr = &tls_for_address;
    tls_for_address = sum;
    
    /* Pattern C: Use in inline asm to prevent optimization */
    __asm__ volatile ("" : "+m" (tls_for_asm));
    tls_for_asm = sum;
    
    /* Use function scope TLS */
    test_function_scope();
    
    /* Use contrast variable */
    sum += tls_global_dynamic;
    
    /* Ensure tls_used_complex is marked used with complex pattern */
    tls_used_complex = (tls_used_complex * 1103515245 + 12345) & 0x7fffffff;
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(sum));
}

int main(void) {
    /* Initialize some variables */
    tls_common = 10;
    tls_common_initialized = 11;
    
    /* Use all TLS variables */
    use_tls_variables();
    
    /* Additional complex usage */
    for (int i = 0; i < 10; i++) {
        tls_public += tls_static;
        tls_visible_default++;
    }
    
    /* Print something to prevent optimization */
    printf("TLS test completed\n");
    printf("Values: %d %d %d\n", tls_public, tls_visible_default, tls_used_complex);
    
    return 0;
}
