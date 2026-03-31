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
/* Simulate similar attribute on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif
__thread int tls_imported = 5000;

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 99;
static __thread float tls_static_uninit;

/* Helper function to ensure addresses are taken and used */
__attribute__((noinline, noipa))
static void use_tls_pointers(void *p1, void *p2, void *p3, void *p4, void *p5, 
                             void *p6, void *p7, void *p8) {
    /* Dummy writes to prevent optimization */
    volatile static int sink;
    sink = (int)((intptr_t)p1 ^ (intptr_t)p2 ^ (intptr_t)p3 ^ 
                 (intptr_t)p4 ^ (intptr_t)p5 ^ (intptr_t)p6 ^ 
                 (intptr_t)p7 ^ (intptr_t)p8);
}

/* Another helper to force variable usage */
__attribute__((noinline, noipa))
static int compute_with_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computations */
    tls_static_init = argc * 2;
    result += tls_static_init;
    
    tls_extern = 'A' + (argc % 26);
    result += tls_extern;
    
    if (tls_weak != 0) {
        tls_weak = argc * 3;
        result += tls_weak;
    }
    
    tls_hidden = argc * 4L;
    result += (int)tls_hidden;
    
    tls_protected = argc * 5L;
    result += (int)tls_protected;
    
    tls_imported = argc * 6;
    result += tls_imported;
    
    tls_volatile = argc * 7;
    result += tls_volatile;
    
    tls_static_uninit = argc * 8.0f;
    result += (int)tls_static_uninit;
    
    return result;
}

/* Function to test external linkage simulation */
__attribute__((noinline))
static void test_external_linkage(void) {
    /* Tentative definition to test DECL_COMMON */
    __thread int tls_tentative;
    tls_tentative = 1234;
    
    /* Another external reference */
    extern __thread double tls_external_ref;
    static __thread double tls_external_ref = 3.14159;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Ensure all TLS variables are marked as used */
    if (argc > 1) {
        /* Complex usage pattern to prevent optimization */
        for (int i = 0; i < argc; i++) {
            result += compute_with_tls(i);
        }
    } else {
        result = compute_with_tls(argc);
    }
    
    /* Take addresses of all TLS variables - crucial for emulation */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_volatile,
        &tls_static_uninit
    );
    
    /* Test external linkage patterns */
    test_external_linkage();
    
    /* Create control flow dependencies on TLS values */
    volatile int output = 0;
    if (tls_static_init > 0) {
        output += tls_extern;
    }
    if (tls_hidden != 0) {
        output += tls_protected;
    }
    
    /* Final result depends on all TLS variables */
    result += output;
    
    /* Prevent dead code elimination */
    printf("Result: %d (TLS values: %d, %c, %d, %ld, %ld, %d, %d, %.2f)\n",
           result,
           tls_static_init,
           tls_extern,
           tls_weak,
           tls_hidden,
           tls_protected,
           tls_imported,
           tls_volatile,
           tls_static_uninit);
    
    return result != 0 ? 0 : 1;
}

/* Additional file-scope TLS to test more cases */
__thread uint64_t tls_file_scope = 0xDEADBEEF;
static __thread int tls_static_file_scope = 999;

/* Weak external reference without definition to test DECL_EXTERNAL */
extern __thread int tls_pure_extern __attribute__((weak));
