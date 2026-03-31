/* main.c - Primary file with complex operations to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) weak_helper_function(void);

/* Global variables for transactional memory */
volatile int global_counter = 0;
volatile double global_data[1024];

/* Vector types using GCC extensions */
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
    volatile v4si *volatile_ptr = &result;
    (void)volatile_ptr;
}

/* Function using CPU feature detection */
void cpu_feature_dependent_code(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for various features - may generate helper calls */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Use the results in a way that can't be optimized away */
    volatile int features = has_avx512 + has_avx2 + has_sse4;
    (void)features;
    
    if (has_avx2) {
        /* Switch to AVX2 target pragma */
        #pragma GCC push_options
        #pragma GCC target("avx2")
        {
            v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
            v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
            
            /* Complex floating-point vector operation */
            v4sf result = vec1 * vec2 + vec1 / (vec2 + 1.0f);
            
            volatile v4sf *vptr = &result;
            (void)vptr;
        }
        #pragma GCC pop_options
    }
}

/* Transactional memory function */
void transactional_operation(void) {
    /* Use GNU Transactional Memory - requires runtime support */
    __transaction_atomic {
        global_counter++;
        global_data[global_counter % 1024] = (double)global_counter;
    }
    
    /* Nested transaction */
    __transaction_atomic {
        for (int i = 0; i < 10; i++) {
            __transaction_relaxed {
                global_data[i] *= 1.1;
            }
        }
    }
}

/* OpenMP offloading attempt */
void openmp_offload_attempt(void) {
    int data[1000];
    
    /* Initialize data */
    for (int i = 0; i < 1000; i++) {
        data[i] = i;
    }
    
    /* Attempt offloading - will likely generate fallback helpers */
    #pragma omp target map(tofrom: data[0:1000]) device(0)
    #pragma omp teams distribute parallel for
    for (int i = 0; i < 1000; i++) {
        data[i] = data[i] * 2 + 1;
    }
    
    /* Use result */
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* Function with large stack for stack protector */
void function_with_large_stack(void) {
    /* Large array to trigger stack protection */
    char large_buffer[4096];
    int another_buffer[2048];
    
    /* Initialize with pattern */
    for (int i = 0; i < 4096; i++) {
        large_buffer[i] = i % 256;
    }
    
    /* Complex operations on large buffers */
    for (int i = 0; i < 2048; i++) {
        another_buffer[i] = large_buffer[i * 2] + large_buffer[i * 2 + 1];
    }
    
    /* Use results */
    volatile int total = 0;
    for (int i = 0; i < 2048; i++) {
        total += another_buffer[i];
    }
    (void)total;
}

int main(void) {
    printf("Starting target hook triggering program...\n");
    
    /* 1. CPU feature detection and vector operations */
    cpu_feature_dependent_code();
    
    /* 2. AVX2-specific vector operations */
    avx2_vector_operations();
    
    /* 3. Transactional memory operations */
    transactional_operation();
    
    /* 4. OpenMP offloading attempt */
    openmp_offload_attempt();
    
    /* 5. Function with large stack for stack protector */
    function_with_large_stack();
    
    /* 6. Call weak helper function from another compilation unit */
    if (&weak_helper_function) {
        weak_helper_function();
    }
    
    /* 7. Additional ARM/PowerPC built-in simulation */
    #if defined(__arm__) || defined(__aarch64__)
    {
        /* ARM system register access - may need helper */
        unsigned long fpcr = 0;
        #ifdef __aarch64__
        fpcr = __builtin_aarch64_get_fpcr();
        #else
        fpcr = __builtin_arm_mrc(15, 7, 0, 5, 0);
        #endif
        volatile unsigned long v = fpcr;
        (void)v;
    }
    #elif defined(__powerpc__) || defined(__ppc__)
    {
        /* PowerPC special register access */
        unsigned long fpscr = 0;
        __builtin_ppc_mtfsf(0xFF, fpscr);
    }
    #endif
    
    /* 8. More complex vector math that might need libcalls */
    {
        v4si v1 = {100, 200, 300, 400};
        v4si v2 = {5, 6, 7, 8};
        
        /* Complex operation that might need runtime support */
        v4si result = (v1 % v2) | (v1 & ~v2) ^ (v1 << (v2 % 4));
        
        volatile v4si *vr = &result;
        (void)vr;
    }
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
