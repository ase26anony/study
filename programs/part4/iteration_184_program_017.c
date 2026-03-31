/* test-emutls-attributes.c */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Force emulated TLS by targeting architectures without native TLS support
   or using -femulated-tls flag */

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage, initialized */
static __thread int tls_static = 2;

/* 3. Extern declaration (simulating header) */
extern __thread int tls_extern_decl;

/* 4. Weak TLS symbol - may be overridden */
__attribute__((weak)) __thread int tls_weak = 4;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 5;

/* 6. TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_default_vis = 6;

/* 7. Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 7;

/* 8. Uninitialized TLS */
__thread int tls_uninit;

/* 9. Common TLS (uninitialized external) - triggers DECL_COMMON */
__thread int tls_common;

/* 10. Definition of extern declaration */
__thread int tls_extern_decl = 3;

/* 11. DLL import simulation (for Windows targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* For non-Windows, use another attribute combination */
__attribute__((weak, visibility("hidden"))) __thread int tls_combo = 11;
#endif

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_static += tls_default;
    tls_hidden = tls_weak * 2;
    tls_used = tls_default_vis - 1;
    
    /* Use uninitialized TLS */
    tls_uninit = 100;
    tls_common = 200;
    
    /* Take address to inhibit optimizations */
    int *ptr1 = &tls_static;
    int *ptr2 = &tls_hidden;
    
    /* Create side effects */
    if (ptr1 != ptr2) {
        *ptr1 += 1;
    }
    
#ifdef _WIN32
    if (&tls_dllimport != NULL) {
        /* Access dllimport TLS */
        tls_dllimport = 999;
    }
#else
    tls_combo = tls_weak + tls_hidden;
#endif
}

/* Another function taking TLS addresses */
void use_tls_pointers(void) {
    /* Take addresses of multiple TLS variables */
    int *ptrs[] = {
        &tls_default,
        &tls_static,
        &tls_extern_decl,
        &tls_weak,
        &tls_hidden,
        &tls_default_vis,
        &tls_used,
        &tls_uninit,
        &tls_common,
#ifndef _WIN32
        &tls_combo,
#endif
        NULL
    };
    
    /* Use pointers to create observable behavior */
    int sum = 0;
    for (int i = 0; ptrs[i] != NULL; i++) {
        sum += *ptrs[i];
    }
    
    /* Prevent dead code elimination */
    if (sum > 0) {
        tls_default = sum % 100;
    }
}

int main(void) {
    int result = 0;
    
    /* Initialize uninitialized TLS */
    tls_uninit = 8;
    tls_common = 9;
    
    /* Use all TLS variables in main */
    result += tls_default;
    result += tls_static;
    result += tls_extern_decl;
    result += tls_weak;
    result += tls_hidden;
    result += tls_default_vis;
    result += tls_used;
    result += tls_uninit;
    result += tls_common;
    
#ifndef _WIN32
    result += tls_combo;
#endif
    
    printf("Initial sum: %d\n", result);
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate after modification */
    result = 0;
    result += tls_default;
    result += tls_static;
    result += tls_extern_decl;
    result += tls_weak;
    result += tls_hidden;
    result += tls_default_vis;
    result += tls_used;
    result += tls_uninit;
    result += tls_common;
    
#ifndef _WIN32
    result += tls_combo;
#endif
    
    printf("After modify_tls: %d\n", result);
    
    /* Use pointers to TLS variables */
    use_tls_pointers();
    
    /* Final computation using TLS */
    int final = tls_default + tls_static * 2 + tls_hidden / 2;
    printf("Final computation: %d\n", final);
    
    /* Take address of a TLS variable and use it */
    int *tls_ptr = &tls_default;
    *tls_ptr += final;
    printf("tls_default final value: %d\n", tls_default);
    
    return 0;
}
