/* Test for TLS declaration duplication in GCC's emulated TLS implementation */
/* This should trigger the attribute copying block in tree-emutls.cc */

#include <stdio.h>
#include <stdlib.h>

/* Declare TLS variables with various attributes to be copied */
/* Plain TLS with external linkage */
__thread int tls_ext_int __attribute__((used)) = 42;

/* TLS with weak linkage */
__thread long tls_weak_long __attribute__((weak, visibility("default"))) = 100;

/* Static TLS (file-local) with hidden visibility */
static __thread double tls_static_double __attribute__((visibility("hidden")));

/* TLS with explicit common linkage */
__thread char tls_common_char __attribute__((common));

/* Forward declaration of extern TLS from another TU */
extern __thread int extern_tls_int;

/* Function to force address taking and declaration duplication */
static __attribute__((noinline)) 
void use_tls_addresses(int *a, long *b, double *c, char *d, int *e) {
    /* Use inline assembly to ensure addresses are used */
    asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e) : "memory");
}

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_values(void) {
    static int initialized = 0;
    if (!initialized) {
        srand(12345);
        tls_static_double = (double)rand() / RAND_MAX;
        tls_common_char = 'A' + (rand() % 26);
        initialized = 1;
    }
}

/* Complex statement expression that may trigger declaration cloning */
#define GET_TLS_ADDR(var) ({ \
    typeof(var) *ptr = &(var); \
    asm volatile ("" : "+r"(ptr)); \
    ptr; \
})

int main(void) {
    /* Local TLS variable - may trigger duplication in local context */
    __thread int local_tls_int __attribute__((used)) = 0;
    
    /* Take addresses in various ways to trigger declaration cloning */
    
    /* 1. Direct address taking */
    int *p1 = &tls_ext_int;
    
    /* 2. Address through statement expression (may create new context) */
    long *p2 = GET_TLS_ADDR(tls_weak_long);
    
    /* 3. Address of static TLS */
    double *p3 = &tls_static_double;
    
    /* 4. Address of common TLS */
    char *p4 = &tls_common_char;
    
    /* 5. Address of extern TLS */
    int *p5 = &extern_tls_int;
    
    /* 6. Address of local TLS */
    int *p6 = &local_tls_int;
    
    /* Force all addresses to be used in a noinline function */
    use_tls_addresses(p1, (long*)p2, p3, p4, p5);
    
    /* Complex use that may trigger declaration duplication */
    local_tls_int = tls_ext_int + *p2 + (int)(*p3 * 100) + *p4 + extern_tls_int;
    
    /* Use GNU statement expression with multiple TLS references */
    int result = ({
        int sum = tls_ext_int;
        sum += tls_weak_long;
        sum += (int)tls_static_double;
        sum += tls_common_char;
        sum += extern_tls_int;
        sum += local_tls_int;
        asm volatile ("" : "+r"(sum));
        sum;
    });
    
    printf("TLS checksum: %d\n", result);
    
    /* Additional: take address in inline assembly directly */
    asm volatile ("" : : "m"(tls_ext_int), "m"(tls_weak_long));
    
    return 0;
}
