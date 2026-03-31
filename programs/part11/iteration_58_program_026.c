/* test-emutls-attributes.c
 * 
 * This program creates thread-local variables with diverse attributes
 * to trigger the property copying logic in GCC's emutls_decl function.
 * Compile with: gcc -O2 -ftls-model=emulated -fno-builtin -pthread -m32 -o test test-emutls-attributes.c
 * Or for Windows: x86_64-w64-mingw32-gcc -O2 -ftls-model=emulated -D_WIN32 -o test.exe test-emutls-attributes.c
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimizations */
volatile void *volatile_escape = NULL;
volatile int volatile_result = 0;

/* Force emulated TLS by checking if native TLS is available */
#ifndef __HAVE_TLS
#define EMULATED_TLS_ENABLED 1
#else
/* Still try to force emulated mode via compilation flags */
#define EMULATED_TLS_ENABLED 1
#endif

/* ===== TLS VARIABLES WITH VARIOUS ATTRIBUTES ===== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;

/* 4. External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* 5. Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* 6. TLS with initialization - ensures not optimized as BSS */
__thread int tls_init = 42;

/* 7. Public TLS with specific section */
__thread int tls_public __attribute__((section(".tls_data")));

/* 8. DLL Import TLS (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, use weak external as proxy */
extern __thread int tls_imported __attribute__((weak));
#endif

/* 9. TLS with noinline function context */
static __attribute__((noinline)) void use_tls_in_function(void) {
    static __thread int tls_in_function = 100;
    tls_in_function++;
    volatile_escape = &tls_in_function;
}

/* 10. TLS in different linkage contexts */
static __thread int tls_static = 7;  /* static linkage */

/* ===== FUNCTION TO USE ALL TLS VARIABLES ===== */

/* This function uses all TLS variables to prevent dead code elimination */
__attribute__((noinline)) 
void use_all_tls_variables(void) {
    int sum = 0;
    
    /* Use weak TLS */
    tls_weak = 1;
    sum += tls_weak;
    
    /* Use hidden TLS */
    tls_hidden = 2;
    sum += tls_hidden;
    
    /* Use common TLS */
    tls_common = 3;
    sum += tls_common;
    
    /* Use external TLS (will be defined below) */
    sum += tls_external;
    
    /* Use preserved TLS */
    tls_preserved = 5;
    sum += tls_preserved;
    
    /* Use initialized TLS */
    sum += tls_init;
    
    /* Use public TLS */
    tls_public = 7;
    sum += tls_public;
    
    /* Use imported TLS */
#ifdef _WIN32
    /* On Windows, we might not have definition, so use carefully */
    if (&tls_imported != NULL) {
        sum += 8;
    }
#else
    /* On non-Windows, our weak external might be NULL */
    if (&tls_imported != NULL) {
        sum += 8;
    }
#endif
    
    /* Use function-scoped TLS */
    use_tls_in_function();
    
    /* Use static TLS */
    tls_static++;
    sum += tls_static;
    
    /* Store result to prevent optimization */
    volatile_result = sum;
    
    /* Take addresses of all TLS variables */
    volatile_escape = &tls_weak;
    volatile_escape = &tls_hidden;
    volatile_escape = &tls_common;
    volatile_escape = &tls_external;
    volatile_escape = &tls_preserved;
    volatile_escape = &tls_init;
    volatile_escape = &tls_public;
#ifdef _WIN32
    volatile_escape = &tls_imported;
#endif
    volatile_escape = &tls_static;
}

/* ===== EXTERNAL TLS DEFINITION ===== */
/* This satisfies the external declaration */
__thread int tls_external = 99;

/* For Windows DLL import simulation */
#ifdef _WIN32
/* In a real scenario, this would be in a separate DLL */
__declspec(dllexport) __thread int tls_imported = 88;
#endif

/* ===== MAIN FUNCTION ===== */

int main(void) {
    printf("Testing emulated TLS with various attributes...\n");
    
    /* Check if addresses differ (forces referencing) */
    if (&tls_weak != &tls_hidden) {
        printf("TLS addresses differ as expected\n");
    }
    
    /* Use all TLS variables */
    use_all_tls_variables();
    
    /* Print some values to ensure they're used */
    printf("tls_init = %d\n", tls_init);
    printf("tls_external = %d\n", tls_external);
    printf("tls_static = %d\n", tls_static);
    
    /* Force use of other variables */
    tls_hidden = volatile_result;
    tls_common = tls_hidden + 1;
    
    printf("volatile_result = %d\n", volatile_result);
    
    /* Check TLS model at runtime */
    printf("TLS variable addresses:\n");
    printf("  tls_weak: %p\n", (void*)&tls_weak);
    printf("  tls_hidden: %p\n", (void*)&tls_hidden);
    printf("  tls_common: %p\n", (void*)&tls_common);
    printf("  tls_external: %p\n", (void*)&tls_external);
    
    return 0;
}
