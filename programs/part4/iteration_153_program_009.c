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

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 500;
#else
/* On non-Windows, use a similar attribute if available, or just regular TLS */
extern __thread int tls_imported __attribute__((weak));
__thread int tls_imported = 500;
#endif

/* Additional TLS variables with different types and initializations */
__thread double tls_double = 3.14159;
__thread void* tls_pointer = 0;
__thread volatile int tls_volatile = 99;

/* Helper function to force address taking and prevent optimization */
__attribute__((noinline)) 
static void use_tls_addresses(int* static_ptr, char* extern_ptr, int* weak_ptr,
                              long* hidden_ptr, long* protected_ptr, int* import_ptr,
                              double* double_ptr, void** ptr_ptr, volatile int* volatile_ptr) {
    /* Dummy operations to prevent optimization */
    *static_ptr += 1;
    *extern_ptr += 1;
    *weak_ptr += 1;
    *hidden_ptr += 1;
    *protected_ptr += 1;
    *import_ptr += 1;
    *double_ptr += 1.0;
    *ptr_ptr = (void*)((uintptr_t)*ptr_ptr + 1);
    (void)*volatile_ptr; /* Read volatile to force access */
}

/* Another helper that uses TLS variables in computations */
__attribute__((noinline, const))
static int compute_with_tls(int arg) {
    /* Use all TLS variables in a computation */
    int result = tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += (int)tls_hidden;
    result += (int)tls_protected;
    result += tls_imported;
    result += (int)tls_double;
    result += (int)(uintptr_t)tls_pointer;
    result += tls_volatile;
    
    return result * arg;
}

/* Function to ensure TLS variables are used in different scopes */
static void modify_tls_variables(int argc) {
    /* Modify Pattern A */
    tls_static_init = argc * 10;
    
    /* Modify Pattern B */
    tls_extern = 'A' + (argc % 26);
    
    /* Modify Pattern C if it exists (weak symbol might be absent) */
    if (&tls_weak) {
        tls_weak = argc * 20;
    }
    
    /* Modify Pattern D */
    tls_hidden = argc * 100L;
    tls_protected = argc * 200L;
    
    /* Modify Pattern E */
    tls_imported = argc * 50;
    
    /* Modify additional TLS variables */
    tls_double = argc * 3.14;
    tls_pointer = (void*)(uintptr_t)argc;
    tls_volatile = argc * 7;
}

int main(int argc, char **argv) {
    int i;
    
    /* Initial modification based on argc */
    modify_tls_variables(argc);
    
    /* Use TLS variables in a loop with control flow */
    int sum = 0;
    for (i = 0; i < argc; i++) {
        /* Conditional access to different TLS variables */
        if (i % 2 == 0) {
            sum += tls_static_init;
            tls_extern += i;
        } else if (i % 3 == 0) {
            sum += tls_weak;
            tls_hidden += i;
        } else if (i % 5 == 0) {
            sum += tls_imported;
            tls_protected += i;
        } else {
            sum += (int)tls_double;
            tls_volatile += i;
        }
        
        /* Periodically update tls_pointer */
        if (i % 4 == 0) {
            tls_pointer = (void*)(sum + i);
        }
    }
    
    /* Force address taking of all TLS variables */
    use_tls_addresses(&tls_static_init, &tls_extern, &tls_weak,
                     &tls_hidden, &tls_protected, &tls_imported,
                     &tls_double, &tls_pointer, &tls_volatile);
    
    /* Final computation using all TLS variables */
    int final_result = compute_with_tls(argc);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d (argc=%d)\n", final_result + sum, argc);
    
    return (final_result + sum) > 100 ? 0 : 1;
}

/* Additional file-like separation using weak symbols */
/* This creates a "common" TLS symbol scenario */
__thread int tls_common __attribute__((common));

/* Force a reference to tls_common */
__attribute__((constructor))
static void init_tls_common(void) {
    tls_common = 12345;
}
