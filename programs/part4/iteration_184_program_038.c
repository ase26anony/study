/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS emulation attribute copying logic */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

#include <stdio.h>

/* Force emulated TLS by using attributes that require special handling */

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage */
static __thread int tls_static = 2;

/* 3. External declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol - should set DECL_WEAK */
__attribute__((weak)) __thread int tls_weak = 4;

/* 5. TLS with hidden visibility - should set DECL_VISIBILITY */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* 6. TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible = 6;

/* 7. Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* 8. Uninitialized TLS */
__thread int tls_uninit;

/* 9. DLL import simulation (for DECL_DLLIMPORT_P) */
/* Note: This typically requires Windows target, but we include it for completeness */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_imported;
#else
/* On non-Windows, we'll just declare it normally */
__thread int tls_imported = 9;
#endif

/* 10. External definition (matches earlier declaration) */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden -= 1;
    tls_visible = tls_default + tls_static;
    
    /* Ensure weak symbol is used */
    if (tls_weak) {
        tls_weak++;
    }
    
    /* Use the uninitialized TLS */
    tls_uninit = 100;
    
    /* Use the imported TLS */
    tls_imported = 999;
}

/* Another helper that takes addresses of TLS variables */
void use_tls_pointers(void) {
    /* Taking addresses forces symbol references */
    int *p1 = &tls_default;
    int *p2 = &tls_static;
    int *p3 = &tls_hidden;
    int *p4 = &tls_visible;
    int *p5 = &tls_weak;
    int *p6 = &tls_used;
    int *p7 = &tls_uninit;
    int *p8 = &tls_extern;
    int *p9 = &tls_imported;
    
    /* Use pointers to create side effects */
    *p1 += 1;
    *p2 += 1;
    
    /* Prevent optimization of pointers */
    if (p3 == p4) {
        /* This should never happen, but prevents dead code elimination */
        printf("Unexpected pointer equality\n");
    }
}

int main(void) {
    int sum = 0;
    
    /* Initial use of TLS variables */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible;
    sum += tls_used;
    sum += tls_imported;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_visible + tls_used + tls_imported + tls_uninit;
    
    printf("After modify_tls sum: %d\n", sum);
    
    /* Use pointers to TLS variables */
    use_tls_pointers();
    
    /* Final calculation and output */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_visible + tls_used + tls_imported + tls_uninit;
    
    printf("Final sum: %d\n", sum);
    
    /* Additional operations to ensure all TLS variables are used */
    if (tls_uninit > 0) {
        tls_used = tls_uninit;
    }
    
    return sum > 100 ? 0 : 1;
}
