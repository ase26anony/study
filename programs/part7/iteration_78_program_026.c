/* Main file with various TLS declarations and usage */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")
#endif

/* ===== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ===== */

/* Public TLS with explicit visibility and used attribute */
__thread int tls_public_used __attribute__((used, visibility("default"))) = 42;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 100;

/* Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* External TLS declaration (defined in another file) */
extern __thread int tls_external;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute on non-Windows */
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 300;
    tls_static_func++;
    /* Force address taking to prevent optimization */
    asm volatile("" : : "r"(&tls_static_func));
}

/* TLS pointer */
__thread void* tls_pointer __attribute__((visibility("default")));

/* Weak external reference */
extern __thread int tls_weak_extern __attribute__((weak));

/* ===== FUNCTION DECLARATIONS ===== */
void test_file2(void);
uint32_t compute_tls_checksum(void);

/* ===== FUNCTIONS THAT USE TLS ===== */

void modify_public_tls(void) {
    tls_public_used += 1;
    /* Ensure variable is considered used */
    asm volatile("" : : "r"(&tls_public_used));
}

void use_weak_tls(void) {
    if (&tls_weak != NULL) {
        tls_weak *= 2;
    }
    if (&tls_weak_extern != NULL) {
        /* Access weak extern if available */
        asm volatile("" : : "r"(&tls_weak_extern));
    }
}

void set_common_tls(int value) {
    tls_common = value;
    /* Prevent dead store elimination */
    asm volatile("" : : "r"(&tls_common));
}

void use_hidden_tls(void) {
    tls_hidden -= 5;
    /* Force compiler to keep the variable */
    asm volatile("" : : "r"(&tls_hidden));
}

void use_external_tls(void) {
    /* External TLS should be defined in another file */
    tls_external++;
    asm volatile("" : : "r"(&tls_external));
}

void use_dllimport_tls(void) {
#ifdef _WIN32
    tls_dllimport = 999;
#else
    /* For non-Windows, we'll define it locally but marked dllimport */
    extern __thread int tls_dllimport __attribute__((dllimport));
    asm volatile("" : : "r"(&tls_dllimport));
#endif
}

void set_tls_pointer(void* ptr) {
    tls_pointer = ptr;
    asm volatile("" : : "r"(&tls_pointer));
}

/* Compute checksum of all TLS variables to ensure they're used */
uint32_t compute_tls_checksum(void) {
    uint32_t sum = 0;
    
    sum += tls_public_used;
    sum += tls_weak;
    sum += tls_common;
    sum += tls_hidden;
    sum += tls_external;
    
    /* Cast pointer to integer for checksum */
    sum += (uintptr_t)tls_pointer;
    
    func_with_static_tls();
    
    return sum;
}

int main(void) {
    uint32_t checksum;
    
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Access and modify all TLS variables */
    modify_public_tls();
    use_weak_tls();
    set_common_tls(123);
    use_hidden_tls();
    use_external_tls();
    use_dllimport_tls();
    set_tls_pointer((void*)0xABCD);
    
    /* Call function from another translation unit */
    test_file2();
    
    /* Compute final checksum */
    checksum = compute_tls_checksum();
    
    printf("TLS checksum: %u\n", checksum);
    printf("All TLS variables accessed and modified.\n");
    
    return 0;
}
