/* test_emutls.c - Comprehensive TLS emulation test */
/* Compile with: gcc -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls */

#include <stdio.h>
#include <stdint.h>

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

/* Pattern E: DLL import simulation (for MinGW targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 3000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported = 3000;
#endif

/* Additional TLS variables with different types and qualifiers */
__thread volatile double tls_volatile = 3.14159;
__thread const char* tls_pointer = "TLS String";

/* Helper function to ensure addresses are taken and used */
__attribute__((noinline)) 
static void use_tls_addresses(void* addr1, void* addr2, void* addr3, 
                              void* addr4, void* addr5, void* addr6) {
    /* Dummy operations to prevent optimization */
    volatile int dummy = 0;
    if (addr1) dummy += 1;
    if (addr2) dummy += 2;
    if (addr3) dummy += 3;
    if (addr4) dummy += 4;
    if (addr5) dummy += 5;
    if (addr6) dummy += 6;
    
    /* Prevent unused parameter warnings */
    (void)addr1; (void)addr2; (void)addr3;
    (void)addr4; (void)addr5; (void)addr6;
}

/* Another helper to create control flow dependencies */
__attribute__((noinline))
static int compute_with_tls(int base) {
    int result = base;
    
    /* Use all TLS variables in computations */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += (int)tls_hidden;
    result += (int)tls_protected;
    result += tls_imported;
    result += (int)tls_volatile;
    result += (int)(uintptr_t)tls_pointer;
    
    /* Create conditional behavior */
    if (tls_static_init > 0) {
        result *= 2;
    }
    
    if (tls_weak < 50) {
        result -= 100;
    }
    
    return result;
}

/* Function to modify TLS variables */
__attribute__((noinline))
static void modify_tls_vars(int argc) {
    /* Modify Pattern A */
    tls_static_init = argc * 10;
    
    /* Modify Pattern B */
    tls_extern = 'A' + (argc % 26);
    
    /* Modify Pattern C - only if strong definition exists */
    if (tls_weak != 0) {
        tls_weak = argc * 20;
    }
    
    /* Modify Pattern D */
    tls_hidden = argc * 100L;
    tls_protected = argc * 200L;
    
    /* Modify Pattern E */
    tls_imported = argc * 300;
    
    /* Modify other TLS variables */
    tls_volatile = (double)argc / 2.0;
    tls_pointer = argc > 0 ? "Modified" : "Original";
}

int main(int argc, char** argv) {
    int result = 0;
    
    /* Initial computation */
    result = compute_with_tls(argc);
    
    /* Modify TLS variables based on argc */
    modify_tls_vars(argc);
    
    /* Take addresses of all TLS variables to force emulation */
    use_tls_addresses(
        (void*)&tls_static_init,
        (void*)&tls_extern,
        (void*)&tls_weak,
        (void*)&tls_hidden,
        (void*)&tls_protected,
        (void*)&tls_imported
    );
    
    /* Additional address usage for remaining TLS variables */
    volatile void* addr1 = (void*)&tls_volatile;
    volatile void* addr2 = (void*)&tls_pointer;
    (void)addr1; (void)addr2;
    
    /* Final computation with modified values */
    result += compute_with_tls(argc);
    
    /* Loop to create more complex control flow */
    for (int i = 0; i < argc; i++) {
        tls_static_init += i;
        tls_weak -= i;
        
        if (i % 2 == 0) {
            tls_hidden += i;
        } else {
            tls_protected += i;
        }
    }
    
    /* One more computation */
    result += compute_with_tls(argc);
    
    /* Return value depends on all TLS variables */
    return result % 256;  /* Ensure small return value */
}
