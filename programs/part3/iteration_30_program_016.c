/* test_tls_emulation.c - Comprehensive TLS emulation test */

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern int get_seed(void);
extern void escape(void *p);

/* Global seed for unpredictable control flow */
static volatile int global_seed;

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with external linkage, default visibility */
__thread int tls_public = 42;
extern __thread int tls_extern;  /* Will be defined in another "compilation unit" */

/* Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 100;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* Internal visibility TLS */
__thread int tls_internal __attribute__((visibility("internal"))) = 300;

/* Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 400;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Common TLS (tentative definition) */
__thread int tls_common;  /* No initializer for common symbol */

/* Static TLS with preservation requirement */
static __thread int tls_static_preserve = 500;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 600;

/* TLS with dynamic initialization */
extern int compute_init(void);
__thread int tls_dynamic = compute_init();

/* TLS with alignment requirement */
__thread int tls_aligned __attribute__((aligned(64))) = 700;

/* ================= FUNCTION DECLARATIONS ================= */

static inline int inline_tls_access(int idx);
static void process_tls_pointers(void);
static int compute_checksum(void);

/* ================= INLINE FUNCTION (may trigger declaration copying) ================= */

static inline int inline_tls_access(int idx) {
    /* This inline function accesses TLS - may cause declaration cloning */
    static __thread int local_tls = 0;
    local_tls += idx;
    
    /* Mix with various TLS variables */
    tls_public += local_tls;
    tls_hidden -= idx;
    
    /* Take address of TLS variable inside inline function */
    void *addr = &local_tls;
    escape(addr);
    
    return local_tls;
}

/* ================= TLS VARIABLE DEFINITIONS ================= */

/* Define the extern TLS variable (simulating another compilation unit) */
__thread int tls_extern = 999;

/* Dynamic initializer function */
int compute_init(void) {
    return 1234;
}

/* ================= MAIN TEST LOGIC ================= */

int main(int argc, char *argv[]) {
    /* Unpredictable control flow based on input */
    if (argc > 1) {
        global_seed = argv[1][0];
    } else {
        global_seed = get_seed();
    }
    
    /* Array of volatile pointers to TLS variables */
    volatile void *tls_pointers[15];
    int i = 0;
    
    /* Take addresses of all TLS variables - forces emulation machinery */
    tls_pointers[i++] = (void*)&tls_public;
    tls_pointers[i++] = (void*)&tls_extern;
    tls_pointers[i++] = (void*)&tls_weak;
    tls_pointers[i++] = (void*)&tls_hidden;
    tls_pointers[i++] = (void*)&tls_internal;
    tls_pointers[i++] = (void*)&tls_protected;
    tls_pointers[i++] = (void*)&tls_dllimport;
    tls_pointers[i++] = (void*)&tls_common;
    tls_pointers[i++] = (void*)&tls_static_preserve;
    tls_pointers[i++] = (void*)&tls_volatile;
    tls_pointers[i++] = (void*)&tls_dynamic;
    tls_pointers[i++] = (void*)&tls_aligned;
    
    /* Pass pointers to opaque function to prevent optimization */
    for (int j = 0; j < i; j++) {
        use_ptr((void*)tls_pointers[j]);
    }
    
    /* Complex TLS usage patterns */
    int sum = 0;
    for (int k = 0; k < (global_seed % 100) + 10; k++) {
        /* Mix TLS accesses with runtime values */
        tls_public += k;
        tls_hidden -= (k * 2);
        
        /* Use inline function with TLS access */
        sum += inline_tls_access(k);
        
        /* Conditional TLS access */
        if (k % 3 == 0) {
            tls_weak = tls_public;
        } else if (k % 3 == 1) {
            tls_internal = tls_hidden;
        } else {
            tls_protected = k;
        }
        
        /* Volatile access prevents dead code elimination */
        sum += tls_volatile;
        
        /* Address computation with TLS */
        int *ptr = &tls_common;
        *ptr += sum % 100;
    }
    
    /* Process TLS pointers in separate function */
    process_tls_pointers();
    
    /* Compute final checksum to prevent removal of all accesses */
    int checksum = compute_checksum();
    
    /* Use checksum to affect program output */
    printf("TLS checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* ================= HELPER FUNCTIONS ================= */

static void process_tls_pointers(void) {
    /* Create local array of TLS pointers */
    void *local_ptrs[5];
    
    /* Take addresses inside function */
    local_ptrs[0] = &tls_public;
    local_ptrs[1] = &tls_hidden;
    local_ptrs[2] = &tls_internal;
    local_ptrs[3] = &tls_protected;
    local_ptrs[4] = &tls_static_preserve;
    
    /* Opaque operations */
    for (int i = 0; i < 5; i++) {
        escape(local_ptrs[i]);
    }
    
    /* Modify TLS through pointers */
    *(int*)local_ptrs[0] += 1;
    *(int*)local_ptrs[1] += 2;
}

static int compute_checksum(void) {
    int checksum = 0;
    
    /* Access all TLS variables non-volatilely for checksum */
    checksum += tls_public;
    checksum += tls_extern;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_dllimport;
    checksum += tls_common;
    checksum += tls_static_preserve;
    checksum += tls_volatile;
    checksum += tls_dynamic;
    checksum += tls_aligned;
    
    /* Add some non-linear mixing */
    checksum ^= (checksum << 3);
    checksum ^= (checksum >> 5);
    
    return checksum;
}

/* ================= STUB FUNCTIONS (for linking) ================= */

/* These would be defined elsewhere in a real test environment */
void use_ptr(void *p) {
    /* Prevent optimization */
    asm volatile("" : : "r"(p) : "memory");
}

int get_seed(void) {
    return 12345;
}

void escape(void *p) {
    /* Prevent optimization */
    asm volatile("" : : "r"(p) : "memory");
}
