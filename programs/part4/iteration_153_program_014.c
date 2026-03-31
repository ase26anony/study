/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'X';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
#else
/* Simulate with weak external on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif
__thread int tls_imported = 5000;

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 300;
static __thread float tls_static_uninit;

/* Helper function to force address usage */
__attribute__((noinline, used))
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    /* Dummy writes to prevent optimization */
    volatile static int sink;
    sink = (int)((ptrdiff_t)addr1 ^ (ptrdiff_t)addr2 ^ 
                 (ptrdiff_t)addr3 ^ (ptrdiff_t)addr4 ^
                 (ptrdiff_t)addr5 ^ (ptrdiff_t)addr6);
}

/* Another helper to ensure TLS variables are used */
__attribute__((noinline, used))
static int compute_with_tls(int arg) {
    int result = 0;
    
    /* Use all TLS variables in computations */
    result += tls_static_init * 2;
    result += tls_extern;
    result += tls_weak / 2;
    result += tls_hidden % 100;
    result += tls_protected >> 2;
    result += tls_imported & 0xFF;
    result += tls_volatile;
    result += (int)tls_static_uninit;
    
    /* Create control flow dependencies */
    if (arg > 0) {
        tls_static_init = arg;
        tls_extern = (char)(arg % 256);
    } else {
        tls_weak = -arg;
        tls_hidden = arg * 10;
    }
    
    return result;
}

/* Function to test external linkage */
extern __thread int external_tls_var;
__thread int external_tls_var = 999;

/* Tentative definition to test DECL_COMMON */
__thread int tls_tentative;

int main(int argc, char **argv) {
    int sum = 0;
    
    /* Initialize some TLS variables based on argc */
    tls_static_init = argc * 10;
    tls_extern = (char)('A' + (argc % 26));
    tls_weak = argc * 100;
    tls_hidden = argc * 1000L;
    tls_protected = argc * 2000L;
    tls_imported = argc * 5000;
    tls_volatile = argc * 300;
    tls_static_uninit = (float)argc / 2.0f;
    external_tls_var = argc * 999;
    tls_tentative = argc * 777;
    
    /* Use TLS variables in computations */
    for (int i = 0; i < argc; i++) {
        sum += compute_with_tls(i);
        
        /* Modify TLS variables in loop */
        tls_static_init += i;
        tls_volatile -= i;
        
        /* Conditional TLS access */
        if (i % 2 == 0) {
            tls_hidden += i;
        } else {
            tls_protected -= i;
        }
    }
    
    /* Take addresses of all TLS variables */
    use_tls_addresses(&tls_static_init,
                     &tls_extern,
                     &tls_weak,
                     &tls_hidden,
                     &tls_protected,
                     &tls_imported);
    
    /* Take addresses of remaining TLS variables */
    use_tls_addresses(&tls_volatile,
                     &tls_static_uninit,
                     &external_tls_var,
                     &tls_tentative,
                     (void*)0,
                     (void*)0);
    
    /* Final computation using all TLS variables */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += (int)(tls_hidden % 10000);
    sum += (int)(tls_protected % 10000);
    sum += tls_imported;
    sum += tls_volatile;
    sum += (int)tls_static_uninit;
    sum += external_tls_var;
    sum += tls_tentative;
    
    /* Prevent dead code elimination */
    printf("Result: %d (argc=%d)\n", sum, argc);
    
    return sum == 0 ? 0 : 1;
}

/* Additional file-like separation using weak symbols */
__attribute__((weak))
__thread int another_weak_tls = 8888;

/* Function in different "compilation unit" simulated via noinline */
__attribute__((noinline, used))
static void use_weak_tls(void) {
    another_weak_tls = 1234;
    volatile int sink = another_weak_tls;
    (void)sink;
}
