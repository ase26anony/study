/* test_emutls.c - Comprehensive TLS emulation test */

/* Helper function to force address usage */
__attribute__((noinline))
static void use_tls_address(void *addr) {
    volatile static int sink;
    sink = *(int*)addr;  /* Dummy read to prevent optimization */
}

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'X';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 200;
__thread long tls_protected __attribute__((visibility("protected"))) = 300;

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 400;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported = 400;
#endif

/* Additional patterns for comprehensive coverage */

/* TLS variable with common linkage (tentative definition) */
__thread int tls_common;  /* DECL_COMMON should be set */

/* TLS variable used in multiple functions */
__thread double tls_shared = 3.14159;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 999;

/* Function that uses TLS variables extensively */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    int result = 0;
    
    /* Use Pattern A */
    tls_static_init = argc * 2;
    result += tls_static_init;
    
    /* Use Pattern B */
    tls_extern = 'A' + (argc % 26);
    result += tls_extern;
    
    /* Use Pattern C - only use if defined (weak) */
    if (&tls_weak != NULL) {
        tls_weak = argc * 3;
        result += tls_weak;
    }
    
    /* Use Pattern D */
    tls_hidden = argc * 4L;
    tls_protected = argc * 5L;
    result += (int)(tls_hidden + tls_protected);
    
    /* Use Pattern E */
    tls_imported = argc * 6;
    result += tls_imported;
    
    /* Use common TLS */
    tls_common = argc * 7;
    result += tls_common;
    
    /* Use shared TLS */
    tls_shared = argc * 8.0;
    result += (int)tls_shared;
    
    /* Use volatile TLS */
    tls_volatile = argc * 9;
    result += tls_volatile;
    
    return result;
}

/* Another function to ensure TLS addresses are taken in different contexts */
__attribute__((noinline))
static void take_tls_addresses(void) {
    /* Take addresses of all TLS variables */
    use_tls_address(&tls_static_init);
    use_tls_address((void*)&tls_extern);
    use_tls_address(&tls_weak);
    use_tls_address(&tls_hidden);
    use_tls_address(&tls_protected);
    use_tls_address(&tls_imported);
    use_tls_address(&tls_common);
    use_tls_address((void*)&tls_shared);
    use_tls_address((void*)&tls_volatile);
}

/* Main function with control flow dependencies */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Ensure TLS variables are marked as used */
    TREE_USED(&tls_static_init);  /* This is a conceptual marker */
    
    /* Complex control flow to prevent optimization */
    for (int i = 0; i < argc; i++) {
        if (i % 2 == 0) {
            result += compute_with_tls(argc + i);
        } else {
            take_tls_addresses();
        }
        
        /* Modify TLS variables in loop */
        tls_static_init += i;
        tls_common -= i;
    }
    
    /* Final computation using all TLS variables */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += (int)(tls_hidden + tls_protected);
    result += tls_imported;
    result += tls_common;
    result += (int)tls_shared;
    result += tls_volatile;
    
    /* One more address taking */
    take_tls_addresses();
    
    /* Return value depends on all TLS variables and argc */
    return result % 256;  /* Prevent large return values */
}

/* Additional file to test external linkage (compile separately if needed) */
#ifdef MULTI_FILE_TEST
/* In a separate file: tls_extern_def.c */
__thread char tls_extern = 'X';

/* In another file: tls_weak_def.c */
__thread int tls_weak __attribute__((weak)) = 100;
#endif
