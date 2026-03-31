/* tls_main.c - Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern int get_tls_sum(void);
extern void opaque_operation(int* a, int* b, int* c);

/* Declaration with external linkage - will be defined in another file */
extern __thread int extern_tls_var;

/* TLS with visibility attribute */
__thread int global_tls_var __attribute__((visibility("default")));

/* Weak TLS declaration */
__thread int weak_tls_var __attribute__((weak));

/* TLS with used attribute to ensure TREE_USED is set */
static __thread int static_tls_used __attribute__((used));

/* TLS with hidden visibility */
static __thread int static_tls_hidden __attribute__((visibility("hidden")));

/* For Windows DLL import simulation */
#ifdef _WIN32
    #define DLLIMPORT __declspec(dllimport)
#else
    #define DLLIMPORT __attribute__((dllimport))
#endif

/* Try to trigger DECL_DLLIMPORT_P */
extern DLLIMPORT __thread int dllimport_tls_var;

/* Constructor to initialize TLS variables with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    global_tls_var = rand() % 100;
    weak_tls_var = rand() % 100;
    static_tls_used = rand() % 100;
    static_tls_hidden = rand() % 100;
}

/* Opaque function that takes addresses of TLS variables */
static __attribute__((noinline))
void use_tls_addresses(void) {
    /* Take addresses of multiple TLS variables */
    int* addr1 = &global_tls_var;
    int* addr2 = &static_tls_used;
    int* addr3 = &static_tls_hidden;
    
    /* Use inline assembly to force address usage */
    __asm__ volatile (
        "# TLS address usage %0, %1, %2"
        : 
        : "r" (addr1), "r" (addr2), "r" (addr3)
        : "memory"
    );
    
    /* Call opaque operation with TLS addresses */
    opaque_operation(&global_tls_var, &static_tls_used, &static_tls_hidden);
}

/* Another function that creates complex context for TLS usage */
static int complex_tls_usage(void) {
    /* Use GNU statement expression with TLS variable */
    int result = ({
        __thread int local_tls_stmt;
        local_tls_stmt = global_tls_var + weak_tls_var;
        
        /* Take address within statement expression */
        int* addr = &local_tls_stmt;
        
        /* Inline assembly to prevent optimization */
        __asm__ volatile (
            "# Statement expr TLS %0"
            : 
            : "r" (addr)
            : "memory"
        );
        
        local_tls_stmt * 2;
    });
    
    return result;
}

/* Function that forces declaration duplication through multiple scopes */
static void force_duplication(void) {
    /* Macro that expands to multiple TLS uses */
    #define USE_TLS_MACRO(var) \
        do { \
            __thread int macro_tls; \
            macro_tls = (var); \
            int* addr = &macro_tls; \
            __asm__ volatile ("# Macro TLS %0" : : "r"(addr) : "memory"); \
        } while(0)
    
    /* Use the macro multiple times */
    USE_TLS_MACRO(global_tls_var);
    USE_TLS_MACRO(weak_tls_var);
    
    /* Another TLS in nested block */
    {
        __thread int nested_tls = global_tls_var + 1;
        int* nested_addr = &nested_tls;
        __asm__ volatile ("# Nested TLS %0" : : "r"(nested_addr) : "memory");
    }
}

int main(void) {
    /* Initialize extern TLS variable */
    extern_tls_var = 42;
    
    /* Local TLS variable in main */
    __thread int main_local_tls = 100;
    
    /* Take address and pass to opaque operation */
    int* main_tls_addr = &main_local_tls;
    
    /* Use inline assembly with TLS address */
    __asm__ volatile (
        "# Main TLS address %0"
        : 
        : "r" (main_tls_addr)
        : "memory"
    );
    
    /* Call functions that use TLS variables */
    use_tls_addresses();
    force_duplication();
    
    int complex_result = complex_tls_usage();
    
    /* Compute checksum using all TLS variables */
    int checksum = global_tls_var 
                   + weak_tls_var 
                   + static_tls_used 
                   + static_tls_hidden 
                   + extern_tls_var
                   + main_local_tls
                   + complex_result;
    
    /* Add external TLS sum */
    checksum += get_tls_sum();
    
    printf("TLS checksum: %d\n", checksum);
    
    /* Ensure all TLS variables are marked as used */
    volatile int* volatile_addrs[] = {
        &global_tls_var,
        &weak_tls_var,
        &static_tls_used,
        &static_tls_hidden,
        &extern_tls_var,
        &main_local_tls
    };
    
    /* Prevent dead code elimination */
    __asm__ volatile (
        "# Final TLS barrier"
        : 
        : "r" (volatile_addrs)
        : "memory"
    );
    
    return 0;
}
