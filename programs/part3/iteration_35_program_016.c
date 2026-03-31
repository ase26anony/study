/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by targeting ARM without hardware TLS */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

/* DECL_PRESERVE_P: used attribute */
__thread int tls_used __attribute__((used)) = 42;

/* TREE_PUBLIC: non-static (public) TLS */
__thread int tls_public = 100;

/* TREE_PUBLIC: static (non-public) TLS */
static __thread int tls_static = 200;

/* DECL_COMMON: TLS without initializer (common linkage) */
__thread int tls_common;

/* DECL_WEAK: weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY: default visibility (explicit) */
__thread int tls_default __attribute__((visibility("default"))) = 500;

/* DECL_CONTEXT: TLS inside function scope */
void function_with_tls(void) {
    __thread int tls_local = 600;
    tls_local++;  /* Ensure TREE_USED */
}

/* External TLS declaration (will be defined in another file) */
extern __thread int tls_external;

/* Ensure all TLS variables are marked TREE_USED */
void use_all_tls(void) {
    tls_used++;
    tls_public++;
    tls_static++;
    tls_common = 1;
    tls_weak++;
    tls_hidden++;
    tls_default++;
    
    /* Use external TLS */
    tls_external++;
    
    function_with_tls();
}

/* DLL import simulation for DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Main function that uses everything */
int main(void) {
    use_all_tls();
    
    /* Additional uses to ensure coverage */
    tls_dllimport = 123;
    
    return 0;
}
