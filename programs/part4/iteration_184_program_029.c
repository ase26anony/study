/* Test case specifically designed to trigger TLS emulation attribute copying
   in tree-emutls.cc lines 295-304. This should force the compiler to copy
   DECL_WEAK, DECL_VISIBILITY, DECL_DLLIMPORT_P, and other attributes from
   original TLS declarations to emulated TLS structures. */

/* Force emulated TLS handling - either compile with -femulated-tls or
   target an architecture without native TLS support (e.g., -march=armv7-a) */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Helper function to use TLS variables */
void modify_tls(void);

/* Dummy function to use TLS variable addresses */
void use_pointer(int *p);

/* ===== TLS VARIABLES WITH VARIOUS ATTRIBUTES ===== */

/* 1. Default external linkage TLS with initialization */
__thread int tls_default = 1;

/* 2. Static TLS (internal linkage) */
static __thread int tls_static = 2;

/* 3. External TLS declaration (simulating header declaration) */
extern __thread int tls_extern;

/* 4. External TLS definition */
__thread int tls_extern = 3;

/* 5. Weak TLS symbol - may be overridden */
__attribute__((weak)) __thread int tls_weak;

/* 6. Weak TLS with initialization */
__attribute__((weak)) __thread int tls_weak_init = 5;

/* 7. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* 8. TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_visible = 7;

/* 9. TLS marked as used to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used_attr = 8;

/* 10. TLS with dllimport attribute (for Windows-like targets) */
/* Note: This typically requires proper dllimport/dllexport setup,
   but we include it to potentially trigger DECL_DLLIMPORT_P */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we'll just declare it normally */
__thread int tls_dllimport;
#endif

/* 11. Uninitialized TLS (common symbol behavior) */
__thread int tls_common;

/* 12. Static TLS with hidden visibility */
static __attribute__((visibility("hidden"))) __thread int tls_static_hidden;

/* ===== HELPER FUNCTION DEFINITIONS ===== */

void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    if (tls_weak == 0) {  /* Weak symbol may be uninitialized */
        tls_weak = 100;
    }
    
    tls_hidden = tls_default + tls_static;
    tls_visible++;
    tls_used_attr = 999;
    
    /* Take address of TLS variable to inhibit optimizations */
    int *ptr = &tls_hidden;
    use_pointer(ptr);
}

void use_pointer(int *p) {
    /* Dummy function to use TLS pointer */
    if (p) {
        *p += 1;
    }
}

/* ===== MAIN FUNCTION ===== */

int main(void) {
    int sum = 0;
    
    /* Initialize some TLS variables */
    tls_common = 42;
    tls_hidden = 9;
    tls_static_hidden = 11;
    
    /* Use all TLS variables to ensure they're not optimized away */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_weak_init;
    sum += tls_hidden;
    sum += tls_visible;
    sum += tls_used_attr;
    sum += tls_dllimport;
    sum += tls_common;
    sum += tls_static_hidden;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum after modification */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_weak_init + tls_hidden + tls_visible + tls_used_attr +
          tls_dllimport + tls_common + tls_static_hidden;
    
    printf("Modified sum: %d\n", sum);
    
    /* Take address of TLS variable to create side effect */
    int *addr = &tls_default;
    use_pointer(addr);
    
    /* Use address in conditional */
    if (addr != NULL) {
        tls_default = *addr + 1;
    }
    
    printf("Final tls_default: %d\n", tls_default);
    
    return 0;
}
