/* main.c - Primary file with multiple coverage techniques */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with AVX2 target attribute */
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
    
    /* Use the results in conditional compilation */
    if (has_avx2) {
        #pragma GCC target("avx2")
        {
            v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
            v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
            v4sf vec3 = vec1 * vec2 + vec1 / (vec2 + 1.0f);
            volatile v4sf dummy = vec3;
            (void)dummy;
        }
    }
    
    /* Print features for demonstration */
    printf("CPU Features - AVX512: %d, AVX2: %d, SSE4.2: %d\n", 
           has_avx512, has_avx2, has_sse4);
}

/* Function with transactional memory */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation */
        v4si a = {global_counter, 2, 3, 4};
        v4si b = {5, 6, 7, 8};
        v4si result = a * b - (a / (b + 1));
        
        volatile v4si dummy = result;
        (void)dummy;
    }
    
    printf("Global counter: %d\n", global_counter);
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 100;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt offload - may generate fallback helpers */
    #pragma omp target map(tofrom: array[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Verify some results */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    printf("Array sum after offload attempt: %d\n", sum);
    
    free(array);
}

/* Function with large stack for stack protector */
void function_with_large_stack(void) {
    /* Large array to trigger stack protection */
    char buffer[1024 * 16]; /* 16KB buffer */
    
    /* Fill with pattern */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (i % 256);
    }
    
    /* Use buffer to prevent optimization */
    volatile size_t dummy = buffer[sizeof(buffer) - 1];
    (void)dummy;
    
    /* Call target helper if available */
    if (target_helper) {
        target_helper();
    }
}

int main(void) {
    printf("Starting target hook coverage program...\n");
    
    /* 1. CPU feature detection and vector operations */
    cpu_feature_dependent_code();
    
    /* 2. AVX2-specific operations */
    avx2_vector_operations();
    
    /* 3. Transactional memory operations */
    for (int i = 0; i < 3; i++) {
        transactional_operation();
    }
    
    /* 4. OpenMP offload attempt */
    openmp_offload_attempt();
    
    /* 5. Large stack function (stack protector) */
    function_with_large_stack();
    
    /* 6. Additional architecture-specific built-ins */
    #if defined(__aarch64__)
    /* ARM-specific built-in */
    unsigned long fpcr = __builtin_aarch64_get_fpcr();
    printf("FPCR register: %lx\n", fpcr);
    #elif defined(__powerpc__)
    /* PowerPC-specific built-in */
    double d = 3.14;
    __builtin_ppc_mtfsf(0xFF, d);
    #elif defined(__arm__)
    /* ARM (32-bit) specific built-in */
    unsigned int cp15;
    __builtin_arm_mrc(15, 0, 0, 1, 0);
    #endif
    
    printf("Program completed.\n");
    return 0;
}
