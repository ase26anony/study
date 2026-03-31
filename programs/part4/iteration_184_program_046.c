/* test-emutls-attributes.c */
/* This should trigger emulated TLS code generation */

/* Force emulated TLS even if target supports native TLS */
#pragma GCC target("tls-model=emulated")

/* Declare TLS variables with various attributes and linkages */

/* Default external linkage, initialized */
__thread int tls_default = 1;

/* Static (internal) linkage */
static __thread int tls_static = 2;

/* External declaration (simulating header) */
extern __thread int tls_extern;

/* Weak TLS variable */
__attribute__((weak)) __thread int tls_weak;

/* Hidden visibility TLS */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* Default visibility with explicit attribute */
__attribute__((visibility("default"), used)) __thread int tls_visible;

/* DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use a different attribute to ensure DECL_DLLIMPORT_P might be set */
__thread int tls_dllimport __attribute__((weak));
#endif

/* Common TLS variable (uninitialized external) */
__thread int tls_common;

/* Define the previously declared extern TLS variable */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_extern -= 1;
    
    /* Use weak TLS if available */
    if (&tls_weak) {
        tls_weak = 100;
    }
    
    /* Hidden TLS access */
    tls_hidden = tls_default + tls_static;
    
    /* Ensure TREE_USED is set on all */
    tls_visible = tls_hidden;
    
    /* Common variable usage */
    tls_common = tls_extern;
}

/* Another helper to take addresses (inhibits optimizations) */
void* get_tls_addresses(void) {
    static void* addrs[7];
    
    /* Taking addresses ensures symbols are required */
    addrs[0] = &tls_default;
    addrs[1] = &tls_static;
    addrs[2] = &tls_extern;
    addrs[3] = &tls_weak;
    addrs[4] = &tls_hidden;
    addrs[5] = &tls_visible;
    addrs[6] = &tls_common;
    
    /* Use the addresses to prevent dead store elimination */
    volatile int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += ((int*)addrs[i]) != 0;
    }
    
    return addrs[sum % 7];
}

int main(void) {
    int result = 0;
    
    /* Initial TLS usage */
    result += tls_default;
    result += tls_static;
    result += tls_extern;
    
    /* Initialize uninitialized TLS variables */
    tls_weak = 42;
    tls_hidden = 24;
    tls_visible = 99;
    tls_common = 77;
    
    /* Use DLL import simulation variable */
    tls_dllimport = 55;
    result += tls_dllimport;
    
    /* Call helper to modify TLS */
    modify_tls();
    
    /* More operations after modification */
    result += tls_default * 2;
    result += tls_static / 2;
    result += tls_extern;
    
    /* Take addresses to ensure full processing */
    void* addr = get_tls_addresses();
    result += (addr != 0);
    
    /* Use hidden TLS */
    result += tls_hidden;
    
    /* Final computation using all TLS variables */
    int final_result = 
        tls_default + 
        tls_static + 
        tls_extern + 
        tls_weak + 
        tls_hidden + 
        tls_visible + 
        tls_dllimport + 
        tls_common;
    
    /* Prevent optimization of final_result */
    volatile int output = final_result + result;
    
    return output > 0 ? 0 : 1;
}
