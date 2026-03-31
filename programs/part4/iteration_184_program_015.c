/* This should trigger emulated TLS code generation */
/* Test case designed to exercise TLS attribute copying in tree-emutls.cc */

#include <stdio.h>

/* Force emulated TLS by using appropriate compilation flags */
/* Compile with: -O0 -femulated-tls -fvisibility=hidden -fPIC */

/* 1. Plain TLS with default visibility and used attribute */
__attribute__((used)) __thread int tls_default = 1;

/* 2. Static TLS (internal linkage) */
static __thread int tls_static = 2;

/* 3. External TLS declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* 6. TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_default_vis = 6;

/* 7. Uninitialized TLS */
__thread int tls_uninit;

/* 8. DLL import simulation (for DECL_DLLIMPORT_P) */
/* Note: This typically requires Windows target, but we include it for completeness */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Simulate with a weak imported attribute if not Windows */
__attribute__((weak)) extern __thread int tls_dllimport;
#endif

/* 9. Common TLS (uninitialized with external linkage) */
__thread int tls_common;

/* Definition of the extern TLS variable */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    
    /* Use the weak TLS variable */
    if (&tls_weak) {  /* Ensure weak symbol is referenced */
        tls_weak = 100;
    }
    
    /* Use hidden visibility TLS */
    tls_hidden = tls_default + tls_static;
    
    /* Use the extern TLS */
    tls_extern--;
    
    /* Initialize uninitialized TLS */
    tls_uninit = 42;
    
    /* Use common TLS */
    tls_common = 99;
}

/* Another helper that takes addresses of TLS variables */
void take_addresses(void) {
    /* Taking addresses forces the compiler to fully process TLS symbols */
    int *p1 = &tls_default;
    int *p2 = &tls_static;
    int *p3 = &tls_extern;
    int *p4 = &tls_hidden;
    int *p5 = &tls_default_vis;
    
    /* Use pointers to create side effects */
    if (p1 && p2 && p3 && p4 && p5) {
        *p1 += 1;
        *p5 = *p1 + *p2;
    }
    
    /* Reference weak symbol by address */
    if (&tls_weak) {
        volatile int *weak_ptr = &tls_weak;
        (void)weak_ptr;  /* Prevent unused variable warning */
    }
}

int main(void) {
    int sum = 0;
    
    /* Initialize some variables */
    tls_hidden = 5;
    tls_weak = 7;  /* This may be overridden if strong definition exists elsewhere */
    tls_common = 8;
    
    /* Use all TLS variables in main */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_default_vis;
    
    /* Call helper functions */
    modify_tls();
    take_addresses();
    
    /* Recalculate sum after modifications */
    sum = tls_default + tls_static + tls_extern + tls_hidden + tls_default_vis;
    
    /* Use uninitialized TLS */
    sum += tls_uninit;
    
    /* Use common TLS */
    sum += tls_common;
    
    /* Print result to prevent optimization */
    printf("TLS sum: %d\n", sum);
    
    /* Additional printf to use more variables */
    printf("Values: default=%d, static=%d, extern=%d, hidden=%d\n",
           tls_default, tls_static, tls_extern, tls_hidden);
    
    return 0;
}
