/* main.c - Primary file with multiple coverage techniques */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    volatile v4si *volatile_ptr = &result;
    (void)volatile_ptr;
}

/* Function using AVX512 check */
void check_avx512_features(void) {
    /* Force __builtin_cpu_init call */
    __builtin_cpu_init();
    
    /* Check multiple CPU features - may generate helper functions */
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512vl = __builtin_cpu_supports("avx512vl");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (has_avx512f && has_avx512vl && has_avx512bw) {
        /* Use pragma to switch target */
        #pragma GCC push_options
        #pragma GCC target("avx512f,avx512vl,avx512bw")
        {
            /* Complex vector operations with 512-bit vectors */
            typedef float v16sf __attribute__((vector_size(64)));
            v16sf v1 = {0}, v2 = {0};
            /* Simulate complex computation */
            for (int i = 0; i < 16; i++) {
                v1[i] = i * 1.5f;
                v2[i] = i * 2.5f;
            }
            volatile v16sf v3 = v1 * v2 + v1 / (v2 + 1.0f);
            (void)v3;
        }
        #pragma GCC pop_options
    }
}

/* Function with transactional memory */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
        volatile v4sf result = vec1 * vec2 - vec1 / vec2;
        (void)result;
    }
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 1000;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt offload - will likely generate fallback helpers */
    #pragma omp target map(tofrom: array[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] = array[i] * 2 + 1;
        }
    }
    
    /* Use result to prevent elimination */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    (void)sum;
    
    free(array);
}

/* Function with large stack for stack protector */
void function_with_large_stack(void) {
    /* Large array to trigger stack protection */
    char large_buffer[4096];
    int another_buffer[512];
    
    /* Use buffers to prevent optimization */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i & 0xFF;
    }
    
    /* Complex operation mixing with builtins */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse4.2")) {
        for (int i = 0; i < 512; i++) {
            another_buffer[i] = i * (large_buffer[i % 4096] + 1);
        }
    }
    
    volatile int check = another_buffer[100];
    (void)check;
}

/* ARM-specific builtins (if compiled for ARM) */
#ifdef __arm__
void arm_specific_operations(void) {
    /* ARM system register access */
    unsigned int val;
    __asm__ volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(val));
    
    /* Use GCC builtin */
    val = __builtin_arm_mrc(15, 0, 0, 0, 0);
    volatile unsigned int *vp = &val;
    (void)vp;
}
#endif

/* PowerPC-specific builtins (if compiled for PowerPC) */
#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* PowerPC special register access */
    unsigned long fpscr = 0;
    __builtin_ppc_mtfsf(0xFF, fpscr);
    
    volatile unsigned long *vp = &fpscr;
    (void)vp;
}
#endif

/* Main function orchestrating all coverage paths */
int main(void) {
    printf("Starting target hook coverage test...\n");
    
    /* Path 1: CPU feature detection and vector operations */
    check_avx512_features();
    avx2_vector_operations();
    
    /* Path 2: Transactional memory */
    for (int i = 0; i < 5; i++) {
        transactional_operation();
    }
    printf("Global counter: %d\n", global_counter);
    
    /* Path 3: OpenMP offload attempt */
    openmp_offload_attempt();
    
    /* Path 4: Large stack with protection */
    function_with_large_stack();
    
    /* Path 5: Weak function call (from separate compilation unit) */
    if (&weak_helper_function) {
        weak_helper_function();
    }
    
    /* Architecture-specific paths */
    #ifdef __arm__
    arm_specific_operations();
    #elif defined(__powerpc__)
    powerpc_specific_operations();
    #endif
    
    /* Final check with mixed operations */
    {
        v4si final_vec = {1, 2, 3, 4};
        v4si mask = {0, -1, 0, -1};
        volatile v4si masked = final_vec & mask;
        
        /* Transaction with vector operation */
        __transaction_atomic {
            for (int i = 0; i < 4; i++) {
                final_vec[i] += masked[i];
            }
        }
    }
    
    printf("Test completed.\n");
    return 0;
}
