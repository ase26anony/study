/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force emulated TLS */
#ifdef __APPLE__
/* macOS doesn't support __thread in emulated mode well, use workaround */
#define TLS_MODEL __attribute__((tls_model("emulated")))
#else
#define TLS_MODEL
#endif

/* Declaration with multiple attributes to set DECL_WEAK, DECL_VISIBILITY, etc. */
__attribute__((weak, visibility("hidden"), used))
extern __thread int extern_tls_var TLS_MODEL;

__attribute__((visibility("default"), used))
static __thread long static_tls_var TLS_MODEL = 42;

/* DLL import simulation for non-Windows (won't set DECL_DLLIMPORT_P but keeps structure) */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#endif

/* Variable with dllimport-like attribute */
DLL_IMPORT __thread double imported_tls_var TLS_MODEL;

/* Common linkage TLS variable */
__thread char common_tls_var TLS_MODEL;

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    static_tls_var = rand() % 1000;
    /* extern_tls_var defined in another file */
    common_tls_var = 'A' + (rand() % 26);
}

/* Opaque function that takes addresses, forcing declaration duplication */
static __attribute__((noinline, used))
void use_tls_addresses(int *p1, long *p2, double *p3, char *p4) {
    /* Use inline assembly to prevent optimization */
    asm volatile ("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    
    /* Complex computation to ensure variables are used */
    if (p1 && p2 && p3 && p4) {
        *p1 += *p2 + *p3 + *p4;
    }
}

/* External function that might cause declaration cloning */
extern void external_function(void *addr);

int main(void) {
    /* Local TLS variable in main - may trigger duplication */
    __thread int local_tls_var TLS_MODEL = rand() % 100;
    
    /* Take addresses of various TLS variables */
    int *addr1 = &extern_tls_var;
    long *addr2 = &static_tls_var;
    double *addr3 = &imported_tls_var;
    char *addr4 = &common_tls_var;
    int *addr5 = &local_tls_var;
    
    /* Use GNU statement expression to create complex context for declaration */
    int result = ({
        /* Reference TLS variable in nested scope */
        __thread int nested_tls TLS_MODEL = 100;
        int sum = nested_tls + local_tls_var;
        
        /* Take address and pass to external function */
        external_function(&nested_tls);
        
        /* Use inline assembly with TLS address */
        asm volatile ("# TLS use %0" : : "r"(&nested_tls));
        
        sum;
    });
    
    /* Force use of all TLS variable addresses */
    use_tls_addresses(addr1, addr2, addr3, addr4);
    
    /* Complex computation using TLS variables */
    local_tls_var += *addr1 + *addr2 + result;
    
    /* Compute checksum to prevent optimization */
    long checksum = (long)local_tls_var + (long)*addr2 + (long)*addr3 + (long)*addr4;
    printf("TLS checksum: %ld\n", checksum);
    
    /* Additional trigger: use address in switch with computed goto */
    void *targets[] = { &&label1, &&label2 };
    goto *targets[checksum % 2];
    
label1:
    /* Use TLS address in another asm statement */
    asm volatile ("# Label1 TLS %0" : : "r"(&extern_tls_var));
    return 0;
    
label2:
    asm volatile ("# Label2 TLS %0" : : "r"(&static_tls_var));
    return 1;
}
