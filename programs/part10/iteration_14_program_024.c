/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
extern int compute_checksum(void);

/* External TLS declaration - will be defined in another file */
extern __thread int extern_tls_var __attribute__((weak, visibility("default")));

/* File-local static TLS with attributes */
static __thread int static_tls __attribute__((used, visibility("hidden"))) = 42;

/* Plain TLS with common linkage */
__thread long common_tls;

/* TLS with DLL import simulation (using weak as proxy) */
#ifdef _WIN32
__declspec(dllimport) __thread double imported_tls;
#else
__thread double imported_tls __attribute__((weak));
#endif

/* Constructor to initialize TLS vars */
__attribute__((constructor))
static void init_tls(void) {
    static_tls = rand() % 100;
    common_tls = rand() % 1000;
    imported_tls = (double)(rand() % 10000) / 100.0;
}

/* Noinline function to force address taking */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses in various ways to trigger duplication */
    int *p1 = &static_tls;
    long *p2 = &common_tls;
    double *p3 = &imported_tls;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
    
    /* Complex statement expression with address use */
    int result = ({
        int temp = *p1 + (int)*p2;
        temp += (int)*p3;
        temp;
    });
    
    (void)result;
}

/* Another function that creates a local TLS variable */
static void function_with_local_tls(void) {
    /* Local TLS variable - may trigger declaration in local scope */
    static __thread int local_func_tls __attribute__((used)) = 0;
    local_func_tls++;
    
    /* Use address in statement expression */
    int val = ({
        int *ptr = &local_func_tls;
        *ptr += 10;
        *ptr;
    });
    
    /* Pass address to external function */
    use_tls_pointers(&local_func_tls, &common_tls, &imported_tls);
}

int main(void) {
    /* Seed random for constructor */
    srand(42);
    
    /* Call init (constructor should have run, but ensure) */
    init_tls();
    
    /* Local TLS in main */
    __thread int main_local_tls __attribute__((visibility("default"))) = 100;
    
    /* Force address taking with inline asm */
    int *main_ptr = &main_local_tls;
    __asm__ volatile ("# TLS address: %0" : : "r"(main_ptr));
    
    /* Use GNU statement expression with complex TLS access */
    int checksum = ({
        /* Multiple TLS variable accesses */
        int sum = static_tls;
        sum += common_tls;
        sum += (int)imported_tls;
        sum += main_local_tls;
        
        /* Take address and dereference */
        int *addr = &main_local_tls;
        sum += *addr;
        
        /* Call function that manipulates TLS */
        manipulate_tls_addresses();
        
        sum;
    });
    
    /* Another scope with TLS usage */
    {
        __thread int block_tls = 50;
        checksum += block_tls;
        
        /* Force address into noinline function */
        use_tls_pointers(&block_tls, &common_tls, NULL);
    }
    
    function_with_local_tls();
    
    /* Compute final checksum */
    checksum += compute_checksum();
    
    printf("TLS checksum: %d\n", checksum);
    return 0;
}
