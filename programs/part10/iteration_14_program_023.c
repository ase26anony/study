/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern int extern_tls_var;
extern long extern_tls_var2;
extern void use_tls_pointers(int *p1, long *p2, double *p3);
extern int opaque_function(int x);

/* Global TLS with explicit attributes */
__thread int global_tls __attribute__((used, visibility("default")));
__thread long global_tls2 __attribute__((weak, visibility("hidden")));
__thread double global_tls3 __attribute__((visibility("internal")));

/* Static TLS (file-local) with attributes */
static __thread int static_tls __attribute__((used));
static __thread long static_tls2;

/* Common TLS (no explicit storage) */
__thread int common_tls;
__thread long common_tls2 __attribute__((visibility("protected")));

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    global_tls = rand() % 1000;
    global_tls2 = rand() % 1000;
    global_tls3 = (double)(rand() % 1000) / 10.0;
    static_tls = rand() % 1000;
    static_tls2 = rand() % 1000;
    common_tls = rand() % 1000;
    common_tls2 = rand() % 1000;
    extern_tls_var = rand() % 1000;
    extern_tls_var2 = rand() % 1000;
}

/* Helper function that forces address taking */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &global_tls;
    long *p2 = &global_tls2;
    double *p3 = &global_tls3;
    int *p4 = &static_tls;
    long *p5 = &static_tls2;
    int *p6 = &common_tls;
    long *p7 = &common_tls2;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5), "r"(p6), "r"(p7) : "memory");
    
    /* Call external function with TLS addresses */
    use_tls_pointers(p1, p2, p3);
}

/* Function using statement expression with TLS */
static int tls_in_statement_expr(void) {
    /* GNU statement expression that uses TLS address */
    int result = ({
        int local_tls __attribute__((unused)) = 42;
        int *addr = &global_tls;
        int temp = *addr + common_tls;
        
        /* Force declaration duplication context */
        __asm__ volatile ("# TLS in statement expr %0" : : "r"(addr) : "memory");
        
        /* Call opaque function */
        temp += opaque_function(static_tls);
        temp;
    });
    
    return result;
}

int main(void) {
    /* Local TLS variable in main */
    __thread int local_main_tls __attribute__((used)) = 123;
    
    /* Force address taking of local TLS */
    int *local_addr = &local_main_tls;
    
    /* Manipulate various TLS addresses */
    manipulate_tls_addresses();
    
    /* Use statement expression with TLS */
    int sum = tls_in_statement_expr();
    
    /* Complex computation using multiple TLS variables */
    sum += global_tls;
    sum += global_tls2;
    sum += global_tls3;
    sum += static_tls;
    sum += static_tls2;
    sum += common_tls;
    sum += common_tls2;
    sum += extern_tls_var;
    sum += extern_tls_var2;
    sum += *local_addr;
    
    /* Use inline assembly with TLS address */
    __asm__ volatile (
        "addl %1, %0\n\t"
        : "+r"(sum)
        : "r"(*local_addr)
        : "cc"
    );
    
    printf("TLS checksum: %d\n", sum);
    return 0;
}
