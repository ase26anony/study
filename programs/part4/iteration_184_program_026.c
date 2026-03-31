/* This should trigger emulated TLS code generation */
/* Test case for tree-emutls.cc attribute copying (lines 295-304) */

#include <stdio.h>

/* Force TLS emulation with various attributes */

/* 1. Default external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static (internal linkage) TLS */
static __thread int tls_static = 2;

/* 3. External declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* 6. TLS with default visibility and used attribute */
__attribute__((visibility("default"), used)) __thread int tls_visible_used;

/* 7. DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use a different attribute to test the path */
__attribute__((weak, visibility("protected"))) __thread int tls_dllimport;
#endif

/* 8. Common TLS (uninitialized external) */
__thread int tls_common;

/* 9. Weak external TLS */
extern __thread int tls_weak_extern __attribute__((weak));

/* Definition of the extern declaration */
__thread int tls_extern = 3;

/* Definition of weak extern */
__thread int tls_weak_extern __attribute__((weak)) = 4;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_static = tls_default * 10;
    tls_hidden = tls_static + 5;
    
    if (tls_weak == 0) {
        tls_weak = 100;
    }
    
    tls_visible_used = tls_hidden + tls_weak;
    
    /* Take address to force symbol usage */
    int *addr = &tls_static;
    *addr += 1;  /* Use the pointer */
}

/* Another helper that takes TLS address as parameter */
static void use_tls_pointer(int *ptr) {
    if (ptr) {
        *ptr += 1;
    }
}

int main(void) {
    int sum = 0;
    
    /* Initialize some TLS variables */
    tls_hidden = 10;
    tls_common = 20;
    tls_visible_used = 30;
    
    /* Use all TLS variables to prevent optimization */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible_used;
    sum += tls_common;
    sum += tls_weak_extern;
    
    printf("Initial sum: %d\n", sum);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum after modification */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_visible_used + tls_common + tls_weak_extern;
    
    printf("Modified sum: %d\n", sum);
    
    /* Take address of TLS variables to ensure they're fully processed */
    int *ptrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_visible_used,
        &tls_common,
        &tls_weak_extern
    };
    
    /* Use pointers to create side effects */
    for (int i = 0; i < 8; i++) {
        use_tls_pointer(ptrs[i]);
    }
    
    /* Final calculation */
    sum = tls_default + tls_static + tls_extern + tls_weak + 
          tls_hidden + tls_visible_used + tls_common + tls_weak_extern;
    
    printf("Final sum: %d\n", sum);
    
    return 0;
}
