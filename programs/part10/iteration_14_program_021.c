/* tls_main.c - Main file with TLS declarations and usage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, int *b, double *c, long *d);
extern int checksum;

/* TLS declarations with various attributes */
__thread int tls_global = 42;  /* Potentially common linkage */
__thread int tls_used __attribute__((used)) = 100;
__thread int tls_weak __attribute__((weak)) = 200;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 300;

/* Static TLS - file-local */
static __thread int tls_static = 400;

/* External TLS declaration */
extern __thread int tls_extern;

/* DLL import simulation for non-Windows */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((dllimport));
#endif

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    tls_global = rand() % 1000;
    tls_used = rand() % 1000;
    tls_weak = rand() % 1000;
    tls_hidden = rand() % 1000;
    tls_static = rand() % 1000;
}

/* Noinline helper to force address taking */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses of TLS variables */
    int *addr1 = &tls_global;
    int *addr2 = &tls_used;
    int *addr3 = &tls_weak;
    int *addr4 = &tls_hidden;
    int *addr5 = &tls_static;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile (
        "# TLS address manipulation\n"
        : "+r" (addr1), "+r" (addr2)
        : "r" (addr3), "r" (addr4), "r" (addr5)
        : "memory"
    );
    
    /* Complex statement expression with TLS address */
    int result = ({
        int temp = *addr1 + *addr2;
        temp += *addr3;
        temp;
    });
    
    (void)result;
}

/* Another noinline function that uses TLS in complex ways */
__attribute__((noinline, used))
static void complex_tls_usage(void) {
    /* Create a local TLS variable in a function */
    __thread int local_tls = 500;
    
    /* Use GNU statement expression with TLS */
    int computed = ({
        int sum = local_tls + tls_static;
        /* Force address taking in the expression */
        int *addr = &local_tls;
        sum += *addr;
        sum;
    });
    
    /* Pass TLS addresses to external function */
    use_tls_pointers(&local_tls, &tls_global, 
                     (double*)&tls_used, (long*)&tls_weak);
    
    (void)computed;
}

int main(void) {
    /* Call functions that manipulate TLS */
    manipulate_tls_addresses();
    complex_tls_usage();
    
    /* Use TLS variables in arithmetic */
    int sum = tls_global + tls_used + tls_weak + tls_hidden;
    
    /* Force address taking of extern TLS */
    int *extern_addr = &tls_extern;
    
    /* Complex expression with multiple TLS uses */
    long result = ({
        long val = (long)tls_global * tls_used;
        val += (long)tls_weak * tls_hidden;
        val += (long)*extern_addr;
        
        /* Nested statement expression with TLS address */
        int nested = ({
            int *p = &tls_global;
            *p + tls_static;
        });
        val += nested;
        
        val;
    });
    
    printf("TLS checksum: %ld\n", result);
    
    /* Additional complex usage pattern */
    {
        /* Another local TLS variable */
        __thread int main_local_tls = 600;
        
        /* Use address in inline assembly */
        __asm__ volatile (
            "# Main TLS usage\n"
            : 
            : "r" (&main_local_tls), "r" (&tls_global)
            : "memory"
        );
        
        /* Chain of TLS operations */
        main_local_tls = tls_global + tls_used;
        tls_global = main_local_tls * 2;
    }
    
    return 0;
}
