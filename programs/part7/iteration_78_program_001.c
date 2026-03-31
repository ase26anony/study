/* Main file with various TLS variable declarations */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")
#endif

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* 1. Public TLS with explicit visibility and used attribute */
__thread int tls_public __attribute__((used, visibility("default")));
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 2. Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 42;

/* 3. Common linkage (tentative definition) */
__thread int tls_common;

/* 4. External declaration (defined in tls_aux.c) */
extern __thread int tls_external;

/* 5. DLL import simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif
extern DLL_IMPORT __thread int tls_dllimport;

/* 6. Static TLS with context */
static void func_with_tls(void) {
    static __thread int tls_in_function = 100;
    tls_in_function++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_in_function));
}

/* 7. More complex TLS types */
__thread struct {
    int a;
    char b;
    void* ptr;
} tls_struct __attribute__((used));

/* 8. TLS with thread ID */
__thread uintptr_t thread_specific_id;

/* ===== FUNCTION DECLARATIONS ===== */
void test_tls_access(void);
void modify_tls_vars(void);
uintptr_t compute_tls_checksum(void);

/* ===== TEST FUNCTIONS ===== */
void test_tls_access(void) {
    /* Access all TLS variables to ensure they're used */
    tls_public = 1;
    tls_hidden = 2;
    
    if (&tls_weak) {  /* Force reference to weak symbol */
        tls_weak = 3;
    }
    
    tls_common = 4;
    tls_external = 5;  /* Defined in another file */
    
    /* tls_dllimport is external, just read if available */
    if (&tls_dllimport) {
        int val = tls_dllimport;
        (void)val;
    }
    
    tls_struct.a = 6;
    tls_struct.b = 'T';
    tls_struct.ptr = &tls_struct;
    
    thread_specific_id = (uintptr_t)&thread_specific_id;
    
    func_with_tls();
}

void modify_tls_vars(void) {
    /* Modify TLS variables differently */
    tls_public += 10;
    tls_hidden *= 2;
    
    if (&tls_weak) {
        tls_weak -= 1;
    }
    
    tls_common ^= 0xFF;
    tls_external += 20;
    
    tls_struct.a++;
    tls_struct.b = 'L';
    
    thread_specific_id = (thread_specific_id << 4) | 0xA;
}

uintptr_t compute_tls_checksum(void) {
    /* Compute a checksum to ensure all TLS vars are active */
    uintptr_t sum = 0;
    
    sum += tls_public;
    sum += tls_hidden;
    
    if (&tls_weak) {
        sum += tls_weak;
    }
    
    sum += tls_common;
    sum += tls_external;
    
    sum += tls_struct.a;
    sum += (uintptr_t)tls_struct.b;
    sum += (uintptr_t)tls_struct.ptr;
    
    sum += thread_specific_id;
    
    return sum;
}

/* ===== MAIN FUNCTION ===== */
int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Initialize TLS variables */
    test_tls_access();
    
    /* Modify them */
    for (int i = 0; i < 5; i++) {
        modify_tls_vars();
    }
    
    /* Compute and print checksum */
    uintptr_t checksum = compute_tls_checksum();
    printf("TLS checksum: 0x%lx\n", (unsigned long)checksum);
    
    /* Call function with static TLS multiple times */
    for (int i = 0; i < 3; i++) {
        func_with_tls();
    }
    
    /* Force all TLS addresses to be taken (prevents optimization) */
    void* addresses[] = {
        &tls_public,
        &tls_hidden,
        &tls_weak,
        &tls_common,
        &tls_external,
        &tls_dllimport,
        &tls_struct,
        &thread_specific_id
    };
    
    /* Use addresses in asm to prevent removal */
    asm volatile("" : : "r"(addresses) : "memory");
    
    printf("Test completed.\n");
    return 0;
}
