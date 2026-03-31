/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force emulated TLS */
#pragma GCC tls_model emulated

/* TLS with different attributes to be copied */
/* 1. Static TLS with used attribute */
static __thread int tls_static_used __attribute__((used)) = 42;

/* 2. TLS with weak linkage */
__thread int tls_weak_var __attribute__((weak)) = 100;

/* 3. TLS with hidden visibility */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 999;

/* 4. External TLS declaration (defined in another file) */
extern __thread double tls_extern;

/* 5. TLS with dllimport attribute (or fallback) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar behavior with weak extern */
extern __thread int tls_imported __attribute__((weak));
#endif

/* 6. Public TLS with explicit visibility */
__thread float tls_public __attribute__((visibility("default")));

/* Helper function to force address taking and duplication */
static __attribute__((noinline)) 
void use_tls_address(void *addr1, void *addr2, void *addr3) {
    /* Opaque operations to prevent optimization */
    __asm__ volatile ("" : : "r"(addr1), "r"(addr2), "r"(addr3) : "memory");
}

/* Another helper that returns TLS address through complex expression */
static __attribute__((noinline))
void *get_tls_address_complex(__thread int *ptr) {
    /* Statement expression that might trigger declaration cloning */
    return ({ 
        void *result = ptr;
        /* Force reference in different context */
        __asm__ volatile ("" : "+r"(result) : : "memory");
        result;
    });
}

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    tls_static_used = rand() % 1000;
    tls_hidden = rand() % 10000;
    tls_public = (float)rand() / RAND_MAX;
}

/* Function that uses TLS in ways that might trigger duplication */
static void manipulate_tls(void) {
    /* Local TLS variable - might get duplicated in this scope */
    __thread int tls_local = 123;
    
    /* Take address and use in inline asm - forces declaration reference */
    int *addr1 = &tls_static_used;
    long *addr2 = &tls_hidden;
    float *addr3 = &tls_public;
    int *addr4 = &tls_local;
    
    /* Use addresses in ways that might trigger declaration cloning */
    use_tls_address(addr1, addr2, addr3);
    
    /* Complex expression with TLS address */
    void *complex_addr = get_tls_address_complex(&tls_local);
    (void)complex_addr;
    
    /* Use TLS variables in arithmetic */
    tls_local = tls_static_used * 2 + tls_hidden % 100;
    tls_public = tls_public * 2.0f + (float)tls_local;
    
    /* More address manipulation */
    __thread int *tls_ptr = &tls_local;
    __asm__ volatile ("" : "+r"(tls_ptr) : : "memory");
}

/* Another function with different TLS usage pattern */
static void more_tls_operations(void) {
    /* Different local TLS */
    __thread double tls_local2 = 3.14159;
    
    /* Mix with external TLS */
    tls_local2 += tls_extern;
    
    /* Force address taking with assembly */
    double *addr = &tls_local2;
    __asm__ volatile ("" : "+r"(addr) : : "memory");
    
    /* Use weak TLS */
    if (&tls_weak_var) {
        tls_weak_var++;
    }
}

int main(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Call functions that manipulate TLS */
    manipulate_tls();
    more_tls_operations();
    
    /* Compute checksum from all TLS values */
    long checksum = 0;
    checksum += tls_static_used;
    checksum += tls_hidden;
    checksum += (long)tls_public;
    checksum += (long)tls_extern;
    
    /* Use imported TLS if available */
    if (&tls_imported) {
        checksum += tls_imported;
    }
    
    printf("TLS checksum: %ld\n", checksum);
    
    /* Force one more complex TLS address expression */
    {
        __thread int final_tls = checksum % 100;
        /* GNU statement expression with call */
        int result = ({
            int temp = final_tls;
            use_tls_address(&temp, &tls_static_used, &tls_hidden);
            temp * 2;
        });
        printf("Final result: %d\n", result);
    }
    
    return 0;
}
