/* test_emutls.c - Comprehensive TLS emulation test */
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
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported = 5000;
#endif

/* Additional TLS variables with different types and attributes */
__thread volatile float tls_volatile = 3.14f;
__thread const char* tls_pointer = "TLS string";
__thread _Thread_local double tls_c11 = 2.71828;

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline, used))
static void use_tls_addresses(void* addr1, void* addr2, void* addr3, 
                              void* addr4, void* addr5, void* addr6) {
    /* Dummy writes to force materialization of addresses */
    volatile static int sink = 0;
    sink += (int)((uintptr_t)addr1 ^ (uintptr_t)addr2);
    sink += (int)((uintptr_t)addr3 ^ (uintptr_t)addr4);
    sink += (int)((uintptr_t)addr5 ^ (uintptr_t)addr6);
    
    /* Prevent the compiler from optimizing away the function */
    __asm__ __volatile__("" : : "r"(sink) : "memory");
}

/* Another helper to use TLS values */
__attribute__((noinline, used))
static int compute_with_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computations */
    result += tls_static_init * argc;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 256;
    result += tls_protected % 256;
    result += tls_imported % 256;
    result += (int)tls_volatile;
    result += (int)(tls_c11 * 100);
    
    /* Force pointer dereference */
    if (tls_pointer && tls_pointer[0]) {
        result += tls_pointer[0];
    }
    
    return result;
}

/* Function to modify TLS variables */
__attribute__((noinline, used))
static void modify_tls_vars(int argc) {
    /* Modify each TLS variable based on argc */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30;
    tls_protected = argc * 40;
    tls_imported = argc * 50;
    tls_volatile = argc * 0.1f;
    tls_c11 = argc * 0.01;
    
    /* Create control flow dependencies */
    if (argc > 1) {
        static __thread int tls_conditional = 0;
        for (int i = 0; i < argc; i++) {
            tls_conditional += i;
        }
        tls_static_init += tls_conditional;
    }
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initial computation */
    result = compute_with_tls(argc);
    
    /* Modify TLS variables */
    modify_tls_vars(argc);
    
    /* Force address taking of all TLS variables */
    use_tls_addresses(&tls_static_init, &tls_extern, &tls_weak,
                      &tls_hidden, &tls_protected, &tls_imported);
    
    /* Use volatile TLS variable to prevent optimization */
    volatile float temp = tls_volatile;
    result += (int)(temp * 100);
    
    /* Final computation with modified values */
    result += compute_with_tls(argc);
    
    /* Create output that depends on all TLS variables */
    printf("Result: %d\n", result);
    printf("TLS values: %d %c %d %ld %ld %d %f %f\n",
           tls_static_init, tls_extern, tls_weak,
           tls_hidden, tls_protected, tls_imported,
           tls_volatile, tls_c11);
    
    return result % 256;
}

/* Additional file-scope TLS usage to ensure DECL_CONTEXT is set */
__thread int file_scope_tls = 12345;

void __attribute__((constructor)) init_tls(void) {
    /* Access TLS in constructor to ensure early initialization */
    file_scope_tls = 54321;
}

/* Weak external reference to test DECL_EXTERNAL and DECL_COMMON */
extern __thread int tls_tentative;
__thread int tls_tentative;  /* Tentative definition - tests DECL_COMMON */

/* Thread-local array to test more complex types */
__thread int tls_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
