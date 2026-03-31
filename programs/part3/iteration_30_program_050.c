/* test_tls_emulation.c - Comprehensive TLS emulation test */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int val);
extern int get_random(void);
extern void side_effect(void);

/* Global seed for unpredictable control flow */
static volatile int global_seed;

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage and default visibility */
__thread int tls_public = 42;
extern __thread int tls_extern;  /* Defined in another compilation unit */

/* Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak = 100;

/* Hidden visibility TLS */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* Internal visibility TLS */
__attribute__((visibility("internal"))) __thread int tls_internal = 200;

/* Protected visibility TLS */
__attribute__((visibility("protected"))) __thread int tls_protected;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute if supported */
__attribute__((dllimport)) __thread int tls_dllimport;
#endif

/* Common TLS (tentative definition) */
__thread int tls_common;  /* No initializer = common symbol */

/* Static TLS (internal linkage) */
static __thread int tls_static = 999;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 1234;

/* Dynamically initialized TLS */
extern int compute_init(void);
__thread int tls_dynamic = compute_init();

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64)));

/* TLS that might need preservation (address escapes) */
__thread int tls_preserve;

/* ================= FUNCTION DECLARATIONS ================= */

static inline void inline_tls_access(int idx);
static void process_tls_variables(int seed);
extern void external_tls_user(void *ptr);

/* ================= INLINE FUNCTION (may trigger declaration copying) ================= */

static inline void inline_tls_access(int idx) {
    /* Access various TLS variables inside inline function */
    tls_static = idx;
    tls_hidden = tls_static * 2;
    
    /* Take address of TLS variable - may force proxy creation */
    void *addr = (void *)&tls_static;
    use_ptr(addr);
    
    /* Complex expression with TLS */
    tls_public = (tls_weak + tls_hidden) / (idx + 1);
}

/* ================= HELPER FUNCTIONS ================= */

/* Function that takes address of TLS - forces emulation machinery */
static void take_tls_addresses(void *ptrs[], int *count) {
    ptrs[(*count)++] = (void *)&tls_public;
    ptrs[(*count)++] = (void *)&tls_weak;
    ptrs[(*count)++] = (void *)&tls_hidden;
    ptrs[(*count)++] = (void *)&tls_internal;
    ptrs[(*count)++] = (void *)&tls_protected;
    ptrs[(*count)++] = (void *)&tls_common;
    ptrs[(*count)++] = (void *)&tls_static;
    ptrs[(*count)++] = (void *)&tls_volatile;
    ptrs[(*count)++] = (void *)&tls_dynamic;
    ptrs[(*count)++] = (void *)&tls_aligned;
    ptrs[(*count)++] = (void *)&tls_preserve;
    
    /* External function call with TLS address */
    for (int i = 0; i < *count; i++) {
        external_tls_user(ptrs[i]);
    }
}

/* Function that uses TLS in loops - may trigger optimizations */
static void tls_loop_operations(int seed) {
    volatile int counter = seed; /* Prevent loop unrolling */
    
    for (int i = 0; i < (counter % 100) + 10; i++) {
        /* Mix TLS and non-TLS accesses */
        tls_public += i;
        tls_static -= i;
        
        /* Conditional TLS access */
        if (i % 3 == 0) {
            tls_weak = tls_public * 2;
        } else if (i % 3 == 1) {
            tls_hidden = tls_static / 2;
        }
        
        /* Call inline function with TLS access */
        inline_tls_access(i);
        
        /* Volatile TLS access */
        tls_volatile = i;
    }
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char *argv[]) {
    /* Use argv for unpredictable control flow */
    int seed = 0;
    if (argc > 1) {
        for (char *p = argv[1]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    global_seed = seed;
    
    /* Array to store TLS addresses (prevents optimization) */
    void *tls_addresses[20];
    int addr_count = 0;
    
    /* ===== PHASE 1: Initialize TLS variables with varied patterns ===== */
    
    /* Use seed to create varied initialization patterns */
    tls_public = seed;
    tls_weak = seed * 2;
    tls_hidden = seed % 100;
    tls_internal = seed + 1000;
    tls_protected = seed * seed;
    tls_common = seed / 2;
    tls_static = 777;
    tls_volatile = seed | 0xFF;
    tls_dynamic = seed * 3;
    tls_aligned = seed * 4;
    tls_preserve = seed * 5;
    
    /* ===== PHASE 2: Take addresses of all TLS variables ===== */
    /* This forces the compiler to consider TLS emulation */
    take_tls_addresses(tls_addresses, &addr_count);
    
    /* ===== PHASE 3: Complex operations with TLS ===== */
    tls_loop_operations(seed);
    
    /* ===== PHASE 4: Conditional TLS access based on runtime values ===== */
    /* Use opaque function to prevent optimization */
    int selector = get_random();
    
    /* Switch on TLS variables - forces multiple code paths */
    switch (selector % 5) {
        case 0:
            tls_public = tls_weak + tls_hidden;
            use_ptr(&tls_public);
            break;
        case 1:
            tls_static = tls_internal - tls_protected;
            use_ptr(&tls_static);
            break;
        case 2:
            tls_common = tls_volatile * tls_dynamic;
            use_ptr(&tls_common);
            break;
        case 3:
            tls_aligned = tls_preserve / (seed + 1);
            use_ptr(&tls_aligned);
            break;
        case 4:
            /* Mix all TLS variables */
            tls_public = tls_weak + tls_static + tls_hidden;
            use_ptr(&tls_public);
            break;
    }
    
    /* ===== PHASE 5: Compute checksum to prevent dead code elimination ===== */
    int checksum = 0;
    
    /* Access all TLS variables non-volatilely for checksum */
    checksum += tls_public;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_common;
    checksum += tls_static;
    checksum += tls_volatile;  /* volatile access */
    checksum += tls_dynamic;
    checksum += tls_aligned;
    checksum += tls_preserve;
    
    /* Create side effect with checksum */
    use_int(checksum);
    
    /* Print result to prevent optimization */
    printf("TLS checksum: %d (seed: %d)\n", checksum, seed);
    
    return checksum % 256;
}

/* ================= EXTERNAL FUNCTION DEFINITIONS ================= */

/* These would normally be in another compilation unit */

/* External TLS variable definition */
__thread int tls_extern = 555;

/* Opaque function stubs (for linking) */
void use_ptr(void *p) {
    /* Empty but prevents optimization */
    (void)p;
}

void use_int(int val) {
    /* Empty but prevents optimization */
    (void)val;
}

int get_random(void) {
    /* Simple pseudo-random based on global seed */
    return global_seed * 1103515245 + 12345;
}

void external_tls_user(void *ptr) {
    /* Simulate external code using TLS pointer */
    static int counter = 0;
    counter++;
    (void)ptr;
}

int compute_init(void) {
    /* Dynamic initializer for TLS */
    return 888;
}
