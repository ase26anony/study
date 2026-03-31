/* tls_main.c - Main file with TLS declarations and complex usage patterns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void use_tls_pointers(int *a, long *b, double *c);
extern int compute_checksum(void);

/* External TLS declarations from other TU */
extern __thread int extern_tls_int;
extern __thread long extern_tls_long __attribute__((weak));

/* TLS with various attributes to set flags in DECL */
/* DECL_PRESERVE_P, TREE_USED, TREE_PUBLIC, DECL_EXTERNAL, DECL_COMMON */
__thread int global_tls_int __attribute__((used, visibility("default")));

/* DECL_WEAK, DECL_VISIBILITY, DECL_VISIBILITY_SPECIFIED */
__thread long global_tls_long __attribute__((weak, visibility("hidden")));

/* Static TLS - file scope, not TREE_PUBLIC */
static __thread double static_tls_double;

/* TLS with DLL import attribute simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int dllimport_tls_int;
#else
/* Simulate similar attribute on non-Windows */
__thread int dllimport_tls_int __attribute__((visibility("protected")));
#endif

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor)) 
static void init_tls_values(void) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    global_tls_int = rand() % 1000;
    global_tls_long = rand() % 10000;
    static_tls_double = (double)rand() / RAND_MAX * 100.0;
    
    /* Initialize extern TLS that will be defined in another TU */
    extern_tls_int = rand() % 500;
}

/* Opaque function that forces address taking and declaration duplication */
static __attribute__((noinline, used))
void manipulate_tls_addresses(void) {
    /* Take addresses of TLS variables - forces them to be marked used */
    int *p1 = &global_tls_int;
    long *p2 = &global_tls_long;
    double *p3 = &static_tls_double;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(p1), "r"(p2), "r"(p3) : "memory");
    
    /* Complex statement expression that may trigger declaration cloning */
    int result = ({
        int temp = *p1 + (int)*p2;
        /* Nested use of TLS address */
        double *inner_ptr = &static_tls_double;
        temp += (int)(*inner_ptr);
        temp;
    });
    
    /* Force use of result */
    __asm__ volatile ("" : : "r"(result) : "memory");
}

/* Another function that takes TLS addresses as parameters */
static __attribute__((noinline))
void process_tls_pointers(int *a, long *b, double *c) {
    /* Complex operations that may cause declaration duplication */
    *a += 1;
    *b += *a;
    *c += *b;
    
    /* Use GNU statement expression with address of parameter */
    int val = ({
        int sum = *a;
        sum += *b;
        sum += (int)*c;
        sum;
    });
    
    __asm__ volatile ("" : : "r"(val) : "memory");
}

int main(void) {
    /* Local TLS variable in main - may have different context */
    __thread int local_tls_int = 42;
    
    /* Force address taking of local TLS */
    int *local_ptr = &local_tls_int;
    
    /* Call functions that manipulate TLS addresses */
    manipulate_tls_addresses();
    
    /* Pass TLS addresses to opaque function */
    process_tls_pointers(&global_tls_int, &global_tls_long, &static_tls_double);
    
    /* Use statement expression with TLS variable */
    int complex_result = ({
        /* Reference TLS variable in nested scope */
        __thread int nested_tls __attribute__((used)) = local_tls_int;
        int *nested_ptr = &nested_tls;
        
        /* Multiple uses of TLS addresses */
        *nested_ptr += global_tls_int;
        *nested_ptr += extern_tls_int;
        *nested_ptr;
    });
    
    /* Use the TLS variable addresses in inline assembly */
    __asm__ volatile (
        "addl %1, %0\n\t"
        : "+r"(complex_result)
        : "r"(global_tls_int)
        : "cc"
    );
    
    /* Compute and print checksum to prevent optimization */
    int checksum = complex_result + global_tls_int + global_tls_long + 
                   (int)static_tls_double + extern_tls_int;
    
    printf("TLS checksum: %d\n", checksum);
    printf("Values: global_int=%d, global_long=%ld, static_double=%.2f, extern_int=%d\n",
           global_tls_int, global_tls_long, static_tls_double, extern_tls_int);
    
    return 0;
}
