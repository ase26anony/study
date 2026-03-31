/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern int get_checksum(void);
extern void opaque_operation(int*, int*);

/* External TLS variable declaration (defined in another file) */
extern __thread int extern_tls_var;

/* TLS with various attributes to set the flags in the target block */
__thread int global_tls __attribute__((used, visibility("default")));
static __thread int static_tls __attribute__((used, visibility("hidden")));
__thread int weak_tls __attribute__((weak, used));
__thread int public_tls __attribute__((visibility("default")));

/* For Windows DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate similar attribute */
__thread int imported_tls __attribute__((visibility("default")));
#endif

/* Constructor to initialize TLS variables */
__attribute__((constructor))
static void init_tls(void) {
    srand(time(NULL));
    global_tls = rand() % 100;
    static_tls = rand() % 100;
    weak_tls = rand() % 100;
    public_tls = rand() % 100;
    imported_tls = rand() % 100;
}

/* Force declaration duplication through statement expressions */
#define GET_TLS_ADDR(var) ({ \
    __typeof__(var) *ptr = &(var); \
    asm volatile ("" : "+r" (ptr)); \
    ptr; \
})

/* Noinline function to force address taking and prevent optimization */
static __attribute__((noinline, used))
void use_tls_addresses(void) {
    /* Take addresses of various TLS variables */
    int *addr1 = GET_TLS_ADDR(global_tls);
    int *addr2 = GET_TLS_ADDR(static_tls);
    int *addr3 = &weak_tls;
    int *addr4 = &public_tls;
    int *addr5 = &imported_tls;
    
    /* Opaque operation that might trigger declaration cloning */
    opaque_operation(addr1, addr2);
    
    /* Use inline assembly to ensure addresses are used */
    asm volatile ("" : : "r" (addr3), "r" (addr4), "r" (addr5));
}

/* Another function that creates a local TLS variable context */
static __attribute__((noinline))
int compute_with_local_tls(void) {
    /* Local TLS variable - may trigger duplication in different context */
    static __thread int local_static_tls = 42;
    __thread int local_tls = 24;
    
    /* Complex expression using statement expression */
    int result = ({
        int *p1 = &local_static_tls;
        int *p2 = &local_tls;
        int *p3 = GET_TLS_ADDR(global_tls);
        asm volatile ("" : "+r" (p1), "+r" (p2), "+r" (p3));
        *p1 + *p2 + *p3;
    });
    
    return result;
}

int main(void) {
    /* Initialize external TLS variable */
    extern_tls_var = 100;
    
    /* Use various TLS variables */
    use_tls_addresses();
    
    /* Compute with local TLS */
    int local_result = compute_with_local_tls();
    
    /* Get checksum from another file */
    int checksum = get_checksum() + local_result;
    
    /* Use all TLS variables in computation */
    int total = global_tls + static_tls + weak_tls + public_tls + 
                imported_tls + extern_tls_var + checksum;
    
    printf("Result: %d\n", total);
    
    /* Force use of all variables to prevent optimization */
    asm volatile ("" : : "r" (global_tls), "r" (static_tls), 
                  "r" (weak_tls), "r" (public_tls), 
                  "r" (imported_tls), "r" (extern_tls_var));
    
    return 0;
}
