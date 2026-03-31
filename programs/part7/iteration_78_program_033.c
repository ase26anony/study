/* Main file with various TLS declarations and usage */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC target("tls")  /* Ensure TLS support is considered */
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Public TLS with explicit visibility and used attribute */
__thread int tls_public __attribute__((used, visibility("default")));

/* 2. Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 42;

/* 3. Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* 4. Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;

/* 5. External declaration (defined in another file) */
extern __thread int tls_external;

/* 6. DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with a custom attribute if not on Windows */
#define DLLIMPORT __attribute__((dllimport))
extern DLLIMPORT __thread int tls_dllimport;
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 999;
    tls_static_func++;
    /* Force address taking without optimization removal */
    asm volatile("" : : "r"(&tls_static_func));
}

/* 8. TLS with thread ID storage */
__thread uintptr_t thread_specific_id;

/* 9. TLS pointer */
__thread void* tls_pointer __attribute__((used));

/* ========== FUNCTION DECLARATIONS ========== */
void test_tls_operations(void);
void test_extern_tls(void);
uint32_t compute_tls_checksum(void);

/* ========== HELPER FUNCTIONS ========== */

/* Force variable usage to prevent elimination */
static void force_use(void* var) {
    asm volatile("" : : "r"(var) : "memory");
}

/* Function to test TLS operations */
void test_tls_operations(void) {
    /* Access and modify all TLS variables */
    tls_public = 1;
    tls_weak = tls_weak + 1;  /* Use weak variable */
    tls_common = 123;
    tls_hidden = tls_hidden * 2;
    
    /* Call function with static TLS */
    func_with_static_tls();
    
    /* Store thread-specific data */
    thread_specific_id = (uintptr_t)&thread_specific_id;  /* Use address as ID */
    tls_pointer = &tls_public;
    
    /* Force usage of all addresses */
    force_use(&tls_public);
    force_use(&tls_weak);
    force_use(&tls_common);
    force_use(&tls_hidden);
    force_use(&thread_specific_id);
    force_use(&tls_pointer);
}

/* Compute checksum of all TLS values */
uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    
    /* Access all TLS variables to ensure they're live */
    sum += tls_public;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_hidden;
    sum += (uint32_t)thread_specific_id;
    sum += (uint32_t)(uintptr_t)tls_pointer;
    
    /* Add external TLS if available */
    extern int get_external_tls_value(void);
    sum += get_external_tls_value();
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Perform multiple TLS operations */
    for (int i = 0; i < 3; i++) {
        test_tls_operations();
        test_extern_tls();
        
        uint32_t checksum = compute_tls_checksum();
        printf("Iteration %d: TLS checksum = %u\n", i, checksum);
    }
    
    /* Verify TLS variables maintain separate storage */
    tls_public = 1000;
    tls_common = 2000;
    
    printf("Final values: tls_public=%d, tls_common=%d\n", 
           tls_public, tls_common);
    
    return 0;
}
