/* Main test file for emulated TLS attribute copying coverage */
#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode */
#if defined(__GNUC__) && !defined(__HAVE_TLS)
#define EMUTLS_MODE 1
#else
/* Try to force emulated TLS via compiler flag detection */
#ifdef __FORCE_EMUTLS
#define EMUTLS_MODE 1
#else
#define EMUTLS_MODE 0
#endif
#endif

/* Windows DLL import/export attributes */
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLEXPORT __attribute__((visibility("default")))
#define DLLIMPORT
#endif

/* External TLS declaration - triggers DECL_EXTERNAL */
extern __thread int tls_external;

/* Weak TLS variable - triggers DECL_WEAK */
__thread int tls_weak __attribute__((weak));

/* TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* Public TLS variable - triggers TREE_PUBLIC */
__thread int tls_public = 42;

/* Preserved TLS variable - may trigger DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* DLL Import TLS for Windows targets - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
DLLIMPORT __thread int tls_imported;
#else
/* On non-Windows, use visibility to simulate similar behavior */
__thread int tls_imported __attribute__((visibility("default")));
#endif

/* TLS in different contexts - affects DECL_CONTEXT */
static void function_with_tls(void) {
    /* TLS with function scope context */
    static __thread int tls_function_scope = 100;
    volatile int *p = &tls_function_scope;
    (void)p;
}

/* Helper function to prevent optimization */
__attribute__((noinline)) 
static uintptr_t compute_tls_checksum(void) {
    uintptr_t checksum = 0;
    
    /* Take addresses of all TLS variables */
    checksum ^= (uintptr_t)&tls_weak;
    checksum ^= (uintptr_t)&tls_hidden;
    checksum ^= (uintptr_t)&tls_common;
    checksum ^= (uintptr_t)&tls_public;
    checksum ^= (uintptr_t)&tls_preserved;
    checksum ^= (uintptr_t)&tls_imported;
    
    /* Use their values */
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_common;
    checksum += tls_public;
    checksum += tls_preserved;
    checksum += tls_imported;
    
    /* External TLS reference */
    checksum ^= (uintptr_t)&tls_external;
    
    return checksum;
}

/* Global volatile array to prevent optimization */
volatile uintptr_t tls_addresses[10];

int main(void) {
    /* Initialize some TLS variables */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_preserved = 4;
    
    /* Call function with scope-specific TLS */
    function_with_tls();
    
    /* Compute and use checksum */
    uintptr_t checksum = compute_tls_checksum();
    
    /* Store addresses to prevent optimization */
    tls_addresses[0] = (uintptr_t)&tls_weak;
    tls_addresses[1] = (uintptr_t)&tls_hidden;
    tls_addresses[2] = (uintptr_t)&tls_common;
    tls_addresses[3] = (uintptr_t)&tls_public;
    tls_addresses[4] = (uintptr_t)&tls_preserved;
    tls_addresses[5] = (uintptr_t)&tls_imported;
    
    /* Force comparison of TLS addresses */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Print checksum to ensure TLS is used */
    printf("TLS checksum: 0x%lx\n", (unsigned long)checksum);
    
    /* Print individual TLS values */
    printf("tls_weak: %d\n", tls_weak);
    printf("tls_hidden: %d\n", tls_hidden);
    printf("tls_common: %d\n", tls_common);
    printf("tls_public: %d\n", tls_public);
    printf("tls_preserved: %d\n", tls_preserved);
    
    return 0;
}
