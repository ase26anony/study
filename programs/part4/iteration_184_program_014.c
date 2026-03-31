/* This should trigger emulated TLS code generation */
/* Test case for TLS attribute copying in emulated TLS */

/* Force emulated TLS handling */
#pragma GCC target("tls-model=emulated")

/* Declare TLS variables with various attributes */

/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static (internal linkage) TLS */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Weak TLS variable */
__attribute__((weak)) __thread int tls_weak = 5;

/* TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 6;

/* TLS with default visibility (explicit) */
__attribute__((visibility("default"))) __thread int tls_visible_default = 7;

/* Used attribute ensures TREE_USED is set */
__attribute__((used)) __thread int tls_used = 8;

/* DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, we can still test with dllimport attribute */
__attribute__((dllimport)) __thread int tls_dllimport = 9;
#endif

/* Uninitialized TLS variables with attributes */
__attribute__((weak)) __thread int tls_weak_uninit;
__attribute__((visibility("hidden"))) __thread int tls_hidden_uninit;

/* Define the previously declared extern TLS */
__thread int tls_extern = 3;

/* Common TLS (uninitialized, external linkage) */
__thread int tls_common;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    /* Use hidden visibility TLS */
    tls_hidden = tls_visible_default + 1;
    
    /* Ensure used attribute TLS is accessed */
    tls_used++;
    
    /* Access uninitialized TLS */
    tls_weak_uninit = 42;
    tls_hidden_uninit = 24;
    
    /* Use common TLS */
    tls_common = 99;
}

/* Another function that takes addresses of TLS variables */
void* get_tls_addresses(void) {
    /* Taking addresses forces symbol references */
    static void* addresses[8];
    
    addresses[0] = &tls_default;
    addresses[1] = &tls_static;
    addresses[2] = &tls_extern;
    addresses[3] = &tls_weak;
    addresses[4] = &tls_hidden;
    addresses[5] = &tls_visible_default;
    addresses[6] = &tls_used;
    addresses[7] = &tls_common;
    
    /* Use the addresses to prevent optimization */
    if (addresses[0] == addresses[1]) {
        return 0; /* Should never happen */
    }
    
    return addresses[0];
}

/* Main function demonstrating TLS usage */
int main(void) {
    int sum = 0;
    
    /* Initial values */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible_default;
    sum += tls_used;
    
    /* Print initial sum */
    /* printf("Initial sum: %d\n", sum); */
    
    /* Modify TLS in helper function */
    modify_tls();
    
    /* Recalculate sum */
    sum = 0;
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible_default;
    sum += tls_used;
    sum += tls_common;
    
    /* Print modified sum */
    /* printf("Modified sum: %d\n", sum); */
    
    /* Take addresses to force symbol usage */
    get_tls_addresses();
    
    /* Access DLL import TLS if available */
    if (&tls_dllimport) {
        sum += 1;
    }
    
    /* Return sum as exit code for verification */
    return sum > 0 ? 0 : 1;
}

/* Dummy function using TLS variable addresses to prevent optimization */
void __attribute__((noinline)) use_tls_pointers(void) {
    volatile int* p1 = &tls_default;
    volatile int* p2 = &tls_weak_uninit;
    volatile int* p3 = &tls_hidden_uninit;
    
    /* Create side effects */
    if (p1 && p2 && p3) {
        *p1 = *p1 + 1;
    }
}

/* Call the dummy function to ensure TLS addresses are used */
void init_tls_usage(void) {
    use_tls_pointers();
}

/* Constructor to ensure TLS is used early */
__attribute__((constructor)) static void init(void) {
    init_tls_usage();
}
