/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* TLS variables with diverse attributes */

/* Weak TLS variable with default visibility */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* Hidden visibility TLS */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 100;

/* Protected visibility TLS */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 200;

/* Common linkage (tentative definition) */
__thread int tls_common_var;  /* Should get common linkage */

/* External TLS declaration (will be defined elsewhere) */
extern __thread int tls_external_var;

/* DLL import simulation (using visibility attributes) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* On non-Windows, use visibility to simulate similar behavior */
    __thread int tls_dllimport_var __attribute__((visibility("default")));
#endif

/* Static TLS inside a function context */
static void function_with_static_tls(void) {
    static __thread int tls_static_func = 999;
    volatile int* volatile ptr = &tls_static_func;
    *ptr += 1;  /* Force access through volatile pointer */
}

/* Block-scoped TLS (C++ only in real code, simulated here) */
void block_scoped_tls_access(void) {
    /* In C++: thread_local int tls_block = 555; */
    /* For C, we'll use a static function TLS */
    static __thread int tls_block_simulated = 555;
    volatile int* vptr = &tls_block_simulated;
    *vptr = 556;
}

/* Noinline helper functions that take addresses of TLS variables */

NOINLINE static void use_tls_weak(int* out) {
    volatile int* ptr = &tls_weak_var;
    *out = *ptr;
    *ptr += 1;  /* Modify through volatile pointer */
}

NOINLINE static void use_tls_hidden(int* out) {
    volatile int* ptr = &tls_hidden_var;
    *out = *ptr;
    *ptr += 2;
}

NOINLINE static void use_tls_protected(int* out) {
    volatile int* ptr = &tls_protected_var;
    *out = *ptr;
    *ptr += 3;
}

NOINLINE static void use_tls_common(int* out) {
    volatile int* ptr = &tls_common_var;
    *out = *ptr;
    *ptr += 4;
}

NOINLINE static void use_tls_external(int* out) {
    /* External TLS - will be defined below */
    volatile int* ptr = &tls_external_var;
    *out = *ptr;
    *ptr += 5;
}

NOINLINE static void use_tls_dllimport(int* out) {
    volatile int* ptr = &tls_dllimport_var;
    *out = *ptr;
    *ptr += 6;
}

/* Constructor that interacts with TLS */
CONSTRUCTOR static void tls_constructor(void) {
    /* Initialize and use TLS in constructor */
    tls_hidden_var = 1234;
    tls_protected_var = 5678;
    
    /* Force TLS address taking in constructor */
    volatile int* ptr1 = &tls_hidden_var;
    volatile int* ptr2 = &tls_protected_var;
    (void)ptr1;
    (void)ptr2;
}

/* Destructor that also uses TLS */
DESTRUCTOR static void tls_destructor(void) {
    /* Access TLS in destructor */
    volatile int dummy = tls_weak_var + tls_hidden_var;
    (void)dummy;
}

/* Global variable to prevent optimization */
volatile int global_volatile_counter = 0;

/* Main test function */
int main(void) {
    int results[6] = {0};
    volatile int selector = 0;
    
    /* Define the external TLS variable */
    __thread int tls_external_var = 300;
    
    /* Define the DLL import simulated variable */
    __thread int tls_dllimport_var = 400;
    
    /* Access function-static TLS */
    function_with_static_tls();
    block_scoped_tls_access();
    
    /* Loop with volatile selector to force multiple control paths */
    for (int i = 0; i < 10; i++) {
        selector = global_volatile_counter + i;
        
        /* Conditional TLS access based on volatile selector */
        if (selector % 2 == 0) {
            use_tls_weak(&results[0]);
        }
        if (selector % 3 == 0) {
            use_tls_hidden(&results[1]);
        }
        if (selector % 5 == 0) {
            use_tls_protected(&results[2]);
        }
        if (selector % 7 == 0) {
            use_tls_common(&results[3]);
        }
        
        /* Always access these to ensure they're used */
        use_tls_external(&results[4]);
        use_tls_dllimport(&results[5]);
        
        global_volatile_counter++;
    }
    
    /* Compute checksum of TLS values to prevent elimination */
    int checksum = 0;
    checksum += tls_weak_var;
    checksum += tls_hidden_var;
    checksum += tls_protected_var;
    checksum += tls_common_var;
    checksum += tls_external_var;
    checksum += tls_dllimport_var;
    
    /* Also compute via volatile pointers */
    volatile int* volatile ptrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_external_var,
        &tls_dllimport_var
    };
    
    int volatile_checksum = 0;
    for (int i = 0; i < 6; i++) {
        volatile_checksum += *ptrs[i];
    }
    
    /* Return the checksums to prevent dead code elimination */
    return (checksum + volatile_checksum) % 255;
}

/* Additional TLS variable in different compilation unit context */
__thread int tls_another_external __attribute__((weak, visibility("default")));

/* Force usage of all TLS variables in a noinline function */
NOINLINE void force_tls_usage(void) {
    /* Take addresses of all TLS variables */
    void* addrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_external_var,
        &tls_dllimport_var,
        &tls_another_external
    };
    
    /* Volatile store to prevent optimization */
    volatile void* volatile store_addr = addrs[0];
    (void)store_addr;
}
