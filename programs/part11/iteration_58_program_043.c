/* tls_emutls_test.c
 * Test program to cover GCC's emulated TLS initialization code
 * Specifically targeting lines 295-304 in tree-emutls.cc
 */

/* Force emulated TLS mode if supported */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#pragma GCC tls_model emulated
#endif

/* Prevent optimization of TLS variables */
#define KEEP_ALIVE(var) do { \
    volatile void *__ptr = &(var); \
    __asm__ __volatile__("" : : "r"(__ptr) : "memory"); \
} while(0)

/* Helper to ensure functions aren't inlined */
#define NOINLINE __attribute__((noinline, used))

/* ========== TLS VARIABLES WITH DIFFERENT ATTRIBUTES ========== */

/* 1. Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 1;

/* 2. TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 2;

/* 3. Common TLS variable - tentative definition (no initializer at file scope) */
__thread int tls_common;  /* Should trigger DECL_COMMON */

/* 4. External/Public TLS declaration */
extern __thread int tls_external;  /* DECL_EXTERNAL and TREE_PUBLIC */

/* 5. DLL Import TLS (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__declspec(dllimport) __thread int tls_imported;
#else
/* On non-Windows, use a regular TLS variable */
__thread int tls_imported __attribute__((visibility("default"))) = 5;
#endif

/* 6. Preserved TLS variable - use multiple attributes to influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used, visibility("protected"))) = 6;

/* 7. Static TLS variable (non-public) to test different DECL_CONTEXT scenarios */
static __thread int tls_static = 7;

/* 8. TLS variable with section attribute */
__thread int tls_sectioned __attribute__((section(".tls_test"))) = 8;

/* ========== FUNCTION TO USE TLS VARIABLES ========== */

/* Global array to store results and prevent optimization */
volatile int g_results[16] = {0};

NOINLINE static void use_tls_variables(void) {
    /* Take addresses of all TLS variables */
    volatile int *ptrs[] = {
        &tls_weak,
        &tls_hidden,
        &tls_common,
        &tls_imported,
        &tls_preserved,
        &tls_static,
        &tls_sectioned
    };
    
    /* Use each TLS variable in computations */
    int sum = 0;
    
    /* Initialize tls_common if it's zero */
    if (tls_common == 0) {
        tls_common = 3;
    }
    
    /* For external TLS, try to use it (will be linked from another file) */
    #ifndef NO_EXTERNAL_TLS
    if (&tls_external != NULL) {
        sum += tls_external;
    }
    #endif
    
    /* Compute checksum using all TLS variables */
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_common;
    sum += tls_imported;
    sum += tls_preserved;
    sum += tls_static;
    sum += tls_sectioned;
    
    /* Store results in global volatile array */
    g_results[0] = sum;
    g_results[1] = (int)(long)&tls_weak;
    g_results[2] = (int)(long)&tls_hidden;
    g_results[3] = (int)(long)&tls_common;
    
    /* Force compiler to keep all TLS variables alive */
    KEEP_ALIVE(tls_weak);
    KEEP_ALIVE(tls_hidden);
    KEEP_ALIVE(tls_common);
    KEEP_ALIVE(tls_imported);
    KEEP_ALIVE(tls_preserved);
    KEEP_ALIVE(tls_static);
    KEEP_ALIVE(tls_sectioned);
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Call function that uses TLS variables */
    use_tls_variables();
    
    /* Force referencing of variables in main too */
    volatile int diff = (&tls_weak != (void*)&tls_hidden);
    
    /* Use the results to prevent dead code elimination */
    int result = g_results[0] + diff;
    
    #ifdef __HAVE_TLS
    /* If we have native TLS, print a message */
    printf("Native TLS supported, result: %d\n", result);
    #else
    printf("Emulated TLS likely, result: %d\n", result);
    #endif
    
    /* Return something based on TLS usage */
    return result > 20 ? 0 : 1;
}

/* ========== ADDITIONAL CONTEXTS FOR TLS VARIABLES ========== */

/* TLS variable inside a function (different DECL_CONTEXT) */
NOINLINE static void function_with_tls(void) {
    static __thread int tls_in_function = 42;
    KEEP_ALIVE(tls_in_function);
    g_results[4] = tls_in_function;
}

/* Call the function to ensure its TLS is processed */
__attribute__((constructor)) static void init_tls_contexts(void) {
    function_with_tls();
}
