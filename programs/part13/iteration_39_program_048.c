/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))
#define WEAK __attribute__((weak))
#define HIDDEN __attribute__((visibility("hidden")))
#define PROTECTED __attribute__((visibility("protected")))
#define DLLEXPORT __attribute__((dllexport))
#define DLLIMPORT __attribute__((dllimport))

/* Global TLS variables with various attributes */

/* 1. Regular TLS with default visibility */
__thread int tls_regular = 42;

/* 2. Weak TLS symbol */
__thread int tls_weak WEAK = 100;

/* 3. Hidden visibility TLS */
__thread int tls_hidden HIDDEN = 200;

/* 4. Protected visibility TLS */
__thread int tls_protected PROTECTED = 300;

/* 5. Common linkage (tentative definition) */
__thread int tls_common;

/* 6. External declaration (defined elsewhere) */
extern __thread int tls_external;

/* 7. DLL import style (simulated with weak) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport WEAK;
#endif

/* 8. Static TLS inside function will be tested separately */
/* 9. Preserve flag test - used in constructor */

/* Function-local static TLS */
static void test_local_tls(void) {
    static __thread int tls_local_static = 999;
    volatile int* volatile ptr = &tls_local_static;
    *ptr += 1;
}

/* Non-inlineable helper functions to force TLS usage */

NOINLINE static void use_tls_regular(volatile int* out) {
    volatile int* ptr = &tls_regular;
    *out += *ptr;
    tls_regular++;
}

NOINLINE static void use_tls_weak(volatile int* out) {
    volatile int* ptr = &tls_weak;
    *out += *ptr;
    tls_weak += 2;
}

NOINLINE static void use_tls_hidden(volatile int* out) {
    volatile int* ptr = &tls_hidden;
    *out += *ptr;
    tls_hidden += 3;
}

NOINLINE static void use_tls_protected(volatile int* out) {
    volatile int* ptr = &tls_protected;
    *out += *ptr;
    tls_protected += 4;
}

NOINLINE static void use_tls_common(volatile int* out) {
    volatile int* ptr = &tls_common;
    *out += *ptr;
    tls_common += 5;
}

NOINLINE static void use_tls_external(volatile int* out) {
    /* External TLS - will be defined in main */
    volatile int* ptr = &tls_external;
    *out += *ptr;
}

NOINLINE static void use_tls_dllimport(volatile int* out) {
    volatile int* ptr = &tls_dllimport;
    *out += *ptr;
}

/* Constructor that uses TLS */
CONSTRUCTOR static void init_tls_values(void) {
    /* This should trigger DECL_PRESERVE_P propagation */
    tls_regular = 1;
    tls_common = 1000;
    
    /* Take address to force usage */
    volatile int* ptr1 = &tls_regular;
    volatile int* ptr2 = &tls_common;
    (void)ptr1;
    (void)ptr2;
}

/* Destructor that also uses TLS */
DESTRUCTOR static void cleanup_tls(void) {
    volatile int dummy = tls_regular + tls_hidden;
    (void)dummy;
}

/* Main test function */
int main(void) {
    volatile int checksum = 0;
    volatile int selector = 0;
    
    /* Define the external TLS variable */
    __thread int tls_external = 777;
    
    /* Define the DLL import style variable */
    __thread int tls_dllimport = 888;
    
    /* Test function-local static TLS */
    test_local_tls();
    
    /* Loop with volatile selector to prevent optimization */
    for (selector = 0; selector < 7; selector++) {
        volatile int* out = &checksum;
        
        switch (selector) {
            case 0:
                use_tls_regular(out);
                break;
            case 1:
                use_tls_weak(out);
                break;
            case 2:
                use_tls_hidden(out);
                break;
            case 3:
                use_tls_protected(out);
                break;
            case 4:
                use_tls_common(out);
                break;
            case 5:
                use_tls_external(out);
                break;
            case 6:
                use_tls_dllimport(out);
                break;
        }
        
        /* Force compiler to keep all variables alive */
        volatile int* addrs[] = {
            &tls_regular,
            &tls_weak,
            &tls_hidden,
            &tls_protected,
            &tls_common,
            &tls_external,
            &tls_dllimport
        };
        
        /* Access all addresses to prevent DCE */
        for (int i = 0; i < 7; i++) {
            checksum += (int)(long)addrs[i];
        }
    }
    
    /* Compute final checksum using all TLS values */
    checksum += tls_regular;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_protected;
    checksum += tls_common;
    checksum += tls_external;
    checksum += tls_dllimport;
    
    /* Print to prevent optimization */
    printf("TLS checksum: %d\n", checksum);
    
    /* Verify emulated TLS is being used */
    #ifdef __EMUTLS__
    printf("Using emulated TLS\n");
    #endif
    
    return checksum != 0 ? 0 : 1;
}
