/* Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")  /* Prevent optimization removing TLS vars */
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* Public TLS with explicit visibility and used attribute */
__thread int tls_public __attribute__((used, visibility("default")));

/* Weak TLS definition - can be overridden */
__thread int tls_weak __attribute__((weak)) = 42;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* External TLS declaration - defined in another file */
extern __thread int tls_external;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* TLS with internal linkage (static) */
static __thread int tls_static = 999;

/* TLS pointer */
__thread void* tls_pointer __attribute__((used));

/* TLS in different context - inside a function */
static void function_with_tls(void) {
    /* Local TLS with automatic storage context */
    static __thread int tls_local_context = 555;
    tls_local_context++;
    /* Force address taken to prevent optimization */
    asm volatile("" : : "r"(&tls_local_context));
}

/* ========== TEST FUNCTIONS ========== */

/* Function that uses all TLS variables to prevent elimination */
void use_all_tls_vars(void) {
    /* Public TLS */
    tls_public = 1;
    tls_public++;
    
    /* Weak TLS */
    tls_weak = 2;
    tls_weak += 3;
    
    /* Hidden TLS */
    tls_hidden = 4;
    tls_hidden *= 2;
    
    /* Common TLS */
    tls_common = 5;
    tls_common--;
    
    /* External TLS */
    tls_external = 6;
    
    /* DLL import TLS */
    tls_dllimport = 7;
    
    /* Static TLS */
    tls_static = 8;
    tls_static /= 2;
    
    /* TLS pointer */
    tls_pointer = &tls_public;
    
    /* Call function with local context TLS */
    function_with_tls();
    
    /* Force all addresses to be taken */
    asm volatile("" : : 
        "r"(&tls_public),
        "r"(&tls_weak),
        "r"(&tls_hidden),
        "r"(&tls_common),
        "r"(&tls_external),
        "r"(&tls_dllimport),
        "r"(&tls_static),
        "r"(&tls_pointer)
    );
}

/* Function that calculates checksum of TLS variables */
uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_external;
    sum += tls_dllimport;
    sum += tls_static;
    sum += (uintptr_t)tls_pointer & 0xFFFF;
    
    return sum;
}

/* Thread-like usage pattern */
void thread_like_operation(int id) {
    /* Each "thread" gets its own TLS values */
    tls_public = id * 1000;
    tls_weak = id * 100;
    tls_hidden = id * 10;
    tls_common = id;
    
    /* Use the values */
    tls_static = tls_public + tls_weak + tls_hidden + tls_common;
    
    /* Store thread ID in pointer (as integer) */
    tls_pointer = (void*)(uintptr_t)id;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Phase 1: Initial usage to instantiate TLS */
    use_all_tls_vars();
    
    /* Phase 2: Simulate different thread contexts */
    for (int i = 0; i < 3; i++) {
        thread_like_operation(i + 1);
        
        /* Calculate and print checksum for this "thread" */
        uint32_t sum = tls_checksum();
        printf("Thread-like context %d: TLS checksum = %u\n", i + 1, sum);
        
        /* Verify external TLS is accessible */
        printf("  External TLS value: %d\n", tls_external);
    }
    
    /* Phase 3: Test weak linkage override */
    tls_weak = 0xDEADBEEF;
    printf("Weak TLS set to: 0x%x\n", tls_weak);
    
    /* Phase 4: Test common linkage */
    tls_common = 0xCAFEBABE;
    printf("Common TLS set to: 0x%x\n", tls_common);
    
    /* Final checksum */
    printf("Final TLS checksum: %u\n", tls_checksum());
    
    return 0;
}
