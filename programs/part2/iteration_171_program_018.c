/* Test for TLS emulation attribute copying coverage */
#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Attribute availability checks */
#ifdef __GNUC__
#define HAVE_ATTRIBUTE(x) 1
#else
#define HAVE_ATTRIBUTE(x) 0
#endif

/* Test 1: Basic TLS with default visibility and used attribute */
__thread int tls_basic = 42;
#if HAVE_ATTRIBUTE(used)
__thread int tls_used __attribute__((used)) = 100;
#else
__thread int tls_used = 100;
#endif

/* Test 2: Weak TLS variable */
#if HAVE_ATTRIBUTE(weak)
__thread int tls_weak __attribute__((weak)) = 200;
#else
/* Fallback - some compilers use __declspec(weak) */
__thread int tls_weak = 200;
#endif

/* Test 3: Hidden visibility */
#if HAVE_ATTRIBUTE(visibility)
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;
#else
__thread int tls_hidden = 300;
#endif

/* Test 4: Protected visibility */
#if HAVE_ATTRIBUTE(visibility)
__thread int tls_protected __attribute__((visibility("protected"))) = 400;
#else
__thread int tls_protected = 400;
#endif

/* Test 5: External declaration (will be defined in another file) */
extern __thread int tls_external;

/* Test 6: Common TLS (tentative definition) */
__thread int tls_common;  /* Tests DECL_COMMON */

/* Test 7: Public TLS variable */
__thread int tls_public = 700;

/* Test 8: Static TLS (non-public, has DECL_CONTEXT) */
static __thread int tls_static = 800;

/* Test 9: DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__CYGWIN__) || defined(__MINGW32__)
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use a different mechanism or skip */
extern __thread int tls_dllimport;
#endif

/* Function to take addresses and use TLS variables */
void use_tls_variables(void) {
    /* Take addresses to ensure variables are processed */
    int *ptr;
    
    ptr = &tls_basic;          /* Tests DECL_PRESERVE_P, TREE_USED */
    KEEP_ALIVE(tls_basic);
    
    ptr = &tls_used;           /* Tests DECL_PRESERVE_P explicitly */
    KEEP_ALIVE(tls_used);
    
    ptr = &tls_weak;           /* Tests DECL_WEAK */
    KEEP_ALIVE(tls_weak);
    
    ptr = &tls_hidden;         /* Tests DECL_VISIBILITY(hidden) */
    KEEP_ALIVE(tls_hidden);
    
    ptr = &tls_protected;      /* Tests DECL_VISIBILITY(protected) */
    KEEP_ALIVE(tls_protected);
    
    ptr = &tls_external;       /* Tests DECL_EXTERNAL */
    KEEP_ALIVE(tls_external);
    
    ptr = &tls_common;         /* Tests DECL_COMMON */
    KEEP_ALIVE(tls_common);
    
    ptr = &tls_public;         /* Tests TREE_PUBLIC */
    KEEP_ALIVE(tls_public);
    
    ptr = &tls_static;         /* Tests DECL_CONTEXT (static scope) */
    KEEP_ALIVE(tls_static);
    
    ptr = &tls_dllimport;      /* Tests DECL_DLLIMPORT_P */
    KEEP_ALIVE(tls_dllimport);
    
    /* Modify some variables to ensure they're not optimized away */
    tls_basic += 1;
    tls_used += 2;
    tls_common = 123;  /* Initialize the common variable */
}

/* Nested function to create additional DECL_CONTEXT */
static void nested_context(void) {
    /* Local TLS variable inside a function - has non-NULL DECL_CONTEXT */
    static __thread int tls_in_function = 900;
    KEEP_ALIVE(tls_in_function);
    tls_in_function += 1;
}

int main(void) {
    use_tls_variables();
    nested_context();
    
    /* Simple validation that TLS is working */
    if (tls_basic != 43) return 1;
    if (tls_used != 102) return 2;
    if (tls_common != 123) return 3;
    
    return 0;
}
