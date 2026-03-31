/* Test for TLS emulation attribute copying - main file */
#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP(var) asm volatile("" : : "r"(&(var)))

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

/* Test 1: Basic TLS with preservation (tests DECL_PRESERVE_P, TREE_USED) */
#if HAVE_USED
__thread int tls_used __attribute__((used)) = 42;
#else
__thread int tls_used = 42;
#endif

/* Test 2: Public TLS with default visibility (tests TREE_PUBLIC, DECL_VISIBILITY) */
__thread int tls_public = 100;

/* Test 3: External declaration (tests DECL_EXTERNAL) */
extern __thread int tls_external;

/* Test 4: Common linkage (tests DECL_COMMON) */
__thread int tls_common;  /* Tentative definition */

/* Test 5: Weak linkage (tests DECL_WEAK) */
#if HAVE_WEAK
__thread int tls_weak __attribute__((weak)) = 200;
#else
__thread int tls_weak = 200;
#endif

/* Test 6: Hidden visibility (tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED) */
#if HAVE_VISIBILITY
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;
#else
__thread int tls_hidden = 300;
#endif

/* Test 7: Protected visibility */
#if HAVE_VISIBILITY
__thread int tls_protected __attribute__((visibility("protected"))) = 400;
#else
__thread int tls_protected = 400;
#endif

/* Test 8: Static TLS with context (tests DECL_CONTEXT for non-NULL) */
static void func_with_tls(void) {
    static __thread int tls_in_function = 500;
    KEEP(tls_in_function);
}

/* Test 9: DLL import simulation (tests DECL_DLLIMPORT_P) */
/* For MinGW/Cygwin targets */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Simulate with weak external */
extern __thread int tls_dllimport __attribute__((weak));
#endif

/* Function that uses all TLS variables to ensure they're not optimized away */
void use_all_tls(void) {
    /* Take addresses to force symbol references */
    int *ptrs[] = {
        &tls_used,
        &tls_public,
        &tls_external,
        &tls_common,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        #ifdef _WIN32
        &tls_dllimport,
        #endif
        NULL
    };
    
    /* Read-modify-write on some variables */
    tls_used += 1;
    tls_public *= 2;
    if (&tls_weak != NULL) {
        tls_weak -= 1;
    }
    
    /* Ensure all are referenced */
    for (int i = 0; ptrs[i] != NULL; i++) {
        KEEP(*ptrs[i]);
    }
    
    func_with_tls();
}

/* Main function */
int main(void) {
    /* Initialize some values */
    tls_common = 999;
    
    /* Use all TLS variables */
    use_all_tls();
    
    /* Call function from auxiliary file */
    use_aux_tls();
    
    /* Simple validation */
    if (tls_used != 43) return 1;
    if (tls_public != 200) return 2;
    
    return 0;
}
