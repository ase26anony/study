/* Test file for EMUTLS attribute copying - C version */

/* Force EMUTLS transformation by targeting non-TLS architecture */
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
__thread int tls_default_vis __attribute__((visibility("default"))) = 500;

/* DECL_DLLIMPORT_P: Windows dllimport (use appropriate flag) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Function-scoped TLS (tests DECL_CONTEXT) */
void func_with_tls(void) {
    /* DECL_CONTEXT: TLS inside function */
    static __thread int tls_in_func = 600;
    tls_in_func++;
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_tls_vars(void) {
    /* TREE_USED: read/write operations */
    tls_used += 1;
    tls_public = tls_used * 2;
    tls_static--;
    tls_common = 999;
    tls_weak = tls_public + tls_static;
    tls_hidden *= 2;
    tls_default_vis /= 2;
    tls_dllimport = 1234;
    
    func_with_tls();
}

/* External TLS declaration (DECL_EXTERNAL will be tested in another file) */
extern __thread int tls_external;

void use_external_tls(void) {
    tls_external = 8888;
}

int main(void) {
    reference_tls_vars();
    use_external_tls();
    return 0;
}
