/* main.c - Primary file with multiple techniques to trigger target hooks */
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

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
static void avx2_vector_operations(void) {
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

/* Function using AVX512 check */
static void check_avx512_features(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for AVX512 - may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F is supported\n");
        
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
        
        volatile v4sf dummy = vec2;
        (void)dummy;
    } else {
        printf("AVX512F not supported\n");
    }
}

/* Function with large stack for stack protector */
static void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char large_buffer[1024 * 16];
    
    /* Fill with pattern */
    for (size_t i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = (char)(i % 256);
    }
    
    /* Use buffer to prevent optimization */
    volatile size_t sum = 0;
    for (size_t i = 0; i < sizeof(large_buffer); i++) {
        sum += large_buffer[i];
    }
    
    /* Call external function that might be optimized away */
    if (target_helper_init) {
        target_helper_init();
    }
}

/* OpenMP target region */
static void openmp_offload_attempt(void) {
    int data[100];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - may generate fallback functions */
    #pragma omp target map(tofrom: data[0:100]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] *= 2;
        }
    }
    
    /* Verify some results */
    volatile int check = data[50];
    (void)check;
}

/* Transactional memory function */
static void transactional_operation(void) {
    /* Transactional memory block */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation */
        v4si a = {global_counter, global_counter + 1, 
                  global_counter + 2, global_counter + 3};
        v4si b = a * 2;
        
        volatile v4si dummy = b;
        (void)dummy;
    }
}

int main(void) {
    printf("Starting target hook triggering program\n");
    
    /* 1. Check CPU features and use vector extensions */
    check_avx512_features();
    
    /* 2. Use AVX2 vector operations */
    avx2_vector_operations();
    
    /* 3. Function with large stack for stack protection */
    large_stack_function();
    
    /* 4. Attempt OpenMP offload */
    openmp_offload_attempt();
    
    /* 5. Use transactional memory */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    
    /* 6. ARM/PowerPC specific built-ins (compile-time guarded) */
    #if defined(__arm__) || defined(__aarch64__)
    {
        /* ARM system register access */
        unsigned int fpcr = __builtin_aarch64_get_fpcr();
        volatile unsigned int dummy = fpcr;
        (void)dummy;
    }
    #elif defined(__powerpc__) || defined(__ppc__)
    {
        /* PowerPC special register access */
        __builtin_ppc_mtfsf(0xFF, 0);
    }
    #endif
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
