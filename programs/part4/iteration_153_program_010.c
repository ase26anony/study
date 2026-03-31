/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
#else
/* Simulate with weak linkage on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif
__thread int tls_imported = 5000;

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 99;
static __thread float tls_static_uninit;

/* Helper function to ensure addresses are taken and used */
__attribute__((noinline)) 
static void use_tls_pointers(void *p1, void *p2, void *p3, void *p4, void *p5) {
    /* Dummy writes to prevent optimization */
    volatile static int sink;
    sink = (int)((intptr_t)p1 ^ (intptr_t)p2 ^ (intptr_t)p3 ^ 
                 (intptr_t)p4 ^ (intptr_t)p5);
}

/* Another helper that uses TLS values */
__attribute__((noinline, noipa))
static int compute_with_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computations */
    result += tls_static_init * argc;
    result += tls_extern;
    result += tls_weak / (argc + 1);
    result += tls_hidden % 256;
    result += tls_protected & 0xFF;
    result += tls_imported >> 8;
    result += tls_volatile;
    result += (int)tls_static_uninit;
    
    return result;
}

/* Function to force external linkage reference */
void reference_extern_tls(void) {
    /* Reference to force external linkage handling */
    extern __thread double tls_extern_ref;
    (void)&tls_extern_ref;
}

int main(int argc, char **argv) {
    int sum = 0;
    
    /* Modify TLS variables based on program input */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 100;
    tls_hidden = argc * 1000L;
    tls_protected = argc * 2000L;
    tls_imported = argc * 5000;
    tls_volatile = argc * 99;
    tls_static_uninit = argc * 3.14f;
    
    /* Take addresses of all TLS variables */
    use_tls_pointers(&tls_static_init,
                     &tls_extern,
                     &tls_weak,
                     &tls_hidden,
                     &tls_protected);
    
    use_tls_pointers(&tls_imported,
                     &tls_volatile,
                     &tls_static_uninit,
                     (void*)(intptr_t)argc,
                     (void*)0);
    
    /* Complex computation using TLS values */
    for (int i = 0; i < argc; i++) {
        sum += compute_with_tls(i);
        
        /* Modify TLS inside loop to create dependencies */
        if (i % 2 == 0) {
            tls_static_init += i;
            tls_volatile -= i;
        }
    }
    
    /* Conditional use of TLS variables */
    if (argc > 1) {
        tls_extern = argv[1][0];
        sum += tls_extern * tls_weak;
    }
    
    /* Reference the extern TLS declaration */
    reference_extern_tls();
    
    /* Use sum to prevent dead code elimination */
    printf("Result: %d\n", sum);
    return sum & 0xFF;  /* Return non-constant value */
}

/* External TLS definition (for Pattern B completeness) */
__thread double tls_extern_ref = 3.14159265359;

/* Additional file-like separation using weak symbols */
__attribute__((weak))
__thread int tls_weak_alt = 9999;

/* Force common symbol behavior */
__thread int tls_common;  /* Tentative definition - becomes common */
