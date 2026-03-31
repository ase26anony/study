/* main.c - Main program with TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
void opaque_operation(int* a, long* b, double* c);
int compute_checksum(void);

/* ===================== TLS DECLARATIONS WITH VARIOUS ATTRIBUTES ===================== */

/* Plain __thread with external linkage - will be defined in another TU */
extern __thread int extern_tls_int __attribute__((used));

/* Static TLS with file-local linkage */
static __thread long static_tls_long __attribute__((visibility("hidden")));

/* Weak TLS declaration */
__thread double weak_tls_double __attribute__((weak, used));

/* TLS with explicit default visibility */
__thread char visible_tls_char __attribute__((visibility("default"), used));

/* TLS that might be imported (Windows-specific attribute with fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_int;
#else
__thread int imported_tls_int __attribute__((used));
#endif

/* TLS with common linkage (affected by -fno-common) */
__thread int common_tls_int;

/* ===================== INITIALIZATION FUNCTIONS ===================== */

/* Constructor to initialize TLS variables with non-constant values */
__attribute__((constructor))
static void init_tls_variables(void) {
    static int initialized = 0;
    if (initialized) return;
    initialized = 1;
    
    srand(time(NULL));
    
    /* Initialize with random values to prevent constant folding */
    static_tls_long = rand() % 1000;
    weak_tls_double = (double)(rand() % 1000) / 10.0;
    visible_tls_char = 'A' + (rand() % 26);
    common_tls_int = rand() % 100;
    
    printf("Initialized TLS variables\n");
}

/* ===================== HELPER FUNCTIONS ===================== */

/* Noinline function to force address taking and prevent optimization */
static __attribute__((noinline, used))
void use_tls_addresses(void) {
    /* Take addresses of TLS variables - may trigger declaration duplication */
    int* int_ptr = &extern_tls_int;
    long* long_ptr = &static_tls_long;
    double* double_ptr = &weak_tls_double;
    char* char_ptr = &visible_tls_char;
    int* imported_ptr = &imported_tls_int;
    int* common_ptr = &common_tls_int;
    
    /* Pass to opaque operation */
    opaque_operation(int_ptr, long_ptr, double_ptr);
    
    /* Use statement expression with TLS address - creates complex context */
    int result = ({
        int temp = *int_ptr + *long_ptr;
        temp += (int)(*double_ptr);
        temp += *char_ptr;
        temp;
    });
    
    /* Use inline assembly with TLS variable reference */
    asm volatile (
        "# TLS reference: %0"
        : 
        : "r" (common_ptr)
        : "memory"
    );
}

/* External function that uses TLS variables */
void process_tls_values(void) {
    /* Local TLS variable - may trigger duplication in local scope */
    __thread int local_tls_int = 42;
    
    /* Take address and use in complex expression */
    int* local_ptr = &local_tls_int;
    
    /* Use GNU statement expression with call to external function */
    int computed = ({
        int sum = local_tls_int + extern_tls_int;
        /* Force declaration usage in nested context */
        asm volatile ("# Using TLS: %0" : : "r" (local_ptr));
        sum;
    });
    
    /* Pass to helper */
    use_tls_addresses();
}

/* ===================== MAIN FUNCTION ===================== */

int main(void) {
    printf("Starting TLS emulation test...\n");
    
    /* Initialize extern TLS variable */
    extern_tls_int = 100;
    
    /* Process TLS values */
    process_tls_values();
    
    /* Compute and print checksum */
    int checksum = compute_checksum();
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional complex usage pattern */
    {
        /* Another local TLS with different scope */
        __thread int another_local __attribute__((used)) = checksum;
        
        /* Use address in inline asm */
        asm volatile (
            "# Another TLS reference %0"
            : 
            : "r" (&another_local)
        );
    }
    
    return 0;
}
