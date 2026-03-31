/* tls_main.c - Main file with various TLS declarations and usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern __thread int extern_tls_var;
extern __thread long extern_tls_long __attribute__((weak));
__thread double global_tls_double __attribute__((used, visibility("default")));

/* Static TLS with hidden visibility */
static __thread int static_tls_int __attribute__((visibility("hidden")));

/* Weak TLS variable */
__thread char weak_tls_char __attribute__((weak));

/* DLL import simulation (non-Windows fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_var;
#else
/* Use a different mechanism to potentially set DECL_DLLIMPORT_P */
__thread int imported_tls_var __attribute__((visibility("default")));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor)) 
static void init_tls_vars(void) {
    srand(time(NULL));
    static_tls_int = rand() % 100;
    global_tls_double = (double)rand() / RAND_MAX * 100.0;
    weak_tls_char = 'A' + (rand() % 26);
}

/* Opaque function that forces address taking */
static __attribute__((noinline, used))
void manipulate_tls_pointers(int *p1, double *p2, char *p3, long *p4) {
    /* Use inline assembly to prevent optimization */
    __asm__ volatile (
        "# TLS pointer manipulation"
        : 
        : "r" (p1), "r" (p2), "r" (p3), "r" (p4)
        : "memory"
    );
    
    /* Perform opaque operations */
    if (p1) *p1 += 1;
    if (p2) *p2 *= 1.01;
    if (p3) *p3 = (*p3 - 'A' + 1) % 26 + 'A';
    if (p4) *p4 ^= 0x55555555;
}

/* Another opaque function */
static __attribute__((noinline))
int compute_checksum(void) {
    int sum = 0;
    
    /* Complex statement expression that may trigger declaration cloning */
    sum = ({
        int local_sum = 0;
        __thread int local_tls_in_expr __attribute__((used));
        local_tls_in_expr = static_tls_int + (int)global_tls_double;
        
        /* Take address in statement expression */
        int *addr = &local_tls_in_expr;
        
        /* Force declaration usage in multiple contexts */
        __asm__ volatile (
            "# Statement expr TLS use"
            : "+r" (*addr)
            :
            : "memory"
        );
        
        local_sum = *addr + weak_tls_char;
        local_sum;
    });
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    /* Local TLS variable in main */
    __thread int main_local_tls __attribute__((used)) = 42;
    
    /* Take addresses of various TLS variables */
    int *addr1 = &static_tls_int;
    double *addr2 = &global_tls_double;
    char *addr3 = &weak_tls_char;
    long *addr4 = &extern_tls_long;
    int *addr5 = &main_local_tls;
    
    /* Pass addresses to opaque function - may trigger declaration cloning */
    manipulate_tls_pointers(addr1, addr2, addr3, addr4);
    
    /* Use statement expression with TLS address */
    checksum += ({
        int val = *addr1 + (int)*addr2 + *addr3;
        /* Another local TLS in nested context */
        __thread int nested_tls __attribute__((visibility("hidden")));
        nested_tls = val;
        
        /* Force address taking in nested scope */
        int *nested_addr = &nested_tls;
        __asm__ volatile ("# Nested TLS use" : "+r" (*nested_addr));
        
        *nested_addr + *addr5;
    });
    
    /* More complex usage patterns */
    for (int i = 0; i < 3; i++) {
        /* Loop creates multiple contexts for TLS variables */
        __thread int loop_tls __attribute__((used)) = i * 10;
        checksum += loop_tls + extern_tls_var;
        
        /* Modify through pointer */
        int *loop_ptr = &loop_tls;
        *loop_ptr += i;
    }
    
    /* Compute final checksum */
    checksum += compute_checksum();
    
    /* Use all TLS variables to prevent optimization */
    checksum += static_tls_int;
    checksum += (int)global_tls_double;
    checksum += weak_tls_char;
    checksum += extern_tls_var;
    
    printf("TLS checksum: %d\n", checksum);
    
    return 0;
}
