/* test-emutls-coverage.c
 * Comprehensive test to cover emulated TLS property copying in GCC's tree-emutls.cc
 * Lines 295-304: DECL_* and TREE_* flag copying from original TLS decl to emulated TLS structure
 */

#include <stdio.h>
#include <stdint.h>

/* Force emulated TLS mode if supported */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#pragma GCC tls_model emulated
#endif

/* Prevent optimization of TLS variable usage */
volatile void* volatile tls_addresses[10];
volatile int volatile tls_values[10];
int checksum = 0;

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));
__thread int tls_internal __attribute__((visibility("internal")));
__thread int tls_protected __attribute__((visibility("protected")));

/* 3. Common TLS variable (tentative definition) - may set DECL_COMMON */
__thread int tls_common;

/* 4. External/Public TLS declaration */
extern __thread int tls_external;
__thread int tls_public __attribute__((used));

/* 5. DLL Import TLS (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__attribute__((dllimport)) __thread int tls_imported;
#else
/* On non-Windows, simulate with weak external */
extern __thread int tls_imported __attribute__((weak));
#endif

/* 6. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 7. Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 8. TLS with section attribute - additional complexity */
__thread int tls_sectioned __attribute__((section(".tdata"))) = 100;

/* 9. Static TLS inside function - different DECL_CONTEXT */
static void function_with_static_tls(void) {
    static __thread int tls_static_inside_func = 123;
    tls_addresses[8] = (void*)&tls_static_inside_func;
    tls_values[8] = tls_static_inside_func;
}

/* 10. TLS as static member simulation (C approach) */
struct fake_class {
    int dummy;
};
static struct fake_class class_context;
/* Simulate class member TLS by associating with a context */
#define TLS_IN_CLASS __thread int tls_class_member __attribute__((used))
TLS_IN_CLASS;

/* Helper function that uses all TLS variables - marked noinline to prevent optimization */
__attribute__((noinline, used))
static void use_all_tls_variables(void) {
    /* Take addresses - forces TLS structure creation */
    tls_addresses[0] = (void*)&tls_weak;
    tls_addresses[1] = (void*)&tls_hidden;
    tls_addresses[2] = (void*)&tls_internal;
    tls_addresses[3] = (void*)&tls_protected;
    tls_addresses[4] = (void*)&tls_common;
    tls_addresses[5] = (void*)&tls_public;
    tls_addresses[6] = (void*)&tls_imported;
    tls_addresses[7] = (void*)&tls_preserved;
    
    /* Use values - prevents dead code elimination */
    tls_weak = 1;
    tls_hidden = 2;
    tls_internal = 3;
    tls_protected = 4;
    tls_common = 5;
    tls_public = 6;
    tls_preserved = 7;
    tls_init = 8;
    tls_sectioned = 9;
    
    /* Store values for checksum calculation */
    tls_values[0] = tls_weak;
    tls_values[1] = tls_hidden;
    tls_values[2] = tls_internal;
    tls_values[3] = tls_protected;
    tls_values[4] = tls_common;
    tls_values[5] = tls_public;
    tls_values[6] = 0; /* tls_imported might not be defined */
    tls_values[7] = tls_preserved;
    tls_values[9] = tls_init + tls_sectioned;
    
    /* Force different code paths */
    if (&tls_weak != &tls_hidden) {
        tls_values[0] += 1000;
    }
    
    /* Call function with static TLS */
    function_with_static_tls();
    
    /* Use class-like TLS */
    tls_class_member = 999;
    tls_addresses[9] = (void*)&tls_class_member;
}

/* Another function in different compilation context */
__attribute__((noinline, used))
static void modify_tls_values(void) {
    tls_weak += 10;
    tls_hidden *= 2;
    tls_common = tls_public + tls_init;
}

/* External TLS definition (would normally be in another file) */
__thread int tls_external = 12345;

/* Weak TLS definition (provides definition if none in other files) */
__thread int tls_imported = 6789;

int main(void) {
    int i;
    
    printf("Testing emulated TLS property copying coverage\n");
    printf("=============================================\n");
    
    /* Use all TLS variables to force emutls_decl calls */
    use_all_tls_variables();
    
    /* Modify them again */
    modify_tls_values();
    
    /* Calculate checksum from all TLS values */
    checksum = 0;
    for (i = 0; i < 10; i++) {
        checksum += tls_values[i];
        checksum += (uintptr_t)tls_addresses[i] & 0xFF;
    }
    
    /* Add direct TLS variable usage */
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_common;
    checksum += tls_public;
    checksum += tls_preserved;
    checksum += tls_init;
    checksum += tls_sectioned;
    
    printf("TLS addresses and values:\n");
    for (i = 0; i < 10; i++) {
        if (tls_addresses[i] != 0) {
            printf("  TLS var %d: addr=%p, value=%d\n", 
                   i, (void*)tls_addresses[i], tls_values[i]);
        }
    }
    
    printf("\nChecksum: %d\n", checksum);
    printf("tls_external = %d\n", tls_external);
    
    /* Force reference to all TLS variables one more time */
    volatile int* ptrs[] = {
        &tls_weak, &tls_hidden, &tls_internal, &tls_protected,
        &tls_common, &tls_public, &tls_preserved, &tls_init,
        &tls_sectioned, &tls_external
    };
    
    printf("\nTLS variable addresses confirmed\n");
    
    return checksum != 0 ? 0 : 1;
}

/* Additional file for multi-translation unit testing (compile separately) */
#ifdef COMPILE_SECOND_FILE
/* second-file.c - for external/DLL import testing */
__thread int tls_external = 12345;
__thread int tls_imported = 6789;
#endif
