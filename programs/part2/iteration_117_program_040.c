/* main.c - Primary file with complex operations to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_init(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
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
void check_avx512_features(void) {
    /* Force __builtin_cpu_init and __builtin_cpu_supports */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported\n");
        
        /* Use pragma to switch target */
        #pragma GCC push_options
        #pragma GCC target("avx512f")
        {
            /* Complex vector operation that might need helper */
            typedef float v16sf __attribute__((vector_size(64)));
            v16sf v1 = {0}, v2 = {0};
            for (int i = 0; i < 16; i++) {
                v1[i] = i * 1.5f;
                v2[i] = i * 2.5f;
            }
            volatile v16sf v3 = v1 * v2 + v1 / (v2 + 1.0f);
            (void)v3;
        }
        #pragma GCC pop_options
    } else {
        printf("AVX512F not supported\n");
    }
}

/* Transactional memory function */
void transactional_operation(void) {
    /* This requires -fgnu-tm flag */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local = global_counter;
        for (int i = 0; i < 10; i++) {
            local += i;
        }
        global_counter = local;
    }
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
    
    /* Use result */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    (void)sum;
    
    free(array);
}

/* ARM-specific if compiled for ARM */
#ifdef __arm__
void arm_specific_operations(void) {
    /* Use ARM-specific builtins */
    unsigned int result;
    __asm__ volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(result));
    
    /* Complex expression with builtin */
    volatile unsigned int x = __builtin_arm_mrc(15, 0, 0, 0, 0);
    (void)x;
}
#endif

/* PowerPC-specific if compiled for PowerPC */
#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* Use PowerPC-specific builtins */
    double d = 3.14;
    __builtin_ppc_mtfsf(0xFF, d);
    
    /* Complex FP operation */
    volatile double result = __builtin_sqrt(d);
    (void)result;
}
#endif

int main(void) {
    printf("Starting target hook test program\n");
    
    /* 1. Check CPU features and use vector extensions */
    check_avx512_features();
    
    /* 2. Use AVX2 vector operations */
    avx2_vector_operations();
    
    /* 3. Call weak function from helper.c */
    if (&target_helper_init) {
        target_helper_init();
    }
    
    /* 4. Use transactional memory */
    for (int i = 0; i < 5; i++) {
        transactional_operation();
    }
    printf("Global counter: %d\n", global_counter);
    
    /* 5. Attempt OpenMP offload */
    openmp_offload_attempt();
    
    /* 6. Architecture-specific operations */
    #ifdef __arm__
    arm_specific_operations();
    #elif defined(__powerpc__)
    powerpc_specific_operations();
    #endif
    
    /* Large stack array to trigger stack protection helpers */
    char large_buffer[4096];
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i % 256;
    }
    
    /* Use buffer to prevent optimization */
    volatile int checksum = 0;
    for (int i = 0; i < sizeof(large_buffer); i++) {
        checksum += large_buffer[i];
    }
    (void)checksum;
    
    printf("Program completed\n");
    return 0;
}
