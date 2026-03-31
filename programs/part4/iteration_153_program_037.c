/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported = 5000;
#endif

/* Additional TLS variables with different types and attributes */
__thread volatile double tls_volatile = 3.14159;
__thread const char* tls_const_ptr = "TLS_STRING";

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_address(void* addr, int idx) {
    /* Dummy operation to force address materialization */
    volatile int* p = (volatile int*)addr;
    (void)p;  /* Prevent unused variable warning */
    /* This ensures the address is needed for emulation */
}

/* Another helper to create control flow dependencies */
__attribute__((noinline))
static int compute_checksum(int val) {
    return val ^ 0x55AA55AA;
}

int main(int argc, char** argv) {
    int result = 0;
    
    /* 1. Use all TLS variables to ensure they're not optimized away */
    
    /* Pattern A usage */
    tls_static_init = argc * 10;
    result += tls_static_init;
    use_tls_address(&tls_static_init, 1);
    
    /* Pattern B usage */
    tls_extern = 'A' + (argc % 26);
    result += tls_extern;
    use_tls_address(&tls_extern, 2);
    
    /* Pattern C usage - conditional based on weak symbol availability */
    if (&tls_weak != NULL) {
        tls_weak = argc * 20;
        result += tls_weak;
        use_tls_address(&tls_weak, 3);
    }
    
    /* Pattern D usage */
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    result += (int)(tls_hidden + tls_protected);
    use_tls_address(&tls_hidden, 4);
    use_tls_address(&tls_protected, 5);
    
    /* Pattern E usage */
    tls_imported = argc * 50;
    result += tls_imported;
    use_tls_address(&tls_imported, 6);
    
    /* Additional TLS variable usage */
    tls_volatile = argc * 3.14159;
    result += (int)tls_volatile;
    use_tls_address((void*)&tls_volatile, 7);
    
    /* Use const pointer TLS variable */
    result += (int)(uintptr_t)tls_const_ptr;
    use_tls_address((void*)&tls_const_ptr, 8);
    
    /* 2. Create complex control flow that depends on TLS values */
    for (int i = 0; i < argc; i++) {
        /* Loop that depends on argc and TLS variables */
        tls_static_init += i;
        if (i % 2 == 0) {
            tls_extern = (tls_extern + 1) & 0xFF;
        }
    }
    
    /* 3. Compute final result using all TLS variables */
    result = compute_checksum(result);
    
    /* 4. Return value that depends on all operations */
    return result % 256;
}

/* Additional file-like separation using weak symbols */
/* This creates a "tentative definition" scenario */
__thread int tls_tentative;

/* Force multiple compilation units scenario in single file */
static void dummy_function(void) {
    /* Reference tentative TLS variable */
    tls_tentative = 99;
    
    /* Create reference to external-like TLS */
    extern __thread int tls_extern_ref;
    static __thread int tls_local_static = 1234;
    
    tls_extern_ref = tls_local_static;
}

/* Definition of external reference */
__thread int tls_extern_ref = 0;
