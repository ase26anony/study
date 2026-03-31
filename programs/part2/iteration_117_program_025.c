/* main.c - Primary file with complex patterns to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
int global_counter = 0;

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Declare external weak function */
extern void weak_constructor_func(void) __attribute__((weak));

/* AVX2 target-specific function */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Division on integer vectors often requires helper functions */
    v4si c = a / (b + 1);
    
    /* Mix with floating point for more complexity */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 0.25f, 0.125f, 0.0625f};
    v4sf fc = fa / fb;
    
    /* Prevent dead code elimination */
    volatile v4si temp = c;
    volatile v4sf ftemp = fc;
    (void)temp;
    (void)ftemp;
}

/* Function using CPU feature detection */
void cpu_feature_dependent_code(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check multiple features to increase chance of helper generation */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    volatile int result = has_avx512 + has_avx2 + has_sse4;
    (void)result;
    
    if (has_avx2) {
        avx2_vector_operations();
    }
}

/* OpenMP target region - will likely generate fallback helpers */
void openmp_offload_attempt(void) {
    int data[100];
    
    #pragma omp target map(tofrom: data[0:100]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] = i * 2;
        }
    }
    
    /* Use data to prevent elimination */
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* Transactional memory section */
void transactional_operation(void) {
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local_array[256];
        for (int i = 0; i < 256; i++) {
            local_array[i] = i + global_counter;
        }
        
        /* Use volatile to prevent optimization */
        volatile int* ptr = local_array;
        (void)ptr;
    }
}

/* Large stack usage for stack protector */
void large_stack_function(void) {
    char large_buffer[4096];  /* Large stack for stack protector */
    
    /* Fill with pattern */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i % 256;
    }
    
    /* Call external function that might trigger builtins */
    if (weak_constructor_func) {
        weak_constructor_func();
    }
    
    /* Use buffer to prevent elimination */
    volatile char check = large_buffer[100];
    (void)check;
}

int main(void) {
    printf("Starting target hook trigger program...\n");
    
    /* Execute all patterns in sequence */
    cpu_feature_dependent_code();
    
    openmp_offload_attempt();
    
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    
    large_stack_function();
    
    printf("Global counter: %d\n", global_counter);
    printf("Program completed.\n");
    
    return 0;
}
