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

/* Declare TLS variables with different attributes */
/* Static TLS with hidden visibility and used attribute */
static __thread int tls_static_hidden TLS_MODEL 
    __attribute__((used, visibility("hidden"))) = 42;

/* Weak TLS variable */
__thread long tls_weak TLS_MODEL __attribute__((weak)) = 100;

/* TLS with default visibility */
__thread double tls_default TLS_MODEL 
    __attribute__((visibility("default"))) = 3.14159;

/* External TLS declaration (defined in another file) */
extern __thread char tls_extern TLS_MODEL;

/* Function to take address and force declaration duplication */
static __attribute__((noinline)) void use_tls_address(void* addr1, void* addr2) {
    /* Inline assembly to ensure addresses are used */
    __asm__ volatile ("" : : "r"(addr1), "r"(addr2) : "memory");
}

/* Constructor to initialize TLS variables */
__attribute__((constructor)) static void init_tls(void) {
    srand(time(NULL));
    tls_static_hidden = rand() % 1000;
    tls_weak = rand() % 10000;
    tls_default = (double)rand() / RAND_MAX * 100.0;
}

/* Helper function that creates complex context for TLS */
static long complex_tls_usage(void) {
    /* Statement expression with TLS address usage */
    long result = ({
        __thread int local_tls TLS_MODEL = 123;
        int* ptr = &local_tls;
        /* Force declaration duplication through multiple uses */
        use_tls_address(&local_tls, &tls_weak);
        /* Use in arithmetic */
        *ptr += tls_static_hidden;
        (long)*ptr;
    });
    
    return result;
}

/* Another function that takes TLS addresses */
static void take_tls_addresses(void) {
    /* Take addresses of multiple TLS variables */
    void* addrs[] = {
        &tls_static_hidden,
        &tls_weak,
        &tls_default,
    };
    
    /* Use them in opaque way */
    for (int i = 0; i < 3; i++) {
        use_tls_address(addrs[i], addrs[(i + 1) % 3]);
    }
}

int main(void) {
    /* Local TLS variable in main */
    __thread int main_tls TLS_MODEL = 999;
    
    /* Force address taking and complex usage */
    take_tls_addresses();
    
    /* Use statement expression with TLS */
    long val1 = complex_tls_usage();
    
    /* More complex TLS usage pattern */
    {
        /* Nested block with TLS declaration */
        __thread int nested_tls TLS_MODEL __attribute__((used)) = 777;
        int* nested_ptr = &nested_tls;
        
        /* Mix with other TLS variables */
        *nested_ptr += tls_static_hidden + (int)tls_default;
        
        /* Force declaration context */
        use_tls_address(&main_tls, nested_ptr);
        
        val1 += *nested_ptr;
    }
    
    /* Compute checksum using all TLS variables */
    long checksum = tls_static_hidden + tls_weak + (long)tls_default + 
                    main_tls + (long)tls_extern + val1;
    
    printf("TLS checksum: %ld\n", checksum);
    
    return 0;
}
