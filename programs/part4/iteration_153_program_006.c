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

/* Pattern E: DLL import simulation (for MinGW targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported __attribute__((visibility("default"))) = 5000;
#endif

/* Pattern F: Common TLS variable (tentative definition) */
__thread double tls_common;

/* Pattern G: Thread-local with preserve attribute */
__thread volatile int tls_preserve __attribute__((used));

/* Helper function to ensure addresses are taken and used */
__attribute__((noinline)) 
static void use_tls_pointers(void *ptr1, void *ptr2, void *ptr3, 
                             void *ptr4, void *ptr5, void *ptr6, void *ptr7) {
    /* Dummy writes to prevent optimization */
    volatile static int sink = 0;
    sink += (intptr_t)ptr1 + (intptr_t)ptr2 + (intptr_t)ptr3 + 
            (intptr_t)ptr4 + (intptr_t)ptr5 + (intptr_t)ptr6 + (intptr_t)ptr7;
}

/* Another helper that uses TLS values */
__attribute__((noinline))
static int compute_with_tls(int arg) {
    /* Force reads and writes to all TLS variables */
    tls_static_init += arg;
    tls_extern = (char)((tls_extern + arg) % 128);
    tls_weak ^= arg;
    tls_hidden += tls_weak;
    tls_protected -= arg;
    tls_imported *= (arg + 1);
    tls_common = (double)arg / 3.14159;
    tls_preserve = arg * 2;
    
    /* Complex computation that depends on all TLS variables */
    return tls_static_init + tls_extern + tls_weak + 
           (int)tls_hidden + (int)tls_protected + tls_imported + 
           (int)tls_common + tls_preserve;
}

/* Function in another compilation unit simulation */
__attribute__((weak))
void external_tls_user(void) {
    /* Access TLS variables that might be defined elsewhere */
    extern __thread int external_tls_var;
    if (&external_tls_var) {
        external_tls_var = 999;
    }
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Ensure all TLS variables are used */
    if (argc > 1) {
        /* Pattern A usage */
        tls_static_init = argc * 10;
        
        /* Pattern B usage */
        tls_extern = (char)('A' + (argc % 26));
        
        /* Pattern C usage - only if strong definition exists */
        if (&tls_weak) {
            tls_weak = argc * 100;
        }
        
        /* Pattern D usage */
        tls_hidden = argc * 1000L;
        tls_protected = argc * 2000L;
        
        /* Pattern E usage */
        tls_imported = argc * 5000;
        
        /* Pattern F usage */
        tls_common = (double)argc / 2.71828;
        
        /* Pattern G usage */
        tls_preserve = argc * 3;
    }
    
    /* Take addresses of all TLS variables to force emulation */
    use_tls_pointers(&tls_static_init, &tls_extern, &tls_weak,
                     &tls_hidden, &tls_protected, &tls_imported,
                     &tls_common);
    
    /* Complex computation using TLS variables */
    for (int i = 0; i < argc; i++) {
        result += compute_with_tls(i);
    }
    
    /* Use weak external function */
    external_tls_user();
    
    /* Prevent dead code elimination */
    volatile int output = result;
    
    printf("Result: %d (TLS values: %d, %c, %d, %ld, %ld, %d, %f, %d)\n",
           output, tls_static_init, tls_extern, tls_weak,
           tls_hidden, tls_protected, tls_imported,
           tls_common, tls_preserve);
    
    return output != 0;
}
