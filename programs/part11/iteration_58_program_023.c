/* test-emutls-coverage.c
 * This program tests GCC's emulated TLS initialization by creating
 * thread-local variables with diverse attributes that trigger the
 * property copying in emutls_decl (lines 295-304 in tree-emutls.cc)
 */

/* Force emulated TLS mode if supported by compiler */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#pragma GCC tls_model emulated
#endif

/* For Windows DLL import/export attributes */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT
#define DLL_EXPORT
#endif

/* Prevent optimization of TLS variable usage */
#define USE_VAR(x) do { \
    volatile void *volatile __ptr = (volatile void*)&(x); \
    (void)__ptr; \
} while(0)

/* Helper to force preservation */
__attribute__((noinline, used))
static void use_tls_vars(void);

/* ========== TLS VARIABLES WITH VARIOUS ATTRIBUTES ========== */

/* 1. Weak TLS variable - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* 3. Common TLS variable (tentative definition) - may trigger DECL_COMMON */
__thread int tls_common;

/* 4. Public TLS variable with external linkage */
__thread int tls_public = 42;  /* Initialized to ensure not BSS */

/* 5. External TLS declaration (will be defined elsewhere or remain external) */
extern __thread int tls_external;  /* Triggers DECL_EXTERNAL */

/* 6. DLL Import TLS (for Windows targets) */
DLL_IMPORT __thread int tls_imported;

/* 7. TLS variable that should be preserved */
__thread int tls_preserved __attribute__((used));

/* 8. TLS with protected visibility */
__thread int tls_protected __attribute__((visibility("protected")));

/* 9. TLS in different contexts to affect DECL_CONTEXT */
static void function_with_tls(void) {
    /* TLS with function scope context */
    static __thread int tls_function_scope = 100;
    USE_VAR(tls_function_scope);
}

/* 10. TLS with noinline attribute to influence preservation */
__thread int tls_noinline __attribute__((noinline));

/* ========== FUNCTION THAT USES ALL TLS VARIABLES ========== */

/* This function must be noinline to ensure TLS variables are fully processed */
__attribute__((noinline, used))
static void use_tls_vars(void) {
    /* Take addresses and use all TLS variables */
    volatile int *volatile ptrs[10];
    volatile int volatile values[10];
    
    /* Take addresses (forces TLS instantiation) */
    ptrs[0] = &tls_weak;
    ptrs[1] = &tls_hidden;
    ptrs[2] = &tls_common;
    ptrs[3] = &tls_public;
    ptrs[4] = &tls_external;  /* External reference */
    ptrs[5] = &tls_imported;  /* DLL import reference */
    ptrs[6] = &tls_preserved;
    ptrs[7] = &tls_protected;
    ptrs[8] = &tls_noinline;
    
    /* Use values */
    values[0] = tls_weak;
    values[1] = tls_hidden;
    values[2] = tls_common;
    values[3] = tls_public;
    values[4] = tls_external;  /* May be zero if undefined */
    values[5] = tls_imported;  /* May be zero if undefined */
    values[6] = tls_preserved;
    values[7] = tls_protected;
    values[8] = tls_noinline;
    
    /* Simple computation to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 9; i++) {
        sum += values[i];
        sum += (int)((uintptr_t)ptrs[i] & 0xFF);
    }
    
    /* Store result in a global volatile to prevent optimization */
    static volatile int result;
    result = sum;
    
    /* Also use function-scope TLS */
    function_with_tls();
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Initialize some TLS variables */
    tls_weak = 1;
    tls_hidden = 2;
    tls_common = 3;
    tls_public = 4;  /* Already initialized, but reassign */
    tls_preserved = 7;
    tls_protected = 8;
    tls_noinline = 9;
    
    /* Use all TLS variables through helper function */
    use_tls_vars();
    
    /* Force reference to all TLS variables in main as well */
    volatile int checksum = 0;
    
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_common;
    checksum += tls_public;
    checksum += tls_preserved;
    checksum += tls_protected;
    checksum += tls_noinline;
    
    /* Check that TLS addresses are different (forces referencing) */
    if (&tls_weak != &tls_hidden) {
        checksum += 1;
    }
    if (&tls_public != &tls_common) {
        checksum += 2;
    }
    
    /* Use function-scope TLS indirectly */
    function_with_tls();
    
    /* Return checksum to prevent optimization */
    return checksum & 0xFF;
}

/* ========== SECOND FILE FOR EXTERNAL TLS ========== */
/* 
 * Compile this separately and link with the main program:
 *   gcc -c -O2 -ftls-model=emulated tls-external-def.c -o tls-external-def.o
 *   gcc -O2 -ftls-model=emulated test-emutls-coverage.c tls-external-def.o -o test
 */

#ifdef COMPILE_EXTERNAL_DEF
/* tls-external-def.c */
DLL_EXPORT __thread int tls_external = 123;
DLL_EXPORT __thread int tls_imported = 456;
#endif
