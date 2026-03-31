/* tls_emutls_test.c - Test program for GCC emulated TLS coverage */

/* Force emulated TLS mode if supported by compiler */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#pragma GCC tls_model emulated
#endif

/* Global volatile array to prevent optimization */
volatile void *tls_addresses[10];
volatile int tls_values[10];

/* Weak TLS variable - should trigger DECL_WEAK copying */
__thread int tls_weak __attribute__((weak));

/* TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* Common TLS variable - tentative definition at file scope, may set DECL_COMMON */
__thread int tls_common;

/* External TLS declaration - will be defined in another TU or later */
extern __thread int tls_external;

/* Preserved TLS variable - used attribute may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used));

/* Initialized TLS variable - ensures not optimized as BSS */
__thread int tls_init = 42;

/* DLL Import simulation for Windows targets */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#elif defined(__MINGW32__)
__declspec(dllimport) __thread int tls_imported;
#else
/* For non-Windows, we'll just declare it normally */
__thread int tls_imported;
#endif

/* Static TLS inside function - different DECL_CONTEXT */
static void func_with_static_tls(void) {
    static __thread int tls_static_func = 100;
    tls_addresses[6] = &tls_static_func;
    tls_values[6] = tls_static_func++;
}

/* Noinline function to ensure TLS variables are fully processed */
__attribute__((noinline, used)) 
static void process_tls_variables(void) {
    /* Take addresses of all TLS variables */
    tls_addresses[0] = &tls_weak;
    tls_addresses[1] = &tls_hidden;
    tls_addresses[2] = &tls_common;
    tls_addresses[3] = &tls_external;
    tls_addresses[4] = &tls_preserved;
    tls_addresses[5] = &tls_init;
    
    /* Use the values to prevent dead code elimination */
    tls_values[0] = tls_weak++;
    tls_values[1] = tls_hidden++;
    tls_values[2] = tls_common++;
    tls_values[3] = tls_external++;
    tls_values[4] = tls_preserved++;
    tls_values[5] = tls_init++;
    
    /* Process static function TLS */
    func_with_static_tls();
    
    /* DLL imported TLS if available */
    tls_addresses[7] = &tls_imported;
    tls_values[7] = tls_imported++;
}

/* Another noinline function that uses TLS in a loop */
__attribute__((noinline, used))
static void modify_tls_in_loop(void) {
    for (int i = 0; i < 10; i++) {
        tls_weak += i;
        tls_hidden -= i;
        tls_common *= (i + 1);
        tls_preserved ^= i;
        
        /* Force compiler to keep all operations */
        asm volatile("" : : "r"(tls_weak), "r"(tls_hidden), 
                      "r"(tls_common), "r"(tls_preserved));
    }
}

/* Check if we're using emulated TLS at runtime */
static void check_tls_mode(void) {
    /* This comparison forces taking addresses of TLS variables */
    if (&tls_weak != &tls_hidden) {
        /* Different addresses - good */
        tls_addresses[8] = (void*)((long)&tls_weak - (long)&tls_hidden);
    }
    
    /* Check if TLS is working by modifying and reading back */
    int old_value = tls_init;
    tls_init = 0xABCD;
    if (tls_init == 0xABCD) {
        tls_values[9] = 1; /* TLS is working */
    }
    tls_init = old_value;
}

int main(void) {
    /* Initialize some TLS variables */
    tls_hidden = 1;
    tls_common = 2;
    tls_preserved = 3;
    
    /* Process all TLS variables */
    process_tls_variables();
    
    /* Modify TLS in a loop */
    modify_tls_in_loop();
    
    /* Check TLS mode */
    check_tls_mode();
    
    /* Compute a checksum from TLS values to ensure they're used */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum ^= (int)(long)tls_addresses[i];
        checksum += tls_values[i];
    }
    
    /* Use checksum in a way that can't be optimized away */
    volatile int result = checksum;
    
    /* Return something based on TLS usage */
    return (result > 0) ? 0 : 1;
}

/* Define the external TLS variable to satisfy linker */
__thread int tls_external = 99;

/* For Windows, we'd normally have this in a separate DLL */
#ifdef _WIN32
__declspec(dllexport) __thread int tls_imported = 77;
#elif defined(__MINGW32__)
__declspec(dllexport) __thread int tls_imported = 77;
#else
/* For non-Windows, just define it here */
__thread int tls_imported = 77;
#endif
