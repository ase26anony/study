/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern int opaque_int(void);
extern void escape_ptr(void **p);

/* TLS variables with varied attributes */

/* Public TLS with default visibility */
__thread int public_tls = 42;
__thread int public_tls_uninit;

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* Protected visibility TLS */
__thread int protected_tls __attribute__((visibility("protected"))) = 300;

/* Internal visibility TLS */
__thread int internal_tls __attribute__((visibility("internal"))) = 400;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with attribute if supported */
__thread int imported_tls __attribute__((dllimport));
#endif

/* Static TLS (not public) */
static __thread int static_tls = 500;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 600;

/* Volatile TLS to prevent optimizations */
volatile __thread int volatile_tls = 700;

/* TLS with complex initializer */
extern int get_random(void);
__thread int dynamic_init_tls = 0;

/* External TLS declarations (simulating other compilation units) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* Function that uses TLS address - may be inlined */
static inline void modify_tls_via_ptr(int *ptr, int val) {
    /* This inline function accessing TLS might trigger declaration copying */
    *ptr += val;
    /* Take address and escape it */
    escape_ptr((void **)ptr);
}

/* Another function that heavily uses TLS */
static int compute_tls_sum(void) {
    int sum = 0;
    
    /* Access all TLS variables in a way that prevents dead code elimination */
    sum += public_tls;
    sum += public_tls_uninit;
    sum += weak_tls;
    sum += hidden_tls;
    sum += protected_tls;
    sum += internal_tls;
    sum += imported_tls;
    sum += static_tls;
    sum += aligned_tls;
    sum += volatile_tls;
    sum += dynamic_init_tls;
    sum += external_tls;
    sum += external_weak_tls;
    
    return sum;
}

/* Function that takes address of TLS variables */
static void capture_tls_addresses(void **ptrs, int size) {
    int i = 0;
    
    /* Store addresses of all TLS variables */
    if (i < size) ptrs[i++] = &public_tls;
    if (i < size) ptrs[i++] = &public_tls_uninit;
    if (i < size) ptrs[i++] = &weak_tls;
    if (i < size) ptrs[i++] = &hidden_tls;
    if (i < size) ptrs[i++] = &protected_tls;
    if (i < size) ptrs[i++] = &internal_tls;
    if (i < size) ptrs[i++] = (void *)&imported_tls;
    if (i < size) ptrs[i++] = &static_tls;
    if (i < size) ptrs[i++] = &aligned_tls;
    if (i < size) ptrs[i++] = (void *)&volatile_tls;
    if (i < size) ptrs[i++] = &dynamic_init_tls;
    if (i < size) ptrs[i++] = &external_tls;
    if (i < size) ptrs[i++] = &external_weak_tls;
    
    /* Pass addresses to opaque function */
    for (int j = 0; j < i; j++) {
        use_ptr(ptrs[j]);
    }
}

/* Main function with unpredictable control flow */
int main(int argc, char **argv) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    /* Initialize dynamic TLS with runtime value */
    dynamic_init_tls = seed % 1000;
    
    /* Array to store TLS addresses */
    void *tls_addresses[20];
    
    /* Capture addresses early */
    capture_tls_addresses(tls_addresses, 20);
    
    /* Modify TLS variables based on seed */
    int mod = seed % 7;
    
    /* Complex access pattern to force TLS emulation */
    for (int i = 0; i < 100; i++) {
        /* Use switch with TLS variables to create complex control flow */
        switch ((i + seed) % 10) {
            case 0:
                modify_tls_via_ptr(&public_tls, i);
                break;
            case 1:
                modify_tls_via_ptr(&hidden_tls, i * 2);
                break;
            case 2:
                modify_tls_via_ptr(&protected_tls, i * 3);
                break;
            case 3:
                /* Access via volatile pointer */
                *(volatile int *)&volatile_tls += i;
                break;
            case 4:
                /* Use aligned TLS */
                aligned_tls = (aligned_tls + i) & 0xFF;
                break;
            case 5:
                /* Weak TLS access */
                if (&weak_tls != NULL) {
                    weak_tls -= i;
                }
                break;
            case 6:
                /* Static TLS */
                static_tls = (static_tls * 17 + i) % 1000;
                break;
            case 7:
                /* External TLS */
                external_tls = (seed + i) % 256;
                break;
            case 8:
                /* Internal visibility TLS */
                internal_tls ^= i;
                break;
            case 9:
                /* DLL imported TLS simulation */
                imported_tls = i * i;
                break;
        }
        
        /* Occasionally take address and escape it */
        if (i % 13 == 0) {
            void *escape_addr;
            switch (i % 5) {
                case 0: escape_addr = &public_tls; break;
                case 1: escape_addr = &hidden_tls; break;
                case 2: escape_addr = &static_tls; break;
                case 3: escape_addr = &aligned_tls; break;
                case 4: escape_addr = &volatile_tls; break;
            }
            escape_ptr(&escape_addr);
        }
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = compute_tls_sum();
    
    /* Use checksum in output to prevent dead code elimination */
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    /* Additional unpredictable TLS access */
    if (checksum % 2 == 0) {
        /* Create pointer chain with TLS addresses */
        void *ptr_chain[5];
        ptr_chain[0] = &public_tls;
        ptr_chain[1] = &hidden_tls;
        ptr_chain[2] = &protected_tls;
        ptr_chain[3] = &static_tls;
        ptr_chain[4] = &external_tls;
        
        for (int i = 0; i < 5; i++) {
            use_ptr(ptr_chain[i]);
        }
    }
    
    return checksum % 256;
}

/* External TLS definitions (simulating another compilation unit) */
__thread int external_tls = 800;
__thread int external_weak_tls __attribute__((weak)) = 900;

/* Dummy definitions for opaque functions (for linking) */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void escape_ptr(void **p) {
    /* Simulate pointer escape */
    static volatile void *escaped_ptr = NULL;
    escaped_ptr = *p;
}

int get_random(void) {
    return 42; /* Not actually random, but opaque to compiler */
}
