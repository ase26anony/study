/* main.c - Primary file with complex operations to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_init(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with AVX2 target attribute - may require runtime checks */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression with multiple operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si dummy = result;
    (void)dummy;
}

/* Function using CPU feature detection */
void cpu_feature_dependent_code(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for various features - may generate helper functions */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Use the results in conditional compilation-like pattern */
    if (has_avx512) {
        printf("AVX512 supported\n");
        #pragma GCC target("avx512f")
        {
            /* Complex vector operation with potential helper */
            v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
            v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
            v4sf result = vec1 * vec2 + vec1 / vec2;
            volatile v4sf dummy = result;
            (void)dummy;
        }
    } else if (has_avx2) {
        printf("AVX2 supported\n");
        avx2_vector_operations();
    }
    
    /* Force generation of multiple code paths */
    volatile int features = has_avx512 + has_avx2 + has_sse4;
    (void)features;
}

/* Transactional memory section */
void transactional_operation(void) {
    /* This requires -fgnu-tm flag */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local_array[256];
        for (int i = 0; i < 256; i++) {
            local_array[i] = i * global_counter;
        }
        
        /* Use array to prevent optimization */
        volatile int sum = 0;
        for (int i = 0; i < 256; i++) {
            sum += local_array[i];
        }
        (void)sum;
    }
}

/* OpenMP offloading attempt */
void openmp_offload_attempt(void) {
    int n = 1000;
    int *array = (int *)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt offload - may generate fallback helpers */
    #pragma omp target teams distribute parallel for map(tofrom: array[0:n]) device(0)
    for (int i = 0; i < n; i++) {
        array[i] = array[i] * 2 + 1;
    }
    
    /* Use result */
    volatile int check = array[n-1];
    (void)check;
    
    free(array);
}

/* ARM-specific built-in if compiled for ARM */
#ifdef __arm__
void arm_specific_operations(void) {
    /* Use ARM-specific built-ins */
    unsigned int result = __builtin_arm_mrc(15, 0, 0, 0, 0);
    volatile unsigned int dummy = result;
    (void)dummy;
}
#endif

/* PowerPC-specific if compiled for PPC */
#ifdef __powerpc__
void ppc_specific_operations(void) {
    /* Use PowerPC-specific built-ins */
    __builtin_ppc_mtfsf(0xFF, 1.0);
}
#endif

/* AArch64-specific if compiled for AArch64 */
#ifdef __aarch64__
void aarch64_specific_operations(void) {
    /* Use AArch64-specific built-ins */
    unsigned long result = __builtin_aarch64_get_fpcr();
    volatile unsigned long dummy = result;
    (void)dummy;
}
#endif

int main(void) {
    printf("Starting target hook trigger program\n");
    
    /* 1. CPU feature detection and vector operations */
    cpu_feature_dependent_code();
    
    /* 2. Call weak external function that uses target built-ins */
    if (target_helper_init) {
        target_helper_init();
    }
    
    /* 3. Transactional memory operation */
    transactional_operation();
    
    /* 4. OpenMP offload attempt */
    openmp_offload_attempt();
    
    /* 5. Architecture-specific operations */
    #ifdef __arm__
    arm_specific_operations();
    #endif
    
    #ifdef __powerpc__
    ppc_specific_operations();
    #endif
    
    #ifdef __aarch64__
    aarch64_specific_operations();
    #endif
    
    /* Large stack array to trigger stack protection helpers */
    char large_buffer[4096];
    memset(large_buffer, 0, sizeof(large_buffer));
    
    /* Use buffer to prevent optimization */
    volatile char *buf_ptr = large_buffer;
    buf_ptr[0] = 'X';
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
