/* Test for EMUTLS attribute copying - C version */

/* Force EMUTLS by targeting architecture without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_preserved __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) */
static __thread int tls_static = 200;

/* DECL_COMMON: no initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak attribute */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS inside function (local scope) */
void function_with_tls(void) {
    __thread int tls_local = 600;
    tls_local++;  /* Ensure TREE_USED */
}

/* DECL_EXTERNAL: external declaration (defined in another file) */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: Windows-specific */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Force usage of all TLS variables */
    tls_preserved++;
    tls_public++;
    tls_static++;
    tls_common = 123;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    tls_external++;
    tls_dllimport = 789;
    
    function_with_tls();
}

int main(void) {
    reference_all_tls();
    return 0;
}
