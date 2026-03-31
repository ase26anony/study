/* test_emutls.c - Comprehensive TLS emulation test */
/* Compile with: gcc -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls */

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported __attribute__((visibility("default"))) = 5000;
#endif

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 99;
__thread const int tls_const = 12345;

/* Helper function to ensure addresses are taken and used */
__attribute__((noinline)) 
static void use_tls_pointers(void *p1, void *p2, void *p3, void *p4, void *p5, void *p6) {
    /* Dummy writes to prevent optimization */
    volatile static int sink;
    sink = (int)(long)p1;
    sink = (int)(long)p2;
    sink = (int)(long)p3;
    sink = (int)(long)p4;
    sink = (int)(long)p5;
    sink = (int)(long)p6;
}

/* Another helper to use TLS values */
__attribute__((noinline))
static int compute_with_tls(int base) {
    int result = base;
    
    /* Use all TLS variables in computations */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 100;
    result += tls_protected % 100;
    result += tls_imported % 100;
    result += tls_volatile;
    result += tls_const % 100;
    
    return result;
}

/* Function that forces TLS variable usage in different scopes */
static void modify_tls_vars(int argc) {
    /* Modify static initialized TLS */
    tls_static_init = argc * 2;
    
    /* Modify extern TLS */
    tls_extern = 'A' + (argc % 26);
    
    /* Modify weak TLS if it exists */
    if (&tls_weak) {
        tls_weak = argc * 3;
    }
    
    /* Modify visibility TLS variables */
    tls_hidden = 1000 + argc;
    tls_protected = 2000 + argc * 2;
    
    /* Modify imported TLS */
    tls_imported = 5000 + argc * 5;
    
    /* Modify volatile TLS */
    tls_volatile = argc;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Step 1: Initial TLS usage */
    result = compute_with_tls(argc);
    
    /* Step 2: Modify TLS variables */
    modify_tls_vars(argc);
    
    /* Step 3: Compute again with modified values */
    result += compute_with_tls(argc);
    
    /* Step 4: Take addresses of all TLS variables to force emulation */
    use_tls_pointers(
        (void*)&tls_static_init,
        (void*)&tls_extern,
        (void*)&tls_weak,
        (void*)&tls_hidden,
        (void*)&tls_protected,
        (void*)&tls_imported
    );
    
    /* Additional address taking for remaining TLS variables */
    volatile __thread int *volatile_ptr = &tls_volatile;
    volatile const __thread int *const_ptr = &tls_const;
    
    /* Use pointers to prevent optimization */
    if (volatile_ptr && const_ptr) {
        result += *volatile_ptr;
        result += *const_ptr;
    }
    
    /* Complex control flow with TLS dependencies */
    for (int i = 0; i < argc; i++) {
        tls_static_init += i;
        tls_volatile -= i;
        
        if (i % 2 == 0) {
            tls_hidden += tls_protected;
        } else {
            tls_protected += tls_hidden;
        }
    }
    
    /* Final computation */
    result += compute_with_tls(argc);
    
    /* Return value depends on all TLS operations */
    return result % 256;
}

/* Additional file-scope TLS usage to ensure various contexts */
static void __attribute__((constructor)) init_tls(void) {
    /* Constructor runs before main, uses TLS */
    tls_static_init += 1;
    tls_volatile = 999;
}

static void __attribute__((destructor)) cleanup_tls(void) {
    /* Destructor runs after main, uses TLS */
    volatile int dummy = tls_static_init + tls_volatile;
    (void)dummy;
}
