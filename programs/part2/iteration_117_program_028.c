/* main.c - Primary file with complex patterns to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_init(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector division approximation - may require helper */
    v4si c;
    for (int i = 0; i < 4; i++) {
        /* Simulate division using reciprocal approximation */
        c[i] = a[i] * (b[i] ? (1 << 16) / b[i] : 0);
    }
    
    /* Use result to prevent optimization */
    volatile v4si dummy = c;
    (void)dummy;
}

/* Function with ARM-specific built-in (will be weak/fallback on x86) */
__attribute__((weak, noinline))
void arm_specific_check(void) {
    /* This will only be used if the real implementation isn't linked */
    printf("Using fallback ARM check\n");
}

/* Transactional memory function */
void transactional_increment(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        if (global_counter % 2 == 0) {
            global_counter *= 2;
        }
    }
}

/* Large stack usage for stack protector */
void stack_protector_test(void) {
    char large_buffer[4096];  /* Large stack frame */
    volatile int i;
    
    /* Fill buffer to prevent optimization */
    for (i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i & 0xFF;
    }
    
    /* Use buffer content in computation */
    int sum = 0;
    for (i = 0; i < sizeof(large_buffer); i++) {
        sum += large_buffer[i];
    }
    
    volatile int result = sum;
    (void)result;
}

int main(void) {
    printf("Starting target hook test program...\n");
    
    /* 1. CPU feature detection and conditional execution */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported, running vector operations\n");
        avx2_vector_operations();
        
        /* Additional AVX512 check */
        if (__builtin_cpu_supports("avx512f")) {
            printf("AVX512F also supported\n");
            /* Complex expression with multiple builtins */
            volatile int has_avx512 = __builtin_cpu_supports("avx512f") && 
                                     __builtin_cpu_supports("avx512cd");
            (void)has_avx512;
        }
    }
    
    /* 2. OpenMP target region (will fallback to host if no device) */
    #pragma omp target map(tofrom: global_counter) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            #pragma omp atomic
            global_counter += i;
        }
    }
    
    /* 3. Transactional memory operations */
    printf("Using transactional memory, counter: %d\n", global_counter);
    for (int i = 0; i < 10; i++) {
        transactional_increment();
    }
    printf("After transactions, counter: %d\n", global_counter);
    
    /* 4. Stack protector test with large arrays */
    stack_protector_test();
    
    /* 5. Call weak external function that uses target builtins */
    if (target_helper_init) {
        target_helper_init();
    }
    
    /* 6. Call ARM-specific function (weak fallback) */
    arm_specific_check();
    
    /* 7. PowerPC builtin simulation (compile-time conditional) */
    #ifdef __powerpc__
    {
        unsigned long fpscr;
        /* This builtin requires runtime helper on PowerPC */
        __builtin_ppc_mtfsf(0xFF, 0x12345678);
    }
    #endif
    
    /* 8. More complex vector operations */
    {
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
        
        /* Trigonometric approximation - may trigger helper generation */
        v4sf result;
        for (int i = 0; i < 4; i++) {
            /* Crude sine approximation using Bhaskara I's formula */
            float x = vec1[i];
            float pi = 3.14159265f;
            float x_scaled = x * (180.0f / pi);
            result[i] = (4 * x_scaled * (180 - x_scaled)) / 
                       (40500 - x_scaled * (180 - x_scaled));
        }
        
        volatile v4sf dummy = result;
        (void)dummy;
    }
    
    printf("Program completed successfully\n");
    return 0;
}
