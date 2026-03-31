/* test-emutls-attributes.c */
/* This should trigger emulated TLS code generation */

#include <stdio.h>

/* Force emulated TLS attribute copying */
#ifdef __GNUC__
#define TLS_ATTRS __attribute__((used))
#else
#define TLS_ATTRS
#endif

/* 1. Plain TLS with external linkage, initialized */
__thread int tls_default = 1;

/* 2. Static TLS with internal linkage */
static __thread int tls_static = 2;

/* 3. Extern declaration (simulating header) */
extern __thread int tls_extern;

/* 4. Weak TLS symbol */
__attribute__((weak)) __thread int tls_weak TLS_ATTRS;

/* 5. Hidden visibility TLS */
__attribute__((visibility("hidden"))) __thread int tls_hidden TLS_ATTRS;

/* 6. Default visibility TLS */
__attribute__((visibility("default"))) __thread int tls_visible TLS_ATTRS;

/* 7. DLL import simulation (for DECL_DLLIMPORT_P) */
#ifdef _WIN32
__attribute__((dllimport)) __thread int tls_dllimport;
#else
/* Simulate similar attribute on non-Windows */
__attribute__((weak, visibility("default"))) __thread int tls_dllimport TLS_ATTRS;
#endif

/* 8. Uninitialized TLS */
__thread int tls_uninit TLS_ATTRS;

/* 9. Common TLS (uninitialized external) */
__thread int tls_common TLS_ATTRS;

/* Definition of extern TLS */
__thread int tls_extern = 3;

/* Helper function that uses TLS variables */
void modify_tls(void) {
    /* Read and modify various TLS variables */
    tls_static = tls_default * 10;
    tls_hidden = tls_static + 5;
    
    if (tls_weak == 0) {
        tls_weak = 100;  /* Initialize weak TLS if not already */
    }
    
    tls_visible = tls_hidden + tls_weak;
    tls_uninit = 42;
    
    /* Take address to inhibit optimizations */
    volatile int *addr = &tls_static;
    (void)addr;  /* Use variable to avoid unused warning */
}

/* Another helper that takes TLS address */
int* get_tls_address(int selector) {
    switch (selector) {
        case 0: return &tls_default;
        case 1: return &tls_static;
        case 2: return &tls_hidden;
        case 3: return &tls_visible;
        default: return &tls_uninit;
    }
}

int main(void) {
    int sum = 0;
    
    /* Initialize uninitialized TLS */
    tls_uninit = 7;
    tls_common = 8;
    
    /* Use all TLS variables to ensure they're processed */
    sum += tls_default;
    sum += tls_static;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_visible;
    sum += tls_uninit;
    sum += tls_common;
    
    /* Modify via helper function */
    modify_tls();
    
    /* Recalculate sum after modification */
    sum = tls_default + tls_static + tls_extern + tls_weak +
          tls_hidden + tls_visible + tls_uninit + tls_common;
    
    /* Take addresses to force symbol usage */
    volatile int *addr1 = &tls_default;
    volatile int *addr2 = &tls_hidden;
    volatile int *addr3 = get_tls_address(2);
    
    (void)addr1;
    (void)addr2;
    (void)addr3;
    
    printf("TLS sum: %d\n", sum);
    printf("tls_default=%d, tls_static=%d, tls_extern=%d\n",
           tls_default, tls_static, tls_extern);
    printf("tls_weak=%d, tls_hidden=%d, tls_visible=%d\n",
           tls_weak, tls_hidden, tls_visible);
    
    return 0;
}

/* Force generation of emulated TLS structures */
void* __dummy_tls_refs[] = {
    (void*)&tls_default,
    (void*)&tls_static,
    (void*)&tls_extern,
    (void*)&tls_weak,
    (void*)&tls_hidden,
    (void*)&tls_visible,
    (void*)&tls_dllimport,
    (void*)&tls_uninit,
    (void*)&tls_common,
    0
};
