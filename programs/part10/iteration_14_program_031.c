/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations from tls_aux.c */
extern long extern_tls_var;
extern int extern_tls_with_attr __attribute__((weak, visibility("hidden")));

/* Static TLS with various attributes */
static __thread int static_tls __attribute__((used)) = 42;
static __thread double static_tls_hidden __attribute__((visibility("hidden"))) = 3.14;

/* Non-static TLS with external linkage */
__thread int global_tls = 100;
__thread long global_tls_weak __attribute__((weak)) = 200;

/* TLS with DLL import simulation (using weak as proxy) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
__thread int imported_tls __attribute__((weak));
#endif

/* Constructor to initialize TLS with non-constant values */
void __attribute__((constructor)) init_tls_values(void) {
    srand(time(NULL));
    static_tls = rand() % 1000;
    global_tls = rand() % 500;
    extern_tls_var = rand() % 300;
}

/* Opaque function that forces address taking */
static __attribute__((noinline, used)) 
void use_tls_addresses(int* a, long* b, double* c) {
    /* Inline assembly to prevent optimization */
    asm volatile ("" : : "r"(a), "r"(b), "r"(c) : "memory");
    
    /* Complex operations to ensure variables are used */
    *a += (*b % 100) + (int)(*c * 10);
}

/* Another opaque function */
static __attribute__((noinline)) 
int manipulate_tls(void* addr) {
    int result;
    /* Statement expression creating complex context */
    result = ({
        int temp = *(int*)addr;
        asm volatile ("# TLS use %0" : : "r"(temp));
        temp * 2 + 1;
    });
    return result;
}

int main(void) {
    /* Local TLS variable in main */
    __thread int local_main_tls __attribute__((used)) = 999;
    
    /* Take addresses of various TLS variables */
    int* addr1 = &static_tls;
    long* addr2 = &extern_tls_var;
    double* addr3 = &static_tls_hidden;
    int* addr4 = &global_tls;
    int* addr5 = &local_main_tls;
    
    /* Force address usage in noinline function */
    use_tls_addresses(addr1, addr2, addr3);
    
    /* Use statement expression with TLS address */
    int val1 = manipulate_tls(&global_tls_weak);
    
    /* Complex expression using multiple TLS variables */
    int checksum = static_tls + global_tls + (int)extern_tls_var 
                   + (int)static_tls_hidden + local_main_tls + val1;
    
    /* Use extern TLS with attributes */
    if (&extern_tls_with_attr) {
        checksum += extern_tls_with_attr;
    }
    
    /* Use imported TLS */
    checksum += imported_tls;
    
    printf("TLS checksum: %d\n", checksum);
    printf("static_tls: %d, global_tls: %d\n", static_tls, global_tls);
    
    /* Additional complex use case */
    {
        /* Nested block with another TLS declaration */
        __thread int nested_tls __attribute__((visibility("default"))) = checksum;
        
        /* Take address and use in inline assembly */
        register int* nested_ptr asm("r12") = &nested_tls;
        asm volatile (
            "addl $5, (%1)\n"
            : "=m"(nested_tls)
            : "r"(nested_ptr)
            : "memory"
        );
        
        printf("nested_tls: %d\n", nested_tls);
    }
    
    return 0;
}
