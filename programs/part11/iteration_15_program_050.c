/* tls_common.c - Core TLS definitions with various attributes */

/* Force emulated TLS model for testing */
#pragma GCC tls_model emulated

/* DECL_PRESERVE_P: used attribute prevents elimination */
__thread int tls_used __attribute__((used)) = 42;
__thread int tls_not_used = 0;

/* TREE_PUBLIC: non-static (public) TLS variables */
__thread int tls_public = 100;
static __thread int tls_static = 200;

/* DECL_COMMON: tentative definition (common symbol) */
__thread int tls_common;  /* No initializer at file scope */

/* DECL_WEAK: weak TLS variables */
__thread int tls_weak __attribute__((weak)) = 300;
__thread int tls_strong = 400;

/* DECL_VISIBILITY: various visibility attributes */
__thread int tls_default __attribute__((visibility("default"))) = 500;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 600;
__thread int tls_protected __attribute__((visibility("protected"))) = 700;
__thread int tls_internal __attribute__((visibility("internal"))) = 800;

/* DECL_EXTERNAL: extern declaration (defined elsewhere) */
extern __thread int tls_extern;

/* DECL_DLLIMPORT_P: platform-specific import attribute */
#ifdef _WIN32
extern __thread int tls_import __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_import __attribute__((dllimport));
#endif

/* Function to ensure TREE_USED is set for some variables */
void use_tls_variables(void) {
    /* Reference variables to mark them as used */
    volatile int x = tls_used;
    x += tls_public;
    x += tls_static;
    x += tls_common;
    x += tls_weak;
    x += tls_strong;
    x += tls_default;
    x += tls_hidden;
    x += tls_protected;
    x += tls_internal;
    x += tls_extern;
    
    /* Take addresses to ensure they're processed */
    (void)&tls_used;
    (void)&tls_public;
    (void)&tls_static;
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_weak));
}
