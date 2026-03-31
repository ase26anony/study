/* file1.c - Main program with multiple target-specific patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
volatile int global_counter = 0;

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Weak function declaration that will be defined in file2.c */
extern void __attribute__((weak)) weak_target_helper(void);

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
    volatile v4si volatile_result = result;
    (void)volatile_result;
}

/* Function using ARM-specific built-in (if compiled for ARM) */
#ifdef __arm__
void arm_specific_operations(void) {
    /* Use ARM system register access built-in */
    unsigned int val;
    __asm__ volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(val));
    
    /* Complex expression with the result */
    volatile unsigned int volatile_val = val * 2 + 1;
    (void)volatile_val;
}
#endif

/* Function using PowerPC specific built-in */
#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* Use PowerPC MTFSF built-in */
    double d = 3.14159;
    __builtin_ppc_mtfsf(0xFF, d);
    
    volatile double volatile_d = d * 2.0;
    (void)volatile_d;
}
#endif

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 100;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt offload - compiler may generate hidden helper */
    #pragma omp target map(tofrom: array[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] = array[i] * 2 + 1;
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    (void)sum;
    
    free(array);
}

/* Transactional memory section */
void transactional_memory_operation(void) {
    /* This requires -fgnu-tm flag */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation */
        int local = global_counter;
        for (int i = 0; i < 10; i++) {
            local += i * 2;
        }
        global_counter = local;
    }
}

/* Large stack array to trigger stack protection helpers */
void stack_protection_test(void) {
    /* Large array that might trigger stack protection */
    char large_buffer[4096];
    volatile char *volatile_ptr = large_buffer;
    
    /* Fill with pattern */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i & 0xFF;
    }
    
    /* Complex operation on buffer */
    for (int i = 0; i < sizeof(large_buffer) - 1; i++) {
        large_buffer[i] = (large_buffer[i] + large_buffer[i + 1]) / 2;
    }
    
    /* Use result */
    (void)volatile_ptr;
}

int main(void) {
    printf("Starting target hook test program...\n");
    
    /* 1. CPU feature detection and conditional execution */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported, running vector operations\n");
        avx2_vector_operations();
    } else if (__builtin_cpu_supports("avx")) {
        printf("AVX supported\n");
        /* Use different target attribute */
        #pragma GCC target("avx")
        {
            v4si a = {1, 2, 3, 4};
            v4si b = {5, 6, 7, 8};
            volatile v4si result = a * b;
            (void)result;
        }
    }
    
    /* 2. Architecture-specific built-ins */
    #ifdef __arm__
    printf("ARM architecture detected\n");
    arm_specific_operations();
    #elif defined(__powerpc__)
    printf("PowerPC architecture detected\n");
    powerpc_specific_operations();
    #elif defined(__x86_64__)
    printf("x86_64 architecture detected\n");
    /* Additional x86 specific built-in */
    unsigned long long tsc = __builtin_ia32_rdtsc();
    volatile unsigned long long volatile_tsc = tsc;
    (void)volatile_tsc;
    #endif
    
    /* 3. OpenMP offload attempt */
    printf("Attempting OpenMP offload\n");
    openmp_offload_attempt();
    
    /* 4. Transactional memory operation */
    printf("Performing transactional memory operation\n");
    transactional_memory_operation();
    
    /* 5. Stack protection test */
    printf("Testing stack protection\n");
    stack_protection_test();
    
    /* 6. Call weak function from other compilation unit */
    printf("Calling weak function\n");
    if (&weak_target_helper) {
        weak_target_helper();
    }
    
    /* 7. Complex expression with multiple target built-ins */
    {
        /* Mix of operations that might need runtime helpers */
        v4si v1 = {1, 2, 3, 4};
        v4si v2 = {5, 6, 7, 8};
        
        /* Complex operation that might need helper function */
        v4si complex_result = (v1 * v2) + (v1 / (v2 + 1)) - (v1 % (v2 - 1));
        
        /* Use GCC vector built-in */
        #ifdef __x86_64__
        __builtin_ia32_mfence();
        #endif
        
        volatile v4si volatile_complex = complex_result;
        (void)volatile_complex;
    }
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
