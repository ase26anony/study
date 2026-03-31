/* This should trigger emulated TLS code generation */
/* Test case for TLS emulation attribute propagation */

/* Force emulated TLS by using -femulated-tls or targeting ARM without native TLS */

/* TLS variable with default external linkage, initialized */
__thread int tls_default = 1;

/* Static TLS variable with internal linkage */
static __thread int tls_static = 2;

/* External TLS declaration (simulating header) */
extern __thread int tls_extern;

/* Weak TLS variable - should set DECL_WEAK */
__attribute__((weak)) __thread int tls_weak = 5;

/* TLS with hidden visibility - should set DECL_VISIBILITY */
__attribute__((visibility("hidden"))) __thread int tls_hidden = 6;

/* TLS with default visibility explicitly specified */
__attribute__((visibility("default"))) __thread int tls_visible = 7;

/* Used attribute to ensure TREE_USED is set */
__attribute__((used)) __thread int tls_used = 8;

/* DLL import attribute (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, simulate with other attributes */
__attribute__((weak, visibility("default"))) __thread int tls_dllimport = 9;
#endif

/* Uninitialized TLS variables with different attributes */
__thread int tls_uninit;
__attribute__((weak)) __thread int tls_weak_uninit;
static __thread int tls_static_uninit;

/* Definition of the previously declared extern TLS variable */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS variable */
    if (tls_weak) {
        tls_weak = 100;
    }
    
    /* Use hidden visibility TLS */
    tls_hidden = tls_visible + tls_used;
    
    /* Ensure uninitialized TLS is touched */
    tls_uninit = 42;
    tls_weak_uninit = 24;
    tls_static_uninit = 84;
}

/* Another helper that takes addresses of TLS variables */
void* get_tls_addresses(void) {
    /* Taking addresses forces symbol references */
    static void* addresses[6];
    
    addresses[0] = &tls_default;
    addresses[1] = &tls_static;
    addresses[2] = &tls_extern;
    addresses[3] = &tls_weak;
    addresses[4] = &tls_hidden;
    addresses[5] = &tls_visible;
    
    /* Use the addresses to prevent optimization */
    if ((long)addresses[0] > (long)addresses[1]) {
        return addresses[0];
    }
    return addresses[1];
}

int main(void) {
    int sum = 0;
    
    /* Initial use of TLS variables */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible;
    sum += tls_used;
    
    /* Call function that modifies TLS */
    modify_tls();
    
    /* Use TLS variables after modification */
    sum += tls_default * 2;
    sum += tls_static / 2;
    sum += tls_extern + tls_weak;
    
    /* Initialize uninitialized TLS */
    tls_uninit = sum % 100;
    tls_weak_uninit = sum / 100;
    tls_static_uninit = sum;
    
    /* Take address of TLS variable - inhibits optimizations */
    int *tls_ptr = &tls_default;
    *tls_ptr += 1;
    
    /* Get addresses of multiple TLS variables */
    get_tls_addresses();
    
    /* Use DLL import simulated TLS */
    tls_dllimport = sum;
    
    /* Create side effect with TLS addresses */
    if (&tls_default != &tls_static) {
        sum += *tls_ptr;
    }
    
    /* Prevent dead code elimination */
    return sum % 256;
}

/* Additional TLS definition to test COMMON handling */
__thread int tls_common __attribute__((common));
