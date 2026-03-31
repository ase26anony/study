/* Test program for TLS emulation attribute copying coverage */
/* This tests the copy_decl_attributes function in tree-emutls.cc */

#include <stddef.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(&(var)))

/* Attribute compatibility macros */
#if defined(__GNUC__) || defined(__clang__)
#  define ATTR_WEAK __attribute__((weak))
#  define ATTR_USED __attribute__((used))
#  define ATTR_VIS_HIDDEN __attribute__((visibility("hidden")))
#  define ATTR_VIS_DEFAULT __attribute__((visibility("default")))
#  define ATTR_VIS_PROTECTED __attribute__((visibility("protected")))
#  define ATTR_ALIAS(target) __attribute__((alias(#target)))
#else
#  define ATTR_WEAK
#  define ATTR_USED
#  define ATTR_VIS_HIDDEN
#  define ATTR_VIS_DEFAULT
#  define ATTR_VIS_PROTECTED
#  define ATTR_ALIAS(target)
#endif

#ifdef _WIN32
#  define DLL_IMPORT __declspec(dllimport)
#else
#  define DLL_IMPORT
#endif

/* Test 1: TLS variable with explicit initialization and default visibility
   Tests: DECL_PRESERVE_P (via used), TREE_USED, TREE_PUBLIC */
__thread int tls_defined = 42;
ATTR_USED __thread int tls_used = 100;

/* Test 2: TLS variable with hidden visibility
   Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
ATTR_VIS_HIDDEN __thread int tls_hidden;

/* Test 3: TLS variable with protected visibility
   Tests: DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
ATTR_VIS_PROTECTED __thread int tls_protected;

/* Test 4: Weak TLS variable
   Tests: DECL_WEAK */
ATTR_WEAK __thread int tls_weak;

/* Test 5: External TLS declaration (defined in another file)
   Tests: DECL_EXTERNAL, TREE_PUBLIC */
extern __thread int tls_external;

/* Test 6: Common TLS variable (tentative definition)
   Tests: DECL_COMMON */
__thread int tls_common;

/* Test 7: Static TLS variable inside a function (non-NULL DECL_CONTEXT)
   Tests: DECL_CONTEXT */
static void func_with_tls(void) {
    static __thread int tls_in_func = 99;
    KEEP_ALIVE(tls_in_func);
}

/* Test 8: DLL imported TLS variable (simulated for non-Windows)
   Tests: DECL_DLLIMPORT_P */
#ifdef _WIN32
DLL_IMPORT extern __thread int tls_imported;
#else
/* Simulate with visibility default */
ATTR_VIS_DEFAULT extern __thread int tls_imported;
#endif

/* Forward declaration for function in auxiliary file */
void use_tls_variables(void);

int main(void) {
    /* Force all TLS variables to be processed */
    
    /* Test 1: Defined TLS */
    tls_defined += 1;
    KEEP_ALIVE(tls_defined);
    
    /* Test 1b: Used TLS */
    tls_used *= 2;
    KEEP_ALIVE(tls_used);
    
    /* Test 2: Hidden visibility */
    tls_hidden = 1;
    KEEP_ALIVE(tls_hidden);
    
    /* Test 3: Protected visibility */
    tls_protected = 2;
    KEEP_ALIVE(tls_protected);
    
    /* Test 4: Weak linkage */
    tls_weak = 3;
    KEEP_ALIVE(tls_weak);
    
    /* Test 5: External reference */
    tls_external = 4;
    KEEP_ALIVE(tls_external);
    
    /* Test 6: Common variable */
    tls_common = 5;
    KEEP_ALIVE(tls_common);
    
    /* Test 7: Function-scoped static TLS */
    func_with_tls();
    
    /* Test 8: DLL imported */
    tls_imported = 6;
    KEEP_ALIVE(tls_imported);
    
    /* Use variables from auxiliary file */
    use_tls_variables();
    
    return 0;
}

/* Force DECL_PRESERVE_P for some variables via inline assembly */
__asm__(
    ".globl tls_defined\n"
    ".globl tls_used\n"
);
