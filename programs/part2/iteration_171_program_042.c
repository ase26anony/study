/* Test for TLS emulation attribute copying coverage */
#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Attribute compatibility checks */
#ifdef __GNUC__
#define WEAK_ATTR __attribute__((weak))
#define USED_ATTR __attribute__((used))
#define VISIBILITY_DEFAULT __attribute__((visibility("default")))
#define VISIBILITY_HIDDEN __attribute__((visibility("hidden")))
#define VISIBILITY_PROTECTED __attribute__((visibility("protected")))
#define ALIAS_ATTR(name) __attribute__((alias(#name)))
#else
#define WEAK_ATTR
#define USED_ATTR
#define VISIBILITY_DEFAULT
#define VISIBILITY_HIDDEN
#define VISIBILITY_PROTECTED
#define ALIAS_ATTR(name)
#endif

#ifdef _WIN32
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLIMPORT
#endif

/* Test 1: Basic TLS with used attribute - tests DECL_PRESERVE_P, TREE_USED */
__thread int tls_used_var USED_ATTR = 42;

/* Test 2: Public TLS with default visibility - tests TREE_PUBLIC, DECL_VISIBILITY */
__thread int tls_public VISIBILITY_DEFAULT = 100;

/* Test 3: Weak TLS variable - tests DECL_WEAK */
__thread int tls_weak_var WEAK_ATTR = 200;

/* Test 4: Hidden visibility TLS - tests DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden VISIBILITY_HIDDEN = 300;

/* Test 5: Protected visibility TLS */
__thread int tls_protected VISIBILITY_PROTECTED = 400;

/* Test 6: External declaration (will be defined in another file) - tests DECL_EXTERNAL */
extern __thread int tls_external;

/* Test 7: Common TLS (tentative definition) - tests DECL_COMMON */
__thread int tls_common;

/* Test 8: Static TLS inside function - tests DECL_CONTEXT */
static void func_with_tls(void) {
    static __thread int tls_in_func = 500;
    KEEP_ALIVE(tls_in_func);
}

/* Test 9: DLL import style (simulated with weak) */
#ifdef _WIN32
DLLIMPORT __thread int tls_dllimport;
#else
__thread int tls_dllimport WEAK_ATTR;
#endif

/* Weak alias to test alias handling */
extern __thread int tls_aliased_target;
__thread int tls_aliased WEAK_ATTR ALIAS_ATTR(tls_aliased_target);

/* Global array to take addresses */
void* tls_addresses[10];

int main(void) {
    /* Take addresses to ensure variables are referenced */
    tls_addresses[0] = &tls_used_var;
    tls_addresses[1] = &tls_public;
    tls_addresses[2] = &tls_weak_var;
    tls_addresses[3] = &tls_hidden;
    tls_addresses[4] = &tls_protected;
    tls_addresses[5] = &tls_external;
    tls_addresses[6] = &tls_common;
    tls_addresses[7] = &tls_dllimport;
    tls_addresses[8] = &tls_aliased;
    
    /* Modify some variables */
    tls_used_var += 1;
    tls_public *= 2;
    tls_common = 999;
    
    /* Call function with static TLS */
    func_with_tls();
    
    /* Reference external TLS (defined in aux file) */
    tls_external = 1234;
    
    /* Prevent optimization */
    for (int i = 0; i < 9; i++) {
        KEEP_ALIVE(tls_addresses[i]);
    }
    
    return 0;
}
