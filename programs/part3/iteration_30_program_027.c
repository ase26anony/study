/* test_tls_emulation.c - Comprehensive TLS test for GCC tree-emutls.cc coverage */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern void use_int(int x);
extern int get_random(void);
extern void escape(void *p);

/* Global seed for unpredictable control flow */
static volatile int global_seed = 0;

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage and default visibility */
__thread int tls_public = 42;
extern __thread int tls_extern;  /* Will be defined in another compilation unit */

/* Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 300;

/* Internal visibility TLS */
__thread int tls_internal __attribute__((visibility("internal"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Common TLS symbol (tentative definition) */
__thread int tls_common;  /* No initializer = common symbol */

/* Static TLS with preservation requirements */
static __thread int tls_static_preserve = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 600;

/* TLS with complex initializer (dynamic initialization) */
extern int compute_init(void);
__thread int tls_dynamic = compute_init();

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64))) = 700;

/* TLS array */
__thread int tls_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

/* TLS used in asm (forces DECL_PRESERVE_P) */
#ifdef __GNUC__
register __thread int tls_asm __asm__("tls_asm_reg") = 800;
#endif

/* ================= FUNCTION DECLARATIONS ================= */

/* Inline function accessing TLS - may trigger declaration copying during inlining */
static inline int inline_tls_access(int idx) {
    /* Mix different TLS variables in inline function */
    int result = tls_public + tls_hidden;
    
    /* Address-taking of TLS inside inline function */
    void *addr = &tls_static_preserve;
    escape(addr);
    
    /* Array access with TLS index */
    if (idx >= 0 && idx < 10) {
        result += tls_array[idx];
    }
    
    return result;
}

/* Function that takes address of TLS and uses it */
static void manipulate_tls_pointers(int seed) {
    /* Array of pointers to different TLS variables */
    void *tls_pointers[20];
    volatile void *volatile_ptr;  /* Volatile to prevent optimization */
    
    /* Take addresses of all TLS variables */
    tls_pointers[0] = &tls_public;
    tls_pointers[1] = &tls_extern;
    tls_pointers[2] = &tls_weak;
    tls_pointers[3] = &tls_hidden;
    tls_pointers[4] = &tls_protected;
    tls_pointers[5] = &tls_internal;
    tls_pointers[6] = &tls_dllimport;
    tls_pointers[7] = &tls_common;
    tls_pointers[8] = &tls_static_preserve;
    tls_pointers[9] = &tls_volatile;
    tls_pointers[10] = &tls_dynamic;
    tls_pointers[11] = &tls_aligned;
    tls_pointers[12] = &tls_array[0];
#ifdef __GNUC__
    tls_pointers[13] = &tls_asm;
#endif
    
    /* Use seed to select which pointer to access */
    int idx = seed % 14;
    volatile_ptr = tls_pointers[idx];
    
    /* Pass pointer to opaque function */
    use_ptr((void *)volatile_ptr);
    
    /* Modify TLS through pointer if not volatile */
    if (idx != 9) {  /* Skip volatile TLS */
        int *mod_ptr = (int *)tls_pointers[idx];
        *mod_ptr += seed;
    }
}

/* Function with loop that accesses TLS */
static int process_tls_in_loop(int iterations, int seed) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix of direct and indirect TLS access */
        sum += tls_public;
        sum += tls_hidden;
        
        /* Use inline function */
        sum += inline_tls_access(i % 10);
        
        /* Conditional TLS access based on seed */
        if ((seed + i) % 3 == 0) {
            sum += tls_weak;
        } else if ((seed + i) % 3 == 1) {
            sum += tls_protected;
        } else {
            sum += tls_internal;
        }
        
        /* Array access with TLS index */
        tls_array[i % 10] = (tls_array[i % 10] * 13 + 17) % 100;
    }
    
    return sum;
}

/* Function that creates a local TLS alias scenario */
static void create_tls_alias(void) {
    /* Taking address and storing in different contexts */
    int *alias1 = &tls_public;
    static int *saved_alias = NULL;
    
    if (!saved_alias) {
        saved_alias = &tls_public;  /* Persistent alias */
    }
    
    /* Different paths for alias creation */
    int *alias2;
    if (global_seed % 2) {
        alias2 = &tls_hidden;
    } else {
        alias2 = &tls_protected;
    }
    
    /* Use aliases */
    *alias1 += 1;
    *alias2 += 2;
    if (saved_alias) {
        *saved_alias += 3;
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
    
    /* Initialize some TLS variables with seed-dependent values */
    tls_public = seed;
    tls_hidden = seed * 2;
    tls_protected = seed * 3;
    tls_internal = seed * 4;
    tls_weak = seed * 5;
    
    /* Fill TLS array with pattern */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = (seed + i * 7) % 1000;
    }
    
    /* Phase 1: Manipulate TLS pointers */
    manipulate_tls_pointers(seed);
    
    /* Phase 2: Process TLS in loops */
    int loop_result = process_tls_in_loop(100 + (seed % 50), seed);
    
    /* Phase 3: Create TLS aliases */
    create_tls_alias();
    
    /* Phase 4: More complex TLS usage patterns */
    int *tls_refs[5];
    tls_refs[0] = &tls_public;
    tls_refs[1] = &tls_hidden;
    tls_refs[2] = &tls_protected;
    tls_refs[3] = &tls_internal;
    tls_refs[4] = &tls_weak;
    
    for (int i = 0; i < 5; i++) {
        /* Chain of assignments through TLS pointers */
        int temp = *tls_refs[i];
        *tls_refs[(i + 1) % 5] = temp + i;
        
        /* Call opaque function with TLS address */
        use_ptr(tls_refs[i]);
    }
    
    /* Phase 5: Volatile access pattern */
    for (volatile int i = 0; i < 10; i++) {
        int val = tls_volatile;
        tls_volatile = val + i;
        use_int(val);
    }
    
    /* Compute checksum of all TLS variables to prevent elimination */
    int checksum = 0;
    checksum += tls_public;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_protected;
    checksum += tls_internal;
    checksum += tls_dllimport;
    checksum += tls_common;
    checksum += tls_static_preserve;
    checksum += tls_volatile;
    checksum += tls_dynamic;
    checksum += tls_aligned;
    
    for (int i = 0; i < 10; i++) {
        checksum += tls_array[i];
    }
    
#ifdef __GNUC__
    checksum += tls_asm;
#endif
    
    checksum += loop_result;
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* ================= EXTERNAL TLS DEFINITION ================= */
/* This simulates another compilation unit */
__thread int tls_extern = 999;

/* ================= STUB FUNCTIONS ================= */
/* These would be provided in a separate file in a real test */
int compute_init(void) { return 1234; }
void use_ptr(void *p) { /* Opaque */ }
void use_int(int x) { /* Opaque */ }
void escape(void *p) { /* Opaque */ }
