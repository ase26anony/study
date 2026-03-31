/* Test for TLS emulation attribute copying - Main file */
#include <stddef.h>

/* Prevent optimization of unused variables */
#define KEEP(var) asm volatile("" : : "r"(&var))

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

/* Test 1: Public TLS with default visibility and used attribute
   Tests: DECL_PRESERVE_P, TREE_PUBLIC, TREE_USED */
#if HAVE_USED
__attribute__((used))
#endif
__thread int tls_public_used = 42;

/* Test 2: Weak TLS variable
   Tests: DECL_WEAK, TREE_PUBLIC */
#if HAVE_WEAK
__attribute__((weak))
#endif
__thread int tls_weak = 100;

/* Test 3: Hidden visibility TLS
   Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED, TREE_PUBLIC */
#if HAVE_VISIBILITY
__attribute__((visibility("hidden")))
#endif
__thread int tls_hidden = 200;

/* Test 4: Protected visibility TLS
   Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
#if HAVE_VISIBILITY
__attribute__((visibility("protected")))
#endif
__thread int tls_protected = 300;

/* Test 5: Static TLS (non-public) inside function scope
   Tests: DECL_CONTEXT (when inside function), !TREE_PUBLIC */
static void func_with_tls(void) {
    static __thread int tls_in_function = 500;
    KEEP(tls_in_function);
}

/* Test 6: External declaration (defined in aux file)
   Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 7: Common TLS (tentative definition)
   Tests: DECL_COMMON, TREE_PUBLIC */
__thread int tls_common;

/* Test 8: DLL Import style (Windows-like)
   Tests: DECL_DLLIMPORT_P */
#if HAVE_DLLIMPORT
__declspec(dllimport) __thread int tls_dllimport;
#elif HAVE_VISIBILITY
/* Simulate with visibility on non-Windows */
__attribute__((visibility("default"))) __thread int tls_dllimport;
#else
__thread int tls_dllimport;
#endif

/* Test 9: Internal visibility
   Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
#if HAVE_VISIBILITY
__attribute__((visibility("internal")))
#endif
__thread int tls_internal = 600;

/* ========== FUNCTION DECLARATIONS ========== */
void use_tls_variables(void);
void* get_tls_addresses(void);

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Force all TLS variables to be processed */
    
    /* Test 1: Public used TLS */
    tls_public_used += 1;
    KEEP(tls_public_used);
    
    /* Test 2: Weak TLS */
    tls_weak = tls_public_used + 10;
    KEEP(tls_weak);
    
    /* Test 3: Hidden TLS */
    tls_hidden *= 2;
    KEEP(tls_hidden);
    
    /* Test 4: Protected TLS */
    tls_protected -= 50;
    KEEP(tls_protected);
    
    /* Test 5: Call function with static TLS */
    func_with_tls();
    
    /* Test 6: External TLS (defined in aux) */
    tls_external = 999;
    KEEP(tls_external);
    
    /* Test 7: Common TLS */
    tls_common = 777;
    KEEP(tls_common);
    
    /* Test 8: DLL Import style */
    tls_dllimport = 888;
    KEEP(tls_dllimport);
    
    /* Test 9: Internal TLS */
    tls_internal = 111;
    KEEP(tls_internal);
    
    /* Use variables from auxiliary file */
    use_tls_variables();
    
    /* Get addresses to ensure TLS is accessed */
    void* addrs = get_tls_addresses();
    KEEP(addrs);
    
    return 0;
}

/* Force DECL_CONTEXT for some variables by placing in struct */
struct Container {
    /* This creates a DECL_CONTEXT for the TLS variable */
    static __thread int tls_in_struct;
};

/* Definition of the struct member TLS */
__thread int Container::tls_in_struct = 1234;
