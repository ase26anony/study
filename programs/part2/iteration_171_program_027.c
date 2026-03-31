/* Test for TLS emulation attribute copying - Main file */
#include <stddef.h>

/* Prevent optimization */
#define KEEP(expr) do { \
    static volatile void *__keep_ptr; \
    __keep_ptr = (void*)(expr); \
    (void)__keep_ptr; \
} while(0)

/* Attribute availability checks */
#ifdef __GNUC__
#define HAVE_WEAK 1
#define HAVE_VISIBILITY 1
#define HAVE_USED 1
#else
#define HAVE_WEAK 0
#define HAVE_VISIBILITY 0
#define HAVE_USED 0
#endif

#ifdef _WIN32
#define HAVE_DLLIMPORT 1
#else
#define HAVE_DLLIMPORT 0
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Basic TLS with initialization - tests DECL_PRESERVE_P, TREE_USED */
#if HAVE_USED
__attribute__((used))
#endif
__thread int tls_used = 42;  /* Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */

/* 2. Weak TLS variable - tests DECL_WEAK */
#if HAVE_WEAK
__attribute__((weak))
#endif
__thread int tls_weak;  /* Tests: DECL_WEAK, TREE_PUBLIC */

/* 3. Hidden visibility TLS - tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
#if HAVE_VISIBILITY
__attribute__((visibility("hidden")))
#endif
__thread int tls_hidden = 100;  /* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* 4. Protected visibility TLS */
#if HAVE_VISIBILITY
__attribute__((visibility("protected")))
#endif
__thread int tls_protected;  /* Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */

/* 5. Static TLS (non-public) - tests !TREE_PUBLIC */
static __thread int tls_static = 7;  /* Tests: !TREE_PUBLIC, DECL_CONTEXT (function scope) */

/* 6. External declaration (defined in aux file) - tests DECL_EXTERNAL */
extern __thread int tls_external;  /* Tests: DECL_EXTERNAL, TREE_PUBLIC */

/* 7. Common TLS variable (tentative definition) - tests DECL_COMMON */
__thread int tls_common;  /* Tests: DECL_COMMON, TREE_PUBLIC */

/* 8. DLL Import style (Windows) - tests DECL_DLLIMPORT_P */
#if HAVE_DLLIMPORT
__declspec(dllimport) __thread int tls_dllimport;
#elif HAVE_WEAK
__attribute__((weak)) __thread int tls_dllimport;
#else
__thread int tls_dllimport;
#endif

/* Function to create DECL_CONTEXT for static TLS */
static void create_context(void) {
    /* tls_static has this function as DECL_CONTEXT */
    tls_static++;
}

/* Function that uses all TLS variables to prevent optimization */
void use_all_tls(void) {
    /* Take addresses to force TLS emulation structure creation */
    KEEP(&tls_used);
    KEEP(&tls_weak);
    KEEP(&tls_hidden);
    KEEP(&tls_protected);
    KEEP(&tls_static);
    KEEP(&tls_external);
    KEEP(&tls_common);
    KEEP(&tls_dllimport);
    
    /* Read-modify-write operations */
    tls_used += 1;
    tls_weak = tls_used;
    tls_hidden -= 1;
    tls_protected = tls_hidden * 2;
    create_context();  /* Uses tls_static */
    tls_common = 999;
    
    /* Inline asm to ensure preservation */
    #ifdef __GNUC__
    asm volatile("" : : "r"(&tls_used), "r"(&tls_hidden));
    #endif
}

/* Main function */
int main(void) {
    /* Initialize some values */
    tls_used = 1;
    tls_weak = 2;
    tls_hidden = 3;
    tls_protected = 4;
    tls_static = 5;
    tls_common = 6;
    
    /* Use all TLS variables */
    use_all_tls();
    
    /* Reference external TLS (defined in aux file) */
    tls_external = 100;
    
    /* Take addresses again in main */
    void *addrs[] = {
        &tls_used,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_static,
        &tls_external,
        &tls_common,
        &tls_dllimport
    };
    
    KEEP(addrs);
    
    return 0;
}
