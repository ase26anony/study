/* Main file with various TLS declarations and attribute manipulations */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations */
extern void opaque_operation(void *ptr1, void *ptr2);
extern int external_tls_var;  /* Defined in another TU */

/* TLS variables with different attributes to trigger attribute copying */

/* Static TLS with hidden visibility and used attribute */
static __thread int static_hidden_tls __attribute__((used, visibility("hidden"))) = 42;

/* Weak TLS variable that might be overridden */
__thread long weak_tls_var __attribute__((weak)) = 100;

/* TLS with default visibility and DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread double imported_tls;
#else
/* Simulate similar attributes on non-Windows */
__thread double imported_tls __attribute__((visibility("default")));
#endif

/* Common TLS (might become DECL_COMMON) */
__thread int common_tls;

/* External TLS declaration - defined in another file */
extern __thread char external_tls_char;

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    static_hidden_tls = rand() % 1000;
    weak_tls_var = rand() % 5000;
    imported_tls = (double)rand() / RAND_MAX * 100.0;
    common_tls = rand() % 2000;
}

/* Noinline function to force address taking and potential declaration cloning */
static __attribute__((noinline, used))
void use_tls_addresses(void) {
    /* Take addresses of TLS variables - may trigger declaration duplication */
    int *p1 = &static_hidden_tls;
    long *p2 = &weak_tls_var;
    double *p3 = &imported_tls;
    int *p4 = &common_tls;
    extern char *get_external_tls_addr(void);
    char *p5 = get_external_tls_addr();
    
    /* Opaque operation with inline asm to prevent optimization */
    __asm__ volatile ("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5) : "memory");
    
    /* Complex statement expression that uses TLS addresses */
    int result = ({
        int sum = *p1 + (int)*p2 + (int)*p3 + *p4 + (int)*p5;
        /* Call to external function in statement expression */
        extern int external_helper(int);
        external_helper(sum);
    });
    
    (void)result;
}

/* Another function that creates a local TLS variable in different context */
static __attribute__((noinline))
void function_with_local_tls(void) {
    /* Local TLS declaration - may be cloned when inlined or optimized */
    __thread int local_func_tls __attribute__((used)) = 123;
    
    /* Use address in inline asm - may trigger declaration duplication */
    __asm__ volatile ("# TLS var at %0" : : "r"(&local_func_tls));
    
    /* Modify based on external value */
    local_func_tls += external_tls_var;
}

int main(void) {
    /* Local TLS in main - different scope */
    __thread int main_local_tls __attribute__((used)) = 999;
    
    /* Force address taking and potential declaration cloning */
    use_tls_addresses();
    function_with_local_tls();
    
    /* Complex use of TLS addresses in statement expressions */
    int checksum = ({
        int sum = static_hidden_tls + (int)weak_tls_var + 
                  (int)imported_tls + common_tls + main_local_tls;
        
        /* Use address in asm within statement expression */
        __asm__ volatile ("# Checksum calculation: %0" : : "r"(sum));
        
        /* Call opaque function with TLS address */
        opaque_operation(&main_local_tls, &static_hidden_tls);
        
        sum;
    });
    
    printf("TLS checksum: %d\n", checksum);
    
    /* Additional complex context for TLS usage */
    {
        /* Nested block with another TLS declaration */
        static __thread int nested_tls __attribute__((visibility("hidden"))) = 456;
        
        /* Use GNU statement expression with address taking */
        int nested_result = ({
            int *addr = &nested_tls;
            /* Inline asm that references the address */
            __asm__ volatile ("# Nested TLS: %0" : : "r"(addr));
            *addr += checksum;
            *addr;
        });
        
        printf("Nested TLS result: %d\n", nested_result);
    }
    
    return 0;
}
