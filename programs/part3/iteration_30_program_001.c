/* test_tls_emulation.c - Designed to trigger declaration attribute copying
   in GCC's tree-emutls.cc lines 295-304 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque functions to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* Public TLS with external linkage */
__thread int public_tls = 42;
__thread int public_tls_uninit;

/* Weak TLS symbol */
__thread int weak_tls __attribute__((weak)) = 100;

/* TLS with hidden visibility */
__thread int hidden_tls __attribute__((visibility("hidden"))) = 200;

/* TLS with internal visibility */
__thread int internal_tls __attribute__((visibility("internal"))) = 300;

/* TLS with protected visibility */
__thread int protected_tls __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls;
#else
/* Simulate with attribute if supported */
__thread int dllimport_tls __attribute__((dllimport));
#endif

/* Common TLS symbol (tentative definition) */
__thread int common_tls;

/* Static TLS (file scope, internal linkage) */
static __thread int static_tls = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int volatile_tls = 600;

/* TLS with dynamic initialization */
__thread int dynamic_tls = 0;

/* TLS that might need preservation (address escapes) */
__thread int preserve_tls = 700;

/* TLS with alignment requirement */
__thread int aligned_tls __attribute__((aligned(64))) = 800;

/* External TLS declarations (simulating other compilation units) */
extern __thread int external_tls;
extern __thread int external_weak_tls __attribute__((weak));

/* ========== FUNCTIONS THAT WORK WITH TLS ========== */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    static __thread int inline_tls = 0;
    inline_tls += idx;
    return inline_tls + public_tls;
}

/* Function that takes address of TLS - forces address computation */
void take_tls_addresses(void) {
    void *addresses[] = {
        (void*)&public_tls,
        (void*)&weak_tls,
        (void*)&hidden_tls,
        (void*)&internal_tls,
        (void*)&protected_tls,
        (void*)&dllimport_tls,
        (void*)&common_tls,
        (void*)&static_tls,
        (void*)&volatile_tls,
        (void*)&preserve_tls,
        (void*)&aligned_tls,
        (void*)&external_tls,
        (void*)&external_weak_tls
    };
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < (int)(sizeof(addresses)/sizeof(addresses[0])); i++) {
        use_ptr(addresses[i]);
    }
}

/* Complex expression with TLS that might require proxy creation */
int* get_tls_pointer(int selector) {
    switch (selector % 5) {
        case 0: return &public_tls;
        case 1: return &weak_tls;
        case 2: return &hidden_tls;
        case 3: return &static_tls;
        case 4: return &preserve_tls;
        default: return &common_tls;
    }
}

/* Function using TLS in loops - may trigger optimizations */
void process_tls_in_loop(int iterations) {
    static __thread int loop_tls = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses */
        public_tls += i;
        loop_tls ^= public_tls;
        hidden_tls -= i;
        
        /* Use inline function */
        int val = inline_tls_access(i);
        protected_tls += val;
        
        /* Volatile access prevents dead code elimination */
        volatile int tmp = volatile_tls;
        volatile_tls = tmp + 1;
    }
}

/* Function where TLS address escapes - triggers DECL_PRESERVE_P */
void tls_address_escape(void) {
    /* Store TLS address in global-like location */
    static void *escaped_tls_ptr = NULL;
    escaped_tls_ptr = &preserve_tls;
    use_ptr(escaped_tls_ptr);
    
    /* Use in asm statement if supported */
    #ifdef __GNUC__
    asm volatile("" : : "r"(&preserve_tls) : "memory");
    #endif
}

/* ========== MAIN FUNCTION ========== */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = get_random();
    }
    
    srand(seed);
    
    /* Initialize dynamic TLS with function call */
    dynamic_tls = rand() % 1000;
    
    /* Initialize external TLS variables (simulating definitions) */
    extern __thread int external_tls;
    extern __thread int external_weak_tls __attribute__((weak));
    external_tls = 1234;
    external_weak_tls = 5678;
    
    /* Take addresses of all TLS variables */
    take_tls_addresses();
    
    /* Process TLS in loops with runtime-determined iterations */
    int iterations = (seed % 100) + 1;
    process_tls_in_loop(iterations);
    
    /* Force TLS address escape */
    tls_address_escape();
    
    /* Complex TLS pointer manipulation */
    for (int i = 0; i < 10; i++) {
        int *tls_ptr = get_tls_pointer(seed + i);
        *tls_ptr += i;
        use_ptr(tls_ptr);
    }
    
    /* Mix TLS variables in conditionals */
    if (public_tls > 1000) {
        hidden_tls /= 2;
    } else if (weak_tls < 500) {
        internal_tls *= 2;
    }
    
    /* Compute checksum of all TLS values to prevent removal */
    int checksum = 0;
    checksum += public_tls;
    checksum += public_tls_uninit;
    checksum += weak_tls;
    checksum += hidden_tls;
    checksum += internal_tls;
    checksum += protected_tls;
    checksum += dllimport_tls;
    checksum += common_tls;
    checksum += static_tls;
    checksum += volatile_tls;
    checksum += dynamic_tls;
    checksum += preserve_tls;
    checksum += aligned_tls;
    checksum += external_tls;
    checksum += external_weak_tls;
    
    /* Use checksum to prevent dead code elimination */
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    /* Final use of all TLS variables through opaque function */
    use_int(public_tls);
    use_int(hidden_tls);
    use_int(internal_tls);
    use_int(protected_tls);
    
    return checksum == 0 ? 0 : 1;
}

/* ========== EXTERNAL FUNCTION STUBS (for linking) ========== */
/* In a real test environment, these would be empty or return dummy values */
void use_ptr(void *p) {
    /* Prevent optimization */
    static volatile void *last_ptr = NULL;
    last_ptr = p;
}

void use_int(int x) {
    /* Prevent optimization */
    static volatile int last_int = 0;
    last_int = x;
}

int get_random(void) {
    return 42; /* Deterministic for testing */
}
