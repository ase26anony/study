/* Main file with various TLS declarations and usage patterns */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
extern int checksum;

/* External TLS declaration - will be defined in another TU */
extern __thread int extern_tls_var;

/* TLS with various attributes to set the target flags */
__thread int global_tls __attribute__((used)) = 42;
__thread long global_tls_weak __attribute__((weak)) = 100;
static __thread int file_static_tls = 0;
__thread double tls_hidden __attribute__((visibility("hidden"))) = 3.14;

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    file_static_tls = rand() % 100;
    extern_tls_var = rand() % 200;
    tls_hidden = (double)rand() / RAND_MAX;
}

/* Opaque function that forces address taking */
static __attribute__((noinline))
void manipulate_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *p1 = &global_tls;
    long *p2 = &global_tls_weak;
    double *p3 = &tls_hidden;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
    
    /* Call external function with TLS addresses */
    use_tls_pointers(p1, p2, p3);
}

/* Another function that creates complex context for TLS */
static __attribute__((noinline))
int tls_in_statement_expr(void) {
    /* Use TLS variable in GNU statement expression */
    int result = ({
        __thread int local_stmt_tls __attribute__((used)) = 123;
        int *addr = &local_stmt_tls;
        /* Complex expression with side effects */
        __asm__ volatile ("# TLS in statement expr" : : "r"(addr));
        local_stmt_tls + global_tls;
    });
    
    return result;
}

int main(void) {
    /* Local TLS variable in main */
    __thread int local_main_tls = 789;
    
    /* Force address taking with complex pattern */
    int *addr1 = &local_main_tls;
    int *addr2 = &extern_tls_var;
    
    /* Use statement expression with TLS address */
    int val = ({
        /* This creates a new context that might trigger duplication */
        __thread int temp_tls = *addr1 + *addr2;
        __asm__ volatile ("# Complex TLS use" : : "r"(&temp_tls));
        temp_tls;
    });
    
    /* Call functions that manipulate TLS */
    manipulate_tls_addresses();
    val += tls_in_statement_expr();
    
    /* Compute checksum using all TLS variables */
    int sum = global_tls + global_tls_weak + file_static_tls + 
              (int)tls_hidden + local_main_tls + extern_tls_var + val;
    
    printf("TLS checksum: %d\n", sum);
    
    /* Prevent dead code elimination */
    __asm__ volatile ("# Final TLS barrier" : : "r"(sum));
    
    return 0;
}
