/* Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC target("tls")  /* Ensure TLS support is considered */
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Public TLS with explicit visibility and used attribute */
__thread int tls_public_used __attribute__((used, visibility("default"))) = 42;

/* 2. Weak TLS definition */
__thread int tls_weak_var __attribute__((weak)) = 100;

/* 3. Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* 4. Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* 5. DLL import simulation (even on non-Windows) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with a custom attribute or use weak */
__thread int tls_dllimport __attribute__((weak));
#endif

/* 6. External declaration (defined in another file) */
extern __thread int tls_external;

/* 7. Static TLS within a function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 300;
    tls_static_func++;
    /* Force usage to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* 8. TLS with thread ID pattern */
__thread uintptr_t tls_thread_id;

/* 9. TLS pointer */
__thread void* tls_pointer __attribute__((used));

/* 10. Another weak TLS for cross-file testing */
extern __thread int tls_weak_extern __attribute__((weak));

/* ========== HELPER FUNCTIONS ========== */

/* Force variable addresses to be taken without side effects */
#define FORCE_USE(var) asm volatile("" : : "r"(&(var)))

/* Function that uses all TLS variables to ensure they're live */
static void use_all_tls_vars(void) {
    /* Public used TLS */
    tls_public_used += 1;
    FORCE_USE(tls_public_used);
    
    /* Weak TLS */
    tls_weak_var *= 2;
    FORCE_USE(tls_weak_var);
    
    /* Common TLS */
    tls_common = tls_public_used + tls_weak_var;
    FORCE_USE(tls_common);
    
    /* Hidden TLS */
    tls_hidden -= 5;
    FORCE_USE(tls_hidden);
    
    /* DLL import TLS */
    tls_dllimport = 999;
    FORCE_USE(tls_dllimport);
    
    /* External TLS */
    tls_external++;
    FORCE_USE(tls_external);
    
    /* Weak extern TLS */
    if (&tls_weak_extern != NULL) {
        tls_weak_extern = 777;
        FORCE_USE(tls_weak_extern);
    }
    
    /* Thread ID TLS */
    tls_thread_id = (uintptr_t)__builtin_return_address(0);
    FORCE_USE(tls_thread_id);
    
    /* TLS pointer */
    tls_pointer = &tls_public_used;
    FORCE_USE(tls_pointer);
    
    /* Call function with static TLS */
    func_with_static_tls();
}

/* Function that creates different access patterns */
static void manipulate_tls(void) {
    /* Create dependency chain between TLS variables */
    tls_public_used = (tls_weak_var % 10) + tls_hidden;
    
    /* Use in conditional logic */
    if (tls_common > 1000) {
        tls_dllimport = tls_common / 2;
    } else {
        tls_dllimport = tls_common * 2;
    }
    
    /* Pointer arithmetic with TLS */
    tls_pointer = (void*)((uintptr_t)tls_pointer + tls_thread_id);
    
    /* Ensure external TLS is modified */
    tls_external ^= 0xABCD;
}

/* Checksum function to verify TLS values are active */
static uint32_t tls_checksum(void) {
    uint32_t sum = 0;
    
    /* Mix all TLS values into checksum */
    sum += tls_public_used;
    sum ^= tls_weak_var << 8;
    sum += tls_common;
    sum ^= tls_hidden << 16;
    sum += tls_dllimport;
    sum += tls_external;
    sum ^= (uint32_t)tls_thread_id;
    sum += (uintptr_t)tls_pointer & 0xFFFFFFFF;
    
    /* Handle potentially NULL weak extern */
    if (&tls_weak_extern != NULL) {
        sum += tls_weak_extern;
    }
    
    return sum;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    printf("Starting emulated TLS test...\n");
    
    /* Initial access to all TLS variables */
    use_all_tls_vars();
    
    /* Multiple manipulation cycles */
    for (int i = 0; i < 10; i++) {
        manipulate_tls();
        
        /* Calculate and print checksum occasionally */
        if (i % 3 == 0) {
            uint32_t sum = tls_checksum();
            printf("Iteration %d: TLS checksum = 0x%08X\n", i, sum);
        }
    }
    
    /* Final checksum */
    uint32_t final_sum = tls_checksum();
    printf("Final TLS checksum: 0x%08X\n", final_sum);
    
    /* Verify TLS variables have expected properties */
    printf("TLS variable addresses:\n");
    printf("  tls_public_used: %p\n", (void*)&tls_public_used);
    printf("  tls_weak_var:    %p\n", (void*)&tls_weak_var);
    printf("  tls_common:      %p\n", (void*)&tls_common);
    printf("  tls_hidden:      %p\n", (void*)&tls_hidden);
    printf("  tls_external:    %p\n", (void*)&tls_external);
    
    return (final_sum != 0) ? 0 : 1;
}
