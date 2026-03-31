/* main.c - Primary file with multiple code patterns */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare external weak function from helper.c */
extern void weak_helper_function(void) __attribute__((weak));

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that may need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression with multiple operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si *volatile ptr = &result;
    (void)ptr;
}

/* Function using AVX512 check */
void check_and_use_avx512(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Check for AVX512 support - may generate helper */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported\n");
        
        /* Use vector extensions with complex operations */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
        
        /* Trigonometric approximation using vector operations */
        for (int i = 0; i < 4; i++) {
            /* Taylor series approximation for sin(x) */
            float x = vec1[i];
            float term = x;
            float sum = term;
            
            for (int n = 1; n < 5; n++) {
                term = -term * x * x / ((2*n) * (2*n+1));
                sum += term;
            }
            vec2[i] = sum;
        }
        
        volatile v4sf *volatile vptr = &vec2;
        (void)vptr;
    } else {
        printf("AVX512F not supported\n");
    }
}

/* Function with transactional memory */
void transactional_operation(void) {
    /* GCC Transactional Memory - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested memory access pattern */
        int local_array[256];
        for (int i = 0; i < 256; i++) {
            local_array[i] = i * global_counter;
        }
        
        /* Use the array to prevent optimization */
        volatile int *volatile arr_ptr = local_array;
        (void)arr_ptr;
    }
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 1000;
    int *data = (int*)malloc(n * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offloading - may generate fallback helpers */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2 + 1;
        }
    }
    
    /* Use result */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    
    free(data);
}

/* ARM-specific built-in if compiled for ARM */
#ifdef __arm__
void arm_specific_operations(void) {
    /* Use ARM-specific built-ins */
    unsigned int result;
    __asm__ volatile ("mrc p15, 0, %0, c0, c0, 0" : "=r" (result));
    
    /* Alternative using GCC builtin if available */
    #ifdef __builtin_arm_mrc
    result = __builtin_arm_mrc(15, 0, 0, 0, 0);
    #endif
    
    volatile unsigned int *volatile rptr = &result;
    (void)rptr;
}
#endif

/* PowerPC-specific if compiled for PPC */
#ifdef __powerpc__
void ppc_specific_operations(void) {
    /* Use PowerPC built-in */
    #ifdef __builtin_ppc_mtfsf
    __builtin_ppc_mtfsf(0xFF, 0);
    #endif
}
#endif

/* AArch64-specific if compiled for AArch64 */
#ifdef __aarch64__
void aarch64_specific_operations(void) {
    /* Use AArch64 built-in */
    #ifdef __builtin_aarch64_get_fpcr
    unsigned long fpcr = __builtin_aarch64_get_fpcr();
    volatile unsigned long *volatile fptr = &fpcr;
    (void)fptr;
    #endif
}
#endif

int main(void) {
    printf("Starting target hook test program\n");
    
    /* Path 1: Check CPU features and use vector operations */
    check_and_use_avx512();
    
    /* Path 2: Use AVX2 vector operations */
    avx2_vector_operations();
    
    /* Path 3: Transactional memory operations */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    printf("Global counter: %d\n", global_counter);
    
    /* Path 4: OpenMP offloading attempt */
    openmp_offload_attempt();
    
    /* Path 5: Call weak function that may be aliased */
    if (weak_helper_function) {
        weak_helper_function();
    } else {
        printf("Weak helper function not available\n");
    }
    
    /* Path 6: Architecture-specific operations */
    #ifdef __arm__
    arm_specific_operations();
    #elif defined(__powerpc__)
    ppc_specific_operations();
    #elif defined(__aarch64__)
    aarch64_specific_operations();
    #endif
    
    /* Large stack allocation with function calls for stack protector */
    {
        char large_buffer[4096];
        memset(large_buffer, 0, sizeof(large_buffer));
        
        /* Use buffer in complex way */
        for (int i = 0; i < sizeof(large_buffer); i++) {
            large_buffer[i] = (char)(i % 256);
        }
        
        volatile char *volatile buf_ptr = large_buffer;
        (void)buf_ptr;
    }
    
    printf("Program completed\n");
    return 0;
}
