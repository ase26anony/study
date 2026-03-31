/* Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS compilation */
#ifdef __GNUC__
#pragma GCC optimize("O0")  /* Prevent optimization of TLS variables */
#endif

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

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

/* TLS with complex type */
__thread struct {
    int a;
    double b;
    void* ptr;
} tls_complex __attribute__((used)) = {1, 3.14, 0};

/* ========== FUNCTION DECLARATIONS ========== */
void test_extern_tls(void);
void test_weak_tls(void);
void test_visibility_tls(void);
uint32_t compute_tls_checksum(void);

/* ========== HELPER FUNCTIONS ========== */

/* Force usage of all TLS variables to prevent elimination */
static void use_all_tls_vars(void) {
    /* Public used TLS */
    tls_public_used += 1;
    asm volatile("" : : "r"(&tls_public_used));
    
    /* Weak TLS */
    if (&tls_weak) {
        tls_weak *= 2;
    }
    
    /* Common TLS */
    tls_common = 1234;
    asm volatile("" : : "r"(&tls_common));
    
    /* Hidden TLS */
    tls_hidden -= 5;
    asm volatile("" : : "r"(&tls_hidden));
    
    /* External TLS */
    tls_external = 999;
    asm volatile("" : : "r"(&tls_external));
    
    /* DLL import TLS */
    tls_dllimport = 777;
    asm volatile("" : : "r"(&tls_dllimport));
    
    /* Complex TLS */
    tls_complex.a++;
    tls_complex.b *= 2.0;
    asm volatile("" : : "r"(&tls_complex));
    
    /* Function static TLS */
    func_with_static_tls();
}

/* Thread-like behavior simulation */
static void simulate_thread_usage(void) {
    static int call_count = 0;
    call_count++;
    
    /* Each "thread" would have different values */
    tls_public_used = call_count * 10;
    tls_common = call_count * 20;
    tls_hidden = call_count * 30;
    
    /* Use conditional logic to ensure variables are live */
    if (tls_public_used > 50) {
        tls_weak = tls_public_used / 2;
    }
    
    /* Pointer arithmetic with TLS */
    int* tls_ptr = &tls_common;
    *tls_ptr += 1;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    printf("Testing emulated TLS attribute copying...\n");
    
    /* Phase 1: Initial access to all TLS variables */
    use_all_tls_vars();
    
    /* Phase 2: Test functions from other translation units */
    test_extern_tls();
    test_weak_tls();
    test_visibility_tls();
    
    /* Phase 3: Simulate thread-like usage patterns */
    for (int i = 0; i < 5; i++) {
        simulate_thread_usage();
    }
    
    /* Phase 4: Compute and print checksum */
    uint32_t checksum = compute_tls_checksum();
    printf("TLS checksum: 0x%08x\n", checksum);
    
    /* Phase 5: Verify values are preserved */
    printf("tls_public_used = %d\n", tls_public_used);
    printf("tls_common = %d\n", tls_common);
    printf("tls_hidden = %d\n", tls_hidden);
    
    return 0;
}
