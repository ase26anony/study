/* Test program for TLS emulation attribute copying coverage */
/* This tests the copy_decl_attributes function in tree-emutls.cc */

#include <stddef.h>

/* Prevent optimization */
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

#ifdef _WIN32
#define HAVE_DLLIMPORT 1
#else
#define HAVE_DLLIMPORT 0
#endif

/* Test 1: TLS with __attribute__((used)) - tests DECL_PRESERVE_P */
/* This variable should be marked as used/preserved */
#ifdef HAVE_USED
__thread int tls_used __attribute__((used)) = 42;
#else
__thread int tls_used = 42;
#endif

/* Test 2: TLS with visibility("hidden") - tests DECL_VISIBILITY */
#ifdef HAVE_VISIBILITY
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;
#else
__thread int tls_hidden = 100;
#endif

/* Test 3: TLS with visibility("default") and specified - tests DECL_VISIBILITY_SPECIFIED */
#ifdef HAVE_VISIBILITY
__thread int tls_default __attribute__((visibility("default"))) = 200;
#else
__thread int tls_default = 200;
#endif

/* Test 4: External TLS declaration - tests DECL_EXTERNAL and TREE_PUBLIC */
/* Defined in emutls_aux.c */
extern __thread int tls_external;

/* Test 5: Weak TLS variable - tests DECL_WEAK */
#ifdef HAVE_WEAK
__thread int tls_weak __attribute__((weak)) = 300;
#else
__thread int tls_weak = 300;
#endif

/* Test 6: Common TLS variable (tentative definition) - tests DECL_COMMON */
/* This is a tentative definition that may become common */
__thread int tls_common;

/* Test 7: Static TLS variable (non-public) - contrasts TREE_PUBLIC */
static __thread int tls_static = 400;

/* Test 8: DLL import style (Windows) - tests DECL_DLLIMPORT_P */
#if HAVE_DLLIMPORT
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with visibility protected on non-Windows */
#ifdef HAVE_VISIBILITY
__thread int tls_dllimport __attribute__((visibility("protected"))) = 500;
#else
__thread int tls_dllimport = 500;
#endif
#endif

/* Function to create DECL_CONTEXT for some variables */
static void create_context(void) {
    /* Local TLS variable inside function - has DECL_CONTEXT */
    static __thread int tls_in_function = 600;
    
    /* Use it to prevent optimization */
    tls_in_function++;
    KEEP(tls_in_function);
}

/* Another function that references extern TLS */
void use_extern_tls(void);

int main(void) {
    /* Force all TLS variables to be processed */
    
    /* Test 1: Used variable */
    tls_used += 1;
    KEEP(tls_used);
    
    /* Test 2: Hidden visibility */
    tls_hidden *= 2;
    KEEP(tls_hidden);
    
    /* Test 3: Default visibility */
    tls_default -= 10;
    KEEP(tls_default);
    
    /* Test 4: External (defined elsewhere) */
    /* Take address to force processing */
    int *p_external = &tls_external;
    KEEP(p_external);
    
    /* Test 5: Weak */
    tls_weak = 999;
    KEEP(tls_weak);
    
    /* Test 6: Common */
    tls_common = 111;
    KEEP(tls_common);
    
    /* Test 7: Static */
    tls_static++;
    KEEP(tls_static);
    
    /* Test 8: DLL import style */
    tls_dllimport = 222;
    KEEP(tls_dllimport);
    
    /* Create context for some variables */
    create_context();
    
    /* Use extern TLS from other file */
    use_extern_tls();
    
    /* Simple validation */
    int sum = tls_used + tls_hidden + tls_default + tls_weak + 
              tls_common + tls_static + tls_dllimport;
    
    return sum > 0 ? 0 : 1;
}

/* Additional TLS variable in file scope for more coverage */
__thread int tls_additional = 777;

/* Weak alias test */
#ifdef HAVE_WEAK
extern __thread int tls_weak_alias __attribute__((weak, alias("tls_weak")));
#endif
