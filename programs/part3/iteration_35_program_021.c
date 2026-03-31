/* Main test file with various TLS variables and attributes */

/* Force EMUTLS by targeting ARM without hardware TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden */

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
    __thread int tls_function_scope = 600;
    tls_function_scope++;  /* Ensure TREE_USED */
}

/* DECL_EXTERNAL: external TLS declaration (defined in another file) */
extern __thread int tls_external;

/* DECL_DLLIMPORT_P: Windows-specific attribute */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Read and write to each TLS variable */
    tls_used++;
    tls_public = tls_used * 2;
    tls_static += 3;
    tls_common = tls_public + tls_static;
    
    if (&tls_weak) {  /* Reference weak symbol */
        tls_weak = 999;
    }
    
    tls_hidden--;
    tls_default *= 2;
    
    /* Reference external TLS */
    tls_external = tls_default + 1;
    
    /* Reference DLL import TLS */
    tls_dllimport = 1234;
    
    /* Call function with function-scope TLS */
    function_with_tls();
}

int main(void) {
    reference_all_tls();
    
    /* Additional references to ensure optimization doesn't remove them */
    volatile int sum = 0;
    sum += tls_used;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default;
    sum += tls_external;
    
    return sum > 0 ? 0 : 1;
}
