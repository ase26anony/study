/* This should trigger emulated TLS code generation */
/* Test case for TLS attribute copying in tree-emutls.cc */

/* Force emulated TLS by using appropriate compilation flags:
   -femulated-tls -O0 -fPIC
   or target architecture without native TLS support
*/

#include <stdio.h>

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage */
static __thread int tls_static = 2;

/* 3. External declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak;

/* 5. TLS with hidden visibility */
__attribute__((visibility("hidden"))) __thread int tls_hidden;

/* 6. TLS with default visibility and used attribute */
__attribute__((visibility("default"), used)) __thread int tls_visible_used;

/* 7. DLL import simulation (for Windows-like targets) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* On non-Windows, use a different attribute to ensure DECL_DLLIMPORT_P might be set */
__thread int tls_dllimport __attribute__((weak));
#endif

/* 8. Common TLS (uninitialized, external linkage) */
__thread int tls_common;

/* 9. Weak external reference */
extern __thread int tls_weak_extern __attribute__((weak));

/* Definition of the extern TLS variable */
__thread int tls_extern = 3;

/* Helper function to modify TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_default += 10;
    tls_static *= 2;
    tls_hidden = 100;
    
    /* Use weak TLS if available */
    if (&tls_weak) {
        tls_weak = 50;
    }
    
    /* Force use of tls_visible_used */
    tls_visible_used = tls_default + tls_static;
}

/* Dummy function that takes TLS address */
void use_tls_address(int *addr) {
    /* Prevent optimization */
    static volatile int sink;
    sink = *addr;
}

int main(void) {
    int result = 0;
    
    /* Initialize some TLS variables */
    tls_hidden = 42;
    tls_common = 99;
    
    /* Take addresses to force symbol usage */
    int *addr1 = &tls_default;
    int *addr2 = &tls_static;
    int *addr3 = &tls_extern;
    
    /* Use the addresses to prevent optimization */
    use_tls_address(addr1);
    use_tls_address(addr2);
    use_tls_address(addr3);
    
    /* Call function that modifies TLS */
    modify_tls();
    
    /* Compute result using TLS variables */
    result = tls_default + tls_static + tls_extern + tls_hidden;
    
    /* Add weak TLS if available */
    if (&tls_weak) {
        result += tls_weak;
    }
    
    /* Use common TLS */
    result += tls_common;
    
    /* Use visible used TLS */
    result += tls_visible_used;
    
    printf("TLS result: %d\n", result);
    printf("tls_default=%d, tls_static=%d, tls_extern=%d\n", 
           tls_default, tls_static, tls_extern);
    
    /* Additional complex expression to ensure all TLS is used */
    volatile int check = 
        (tls_default > 0) + 
        (tls_static > 0) + 
        (tls_extern > 0) + 
        (tls_hidden > 0);
    
    return result > 0 ? 0 : 1;
}

/* Force generation of TLS for weak external */
__thread int tls_weak_extern __attribute__((weak)) = 777;
