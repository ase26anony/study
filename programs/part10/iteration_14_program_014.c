/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern long get_tls_sum(void);
static void use_tls_addresses(void) __attribute__((noinline));

/* TLS declarations with different attributes */
/* Plain TLS with default visibility */
__thread int tls_global_default __attribute__((used)) = 42;

/* TLS with hidden visibility */
__thread int tls_global_hidden __attribute__((visibility("hidden"), used)) = 100;

/* Weak TLS declaration */
__thread int tls_weak_var __attribute__((weak, used)) = 200;

/* Static TLS (file-local) */
static __thread double tls_static_local = 3.14159;

/* Extern TLS declaration (defined in another file) */
extern __thread long tls_extern_var;

/* DLL import simulation (with fallback for non-Windows) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#else
__thread int tls_dllimport_var __attribute__((visibility("default"), used));
#endif

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        tls_global_default = rand() % 1000;
        tls_global_hidden = rand() % 1000;
        tls_weak_var = rand() % 1000;
        tls_static_local = (double)rand() / RAND_MAX * 100.0;
        initialized = 1;
    }
}

/* Helper function that takes TLS addresses - forces declaration cloning */
static void __attribute__((noinline))
use_tls_addresses(void) {
    /* Take addresses of TLS variables in ways that might trigger cloning */
    int *addr1 = &tls_global_default;
    int *addr2 = &tls_global_hidden;
    double *addr3 = &tls_static_local;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(addr1), "r"(addr2), "r"(addr3) : "memory");
    
    /* Use statement expression with TLS address */
    int result = ({
        int temp = *addr1 + *addr2;
        /* Force compiler to consider this complex context */
        __asm__ volatile ("# TLS use in statement expr" : : "r"(temp));
        temp;
    });
    
    (void)result;
}

/* Another function that manipulates TLS variables */
static long __attribute__((noinline))
compute_tls_checksum(void) {
    /* Access TLS variables in complex pattern */
    long sum = 0;
    
    /* Use GNU statement expressions with TLS variables */
    sum += ({
        int val = tls_global_default;
        val * 2;
    });
    
    sum += ({
        int val = tls_global_hidden;
        val / 2;
    });
    
    sum += ({
        double val = tls_static_local;
        (long)val;
    });
    
    /* Take address and use in inline assembly */
    int *weak_addr = &tls_weak_var;
    __asm__ volatile (
        "# TLS weak var manipulation"
        : "+r" (*weak_addr)
        : 
        : "memory"
    );
    
    sum += tls_weak_var;
    
    return sum;
}

int main(void) {
    /* Local TLS variable in main */
    __thread int tls_local_main __attribute__((used)) = 999;
    
    /* Force use of local TLS address */
    int *local_addr = &tls_local_main;
    
    /* Call functions that manipulate TLS */
    use_tls_addresses();
    
    long checksum = compute_tls_checksum();
    
    /* Get sum from external TLS variable */
    long extern_sum = get_tls_sum();
    
    /* Final computation using statement expression with TLS */
    long final_result = ({
        long temp = checksum + extern_sum + *local_addr;
        /* Complex context that might trigger declaration cloning */
        __asm__ volatile (
            "# Final TLS computation %0"
            : "+r" (temp)
            : 
            : "memory"
        );
        temp;
    });
    
    printf("TLS checksum result: %ld\n", final_result);
    
    /* Additional complex TLS usage pattern */
    {
        /* Nested scope with TLS manipulation */
        static __thread int nested_tls = 1234;
        int *nested_addr = &nested_tls;
        
        /* Use in asm with multiple constraints */
        __asm__ volatile (
            "addl $1, %0"
            : "=r" (*nested_addr)
            : "0" (*nested_addr)
            : "memory"
        );
        
        printf("Nested TLS value: %d\n", nested_tls);
    }
    
    return 0;
}
