/* test_emutls.c - Test program for GCC TLS emulation attribute copying */

/* Force emulated TLS model if supported */
#ifdef __GNUC__
#pragma GCC tls_model emulated
#endif

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000L;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000L;

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
#else
/* Simulate with weak external on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif

/* Provide definition for tls_imported */
__thread int tls_imported = 500;

/* Additional TLS variables to ensure variety */
__thread volatile int tls_volatile = 999;
__thread const int tls_const = 1234;
__thread int tls_zero_init;  /* Should be zero-initialized */

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    /* Dummy writes to prevent optimization */
    volatile static int sink = 0;
    sink += (int)((long)addr1 ^ (long)addr2 ^ (long)addr3 ^ 
                  (long)addr4 ^ (long)addr5 ^ (long)addr6);
}

/* Another helper to use TLS values */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computations */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 100;
    result += tls_protected % 100;
    result += tls_imported;
    result += tls_volatile;
    result += tls_const;
    result += tls_zero_init;
    
    /* Make result dependent on argc */
    return result * (argc + 1);
}

int main(int argc, char **argv) {
    int i, sum = 0;
    
    /* Modify TLS variables based on argc */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = 1000L + argc;
    tls_protected = 2000L + argc * 2;
    tls_imported = 500 + argc * 5;
    tls_volatile = 999 + argc;
    /* tls_const is const, can't modify */
    tls_zero_init = argc * 3;
    
    /* Use TLS variables in loop with control flow */
    for (i = 0; i < argc; i++) {
        if (i % 2 == 0) {
            sum += tls_static_init;
            tls_weak++;
        } else {
            sum += tls_extern;
            tls_imported--;
        }
        
        /* Conditional TLS usage */
        sum += (tls_hidden > 1000) ? 1 : 0;
        sum += (tls_protected < 3000) ? 1 : 0;
    }
    
    /* Force address taking of all TLS variables */
    use_tls_addresses(&tls_static_init, &tls_extern, &tls_weak,
                     &tls_hidden, &tls_protected, &tls_imported);
    
    /* Use another helper that reads TLS values */
    sum += compute_with_tls(argc);
    
    /* Return value depends on TLS variables and argc */
    return sum % 255;
}

/* Additional file-like separation using weak symbols */
/* This tests DECL_COMMON and DECL_EXTERNAL behaviors */
__thread int tls_tentative;  /* Tentative definition - tests common */

/* External reference to force external linkage handling */
extern __thread int tls_purely_external;
/* Don't provide definition to test DECL_EXTERNAL handling */
