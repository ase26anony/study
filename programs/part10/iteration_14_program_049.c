/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force emulated TLS usage */
#ifdef __APPLE__
#define TLS_MODEL __attribute__((tls_model("emulated")))
#else
#define TLS_MODEL
#endif

/* Declaration with multiple attributes to be copied */
__attribute__((used, visibility("default"), weak))
extern __thread int extern_tls_var TLS_MODEL;

/* Static TLS with hidden visibility */
__attribute__((visibility("hidden")))
static __thread long static_hidden_tls TLS_MODEL = 42;

/* Regular TLS with common linkage */
__thread double regular_tls TLS_MODEL = 3.14159;

/* Weak TLS variable */
__attribute__((weak))
__thread char weak_tls_var TLS_MODEL;

/* Function to take address and force declaration cloning */
static __attribute__((noinline, used)) 
void manipulate_tls_addresses(int* a, long* b, double* c, char* d) {
    /* Opaque operations to prevent optimization */
    __asm__ volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
    
    /* Force use of values */
    *a += 1;
    *b += *a;
    *c += *b;
    *d = (char)(*c);
}

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    regular_tls = (double)rand() / RAND_MAX;
    weak_tls_var = (char)(rand() % 256);
    
    /* Initialize extern variable if we're the defining module */
    extern_tls_var = rand() % 1000;
}

/* Complex statement expression that may trigger declaration cloning */
#define GET_TLS_ADDR_AND_COMPUTE(var) \
    ({ \
        __typeof__(var) *ptr = &(var); \
        __typeof__(var) val = *ptr; \
        /* Call external function to create complex context */ \
        extern int external_helper(void*); \
        external_helper(ptr); \
        val; \
    })

/* External function declaration */
int external_helper(void* ptr) {
    return *(int*)ptr % 256;
}

int main(void) {
    /* Local TLS variable - may trigger duplication in local scope */
    __thread int local_tls TLS_MODEL = 100;
    
    /* Take addresses of various TLS variables */
    int* p1 = &extern_tls_var;
    long* p2 = &static_hidden_tls;
    double* p3 = &regular_tls;
    char* p4 = &weak_tls_var;
    int* p5 = &local_tls;
    
    /* Force declaration cloning through address manipulation */
    manipulate_tls_addresses(p1, (int*)p2, (double*)p3, p4);
    
    /* Use statement expression macro which may clone declarations */
    double computed = GET_TLS_ADDR_AND_COMPUTE(regular_tls);
    
    /* Complex computation using all TLS variables */
    long checksum = (long)(*p1) + (long)(*p2) + (long)(*p3) + (long)(*p4) + (long)(*p5);
    checksum += (long)(computed * 1000);
    
    /* Use inline assembly with TLS address to force duplication */
    __asm__ volatile (
        "addl $1, %0\n\t"
        : "+m" (extern_tls_var)
        :
        : "cc"
    );
    
    printf("TLS checksum: %ld\n", checksum);
    printf("extern_tls_var: %d\n", extern_tls_var);
    printf("regular_tls: %f\n", regular_tls);
    
    return 0;
}
