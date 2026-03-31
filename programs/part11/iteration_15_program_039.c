/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Explicit emulated TLS with various attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_used __attribute__((used)) = 42;
__thread int tls_not_used = 0;

/* TREE_PUBLIC / DECL_EXTERNAL mix */
__thread int tls_public = 100;           /* TREE_PUBLIC = 1 */
static __thread int tls_static = 200;    /* TREE_PUBLIC = 0 */

/* DECL_COMMON - tentative definition */
__thread int tls_common;                 /* Should be DECL_COMMON = 1 */

/* DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY variations */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;
__thread int tls_protected __attribute__((visibility("protected"))) = 500;
__thread int tls_internal __attribute__((visibility("internal"))) = 600;
/* Default visibility is implicit */

/* DECL_DLLIMPORT_P - target specific */
#ifdef _WIN32
extern __thread int tls_imported __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_imported __attribute__((dllimport));
#endif

/* DECL_CONTEXT - different scopes */
static void func_with_tls(void) {
    /* Function scope TLS */
    static __thread int tls_func_scope = 700;
    tls_func_scope++;
}

/* Pattern C: Complex usage to ensure processing */
__thread int* tls_ptr;
__thread int tls_for_asm;

/* Pattern D: Different TLS models for contrast */
#ifdef __GNUC__
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 800;
#endif

/* External declarations (will be defined in another file) */
extern __thread int tls_extern;
extern __thread int tls_extern_hidden __attribute__((visibility("hidden")));

/* Weak alias test */
__thread int tls_original = 900;
extern __thread int tls_alias __attribute__((weak, alias("tls_original")));

/* Usage to set TREE_USED flag */
void use_tls_vars(void) {
    /* Read and write to ensure TREE_USED = 1 */
    tls_used += 1;
    tls_not_used = tls_used;
    tls_public *= 2;
    tls_static -= 50;
    tls_common = tls_public + tls_static;
    tls_weak = tls_common / 2;
    tls_hidden++;
    tls_protected--;
    tls_internal = tls_hidden | tls_protected;
    
    /* Complex usage patterns */
    tls_ptr = &tls_public;
    *tls_ptr = 999;
    
    /* Use in inline asm to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_for_asm));
    
#ifdef __GNUC__
    tls_global_dynamic += 1000;
#endif
    
    tls_extern = 1234;
    tls_extern_hidden = 5678;
    
    func_with_tls();
    
    /* Use the alias */
    tls_alias = tls_original + 1;
}

int main(void) {
    int sum = 0;
    
    /* Initialize and use all TLS variables */
    use_tls_vars();
    
    /* Calculate sum for observable behavior */
    sum += tls_used;
    sum += tls_not_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    
#ifdef __GNUC__
    sum += tls_global_dynamic;
#endif
    
    sum += tls_extern;
    sum += tls_extern_hidden;
    sum += tls_original;
    sum += tls_alias;
    
    printf("TLS sum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
