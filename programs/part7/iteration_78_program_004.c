/* Main file with various TLS variable declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Public TLS with explicit visibility and used attribute */
__thread int tls_public __attribute__((used, visibility("default")));

/* 2. Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 42;

/* 3. Common linkage (tentative definition) */
__thread int tls_common;

/* 4. Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 5. DLL import simulation (for attribute copying) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* 6. External TLS declaration (defined in another file) */
extern __thread int tls_external;

/* 7. Static TLS within function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 100;
    tls_static_func++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* 8. TLS with all attributes combined */
__thread int tls_combo __attribute__((used, weak, visibility("hidden")));

/* 9. Thread-local pointer */
__thread void* tls_pointer;

/* 10. Thread-local array */
__thread char tls_array[64];

/* ========== FUNCTION DECLARATIONS ========== */
void modify_tls_vars(void);
uint32_t compute_tls_checksum(void);

/* ========== HELPER FUNCTIONS ========== */
static __thread int tls_counter = 0;

void increment_counter(void) {
    tls_counter++;
    /* Force use of all TLS variables to prevent elimination */
    tls_public = tls_counter;
    tls_weak = tls_counter * 2;
    tls_common = tls_counter * 3;
    tls_hidden = tls_counter * 4;
    
    /* Access external TLS */
    tls_external = tls_counter * 5;
    
    /* Access DLL import style */
    tls_dllimport = tls_counter * 6;
    
    tls_combo = tls_counter * 7;
    
    /* Use the pointer */
    tls_pointer = &tls_counter;
    
    /* Use the array */
    tls_array[0] = (char)tls_counter;
    tls_array[1] = (char)(tls_counter >> 8);
}

uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    
    /* Access all TLS variables to compute checksum */
    sum += tls_public;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_hidden;
    sum += tls_external;
    sum += tls_dllimport;
    sum += tls_combo;
    sum += (uintptr_t)tls_pointer;
    sum += tls_array[0];
    sum += tls_array[1];
    sum += tls_counter;
    
    /* Call function with static TLS */
    func_with_static_tls();
    
    return sum;
}

/* Thread ID simulation using TLS */
__thread unsigned int thread_id = 0;

void set_thread_id(unsigned int id) {
    thread_id = id;
    /* Force attribute copying by using thread_id in complex way */
    tls_public = id * 0x12345678;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Initialize thread ID */
    set_thread_id(1);
    
    /* Modify TLS variables multiple times */
    for (int i = 0; i < 10; i++) {
        increment_counter();
        modify_tls_vars();
    }
    
    /* Compute and print checksum */
    uint32_t checksum = compute_tls_checksum();
    printf("TLS checksum: 0x%08x\n", checksum);
    
    /* Force use of all variables in different contexts */
    if (tls_public > 0) {
        tls_weak = tls_public;
    }
    
    if (tls_common != 0) {
        tls_hidden = tls_common;
    }
    
    /* Access through pointer */
    if (tls_pointer) {
        *(int*)tls_pointer = checksum;
    }
    
    printf("Thread ID: %u\n", thread_id);
    printf("All TLS variables accessed successfully.\n");
    
    return 0;
}
