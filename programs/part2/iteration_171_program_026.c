/* Test for TLS emulation attribute copying - main file */
#include <stddef.h>

/* Prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(&(var)))

/* Attribute availability checks */
#ifdef __GNUC__
#define HAVE_ATTRIBUTE(x) 1
#else
#define HAVE_ATTRIBUTE(x) 0
#endif

/* Visibility attribute wrapper */
#if HAVE_ATTRIBUTE(visibility)
#define HIDDEN __attribute__((visibility("hidden")))
#define PROTECTED __attribute__((visibility("protected")))
#define INTERNAL __attribute__((visibility("internal")))
#else
#define HIDDEN
#define PROTECTED
#define INTERNAL
#endif

/* Weak attribute wrapper */
#if HAVE_ATTRIBUTE(weak)
#define WEAK __attribute__((weak))
#else
#define WEAK
#endif

/* Used attribute wrapper */
#if HAVE_ATTRIBUTE(used)
#define USED __attribute__((used))
#else
#define USED
#endif

/* DLL import for Windows-like targets */
#ifdef _WIN32
#define DLLIMPORT __declspec(dllimport)
#elif defined(__CYGWIN__) || defined(__MINGW32__)
#define DLLIMPORT __attribute__((dllimport))
#else
#define DLLIMPORT
#endif

/* ========== TLS VARIABLES WITH VARIOUS ATTRIBUTES ========== */

/* Test 1: Basic TLS with default visibility and used attribute
   Tests: DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC */
__thread USED int tls_used_default = 42;

/* Test 2: Hidden visibility TLS
   Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread HIDDEN int tls_hidden = 100;

/* Test 3: Protected visibility with weak linkage
   Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED, DECL_WEAK */
__thread PROTECTED WEAK int tls_protected_weak = 200;

/* Test 4: External declaration (will be defined in aux file)
   Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 5: Common linkage (tentative definition)
   Tests: DECL_COMMON */
__thread int tls_common;

/* Test 6: Static TLS (not public) for contrast
   Tests: TREE_PUBLIC (should be false), DECL_CONTEXT */
static __thread int tls_static = 300;

/* Test 7: DLL import simulation (if supported)
   Tests: DECL_DLLIMPORT_P */
#ifdef DLLIMPORT_SUPPORT
DLLIMPORT __thread int tls_dllimport;
#endif

/* Test 8: Internal visibility
   Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread INTERNAL int tls_internal = 400;

/* Function with local TLS to test DECL_CONTEXT */
static void test_local_tls(void)
{
    /* Local TLS variable - has function as DECL_CONTEXT */
    static __thread int local_tls_in_func = 500;
    KEEP(local_tls_in_func);
    local_tls_in_func++;
}

/* Weak alias target */
__thread int tls_weak_target = 600;

/* Weak alias declaration */
#if HAVE_ATTRIBUTE(weak) && HAVE_ATTRIBUTE(alias)
extern __thread int tls_weak_alias __attribute__((weak, alias("tls_weak_target")));
#endif

/* Function declarations from aux file */
void use_external_tls(void);
void* get_tls_addresses(void);

int main(void)
{
    /* Force all TLS variables to be referenced */
    
    /* Test 1: Used default */
    KEEP(tls_used_default);
    tls_used_default++;
    
    /* Test 2: Hidden */
    KEEP(tls_hidden);
    tls_hidden += 2;
    
    /* Test 3: Protected weak */
    KEEP(tls_protected_weak);
    tls_protected_weak += 3;
    
    /* Test 4: External (defined in aux) */
    KEEP(tls_external);
    use_external_tls();
    
    /* Test 5: Common */
    KEEP(tls_common);
    tls_common = 123;
    
    /* Test 6: Static */
    KEEP(tls_static);
    tls_static++;
    
    /* Test 8: Internal */
    KEEP(tls_internal);
    tls_internal += 5;
    
    /* Test weak alias if available */
#if HAVE_ATTRIBUTE(weak) && HAVE_ATTRIBUTE(alias)
    KEEP(tls_weak_alias);
    tls_weak_alias++;
#endif
    
    /* Test local TLS in function */
    test_local_tls();
    
    /* Get addresses to ensure all TLS vars are processed */
    void* addresses = get_tls_addresses();
    (void)addresses;
    
    /* Simple validation */
    int sum = tls_used_default + tls_hidden + tls_common + tls_static;
    
    return sum > 0 ? 0 : 1;
}
