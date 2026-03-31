/* file1.c - Main file with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
int global_counter = 0;

/* Weak function declaration that will be defined in file2.c */
extern void __attribute__((weak)) weak_target_helper(void);

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need helper functions */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* This division might require helper functions on some architectures */
    v4si result;
    for (int i = 0; i < 4; i++) {
        result[i] = a[i] / (b[i] + 1);  /* Avoid division by zero */
    }
    
    /* Use result to prevent optimization */
    volatile v4si v = result;
    (void)v;
}

/* Function using CPU feature detection */
void cpu_feature_detection(void) {
    /* Force CPU initialization and feature checking */
    __builtin_cpu_init();
    
    /* Check multiple features to increase chance of helper generation */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Use the results to prevent dead code elimination */
    volatile int features = has_avx512 + has_avx2 + has_sse4;
    (void)features;
    
    if (has_avx2) {
        avx2_vector_operations();
    }
}

/* Function with OpenMP target region */
void openmp_offloading(void) {
    int n = 100;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt offloading - will likely generate fallback helpers */
    #pragma omp target map(tofrom: array[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Use result to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    (void)sum;
    
    free(array);
}

/* Function using Transactional Memory */
void transactional_memory_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested operations to increase complexity */
        for (int i = 0; i < 10; i++) {
            global_counter += i % 3;
        }
    }
    
    /* Another transaction with conditional */
    __transaction_atomic {
        if (global_counter > 50) {
            global_counter = 0;
        }
    }
}

/* Function with large stack usage to trigger stack protection helpers */
void large_stack_function(void) {
    /* Large array to potentially trigger stack protection */
    char buffer[4096];
    int large_array[2048];
    
    /* Use the arrays to prevent optimization */
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i % 256;
    }
    
    for (int i = 0; i < 2048; i++) {
        large_array[i] = i * 2;
    }
    
    /* Complex operation mixing arrays */
    volatile int checksum = 0;
    for (int i = 0; i < 1000; i++) {
        checksum += buffer[i] + large_array[i % 2048];
    }
    (void)checksum;
}

/* Constructor that calls weak function */
__attribute__((constructor))
void init_function(void) {
    /* Call weak function - may trigger target hook during linking */
    if (weak_target_helper) {
        weak_target_helper();
    }
    
    /* Also do CPU initialization early */
    __builtin_cpu_init();
}

int main(void) {
    printf("Starting target hook triggering program...\n");
    
    /* Execute all techniques in sequence */
    
    /* 1. CPU feature detection and vector operations */
    printf("1. Running CPU feature detection...\n");
    cpu_feature_detection();
    
    /* 2. OpenMP offloading attempt */
    printf("2. Attempting OpenMP offloading...\n");
    openmp_offloading();
    
    /* 3. Transactional memory operations */
    printf("3. Executing transactional memory operations...\n");
    for (int i = 0; i < 5; i++) {
        transactional_memory_operation();
    }
    
    /* 4. Large stack usage */
    printf("4. Running large stack function...\n");
    large_stack_function();
    
    /* 5. Call weak function if available */
    printf("5. Calling weak function...\n");
    if (weak_target_helper) {
        weak_target_helper();
    }
    
    /* ARM-specific built-in if compiled for ARM */
    #ifdef __arm__
    printf("6. Using ARM-specific built-ins...\n");
    unsigned int fpcr;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 2" : "=r"(fpcr));
    volatile unsigned int arm_result = fpcr;
    (void)arm_result;
    #endif
    
    /* PowerPC-specific if compiled for PowerPC */
    #ifdef __powerpc__
    printf("6. Using PowerPC-specific built-ins...\n");
    double d = 3.14;
    __builtin_ppc_mtfsf(0xFF, d);
    #endif
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
