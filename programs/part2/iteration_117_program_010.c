/* file1.c - Main file with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
int global_counter = 0;

/* Weak function declaration that will be defined in file2.c */
extern void weak_constructor_func(void) __attribute__((weak));

/* Vector extensions with complex operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Target-specific function using AVX2 */
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

/* Function using CPU feature detection */
void cpu_feature_detection(void) {
    /* Force __builtin_cpu_init which may need helpers */
    __builtin_cpu_init();
    
    /* Check multiple CPU features */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Use results to prevent dead code elimination */
    volatile int volatile_features = has_avx512 + has_avx2 + has_sse4;
    (void)volatile_features;
    
    if (has_avx2) {
        avx2_vector_operations();
    }
}

/* Transactional Memory function */
void transactional_operations(void) {
    /* Use GCC Transactional Memory */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operations inside transaction */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {0.5f, 1.5f, 2.5f, 3.5f};
        v4sf result = vec1 / vec2;
        
        volatile v4sf volatile_vec = result;
        (void)volatile_vec;
    }
}

/* OpenMP target region */
void openmp_offloading(void) {
    int n = 100;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt offloading - may generate fallback helpers */
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

/* Function with large stack for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char large_buffer[4096];
    int another_buffer[512];
    
    /* Use buffers to prevent optimization */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i % 256;
    }
    
    /* Complex operations on buffers */
    for (int i = 0; i < 512; i++) {
        another_buffer[i] = large_buffer[i * 2] * 3;
    }
    
    /* Use result */
    volatile int check = another_buffer[100];
    (void)check;
}

int main(void) {
    printf("Starting target hook triggering program...\n");
    
    /* 1. CPU feature detection and vector operations */
    cpu_feature_detection();
    
    /* 2. Transactional Memory operations */
    for (int i = 0; i < 5; i++) {
        transactional_operations();
    }
    
    /* 3. OpenMP offloading attempt */
    openmp_offloading();
    
    /* 4. Large stack function for stack protector */
    large_stack_function();
    
    /* 5. Call weak constructor function from other file */
    if (&weak_constructor_func) {
        weak_constructor_func();
    }
    
    /* 6. Additional ARM/PowerPC built-in simulation */
    #if defined(__arm__) || defined(__aarch64__)
    /* ARM specific built-ins */
    unsigned int fpsr;
    fpsr = __builtin_arm_get_fpscr();
    volatile unsigned int volatile_fpsr = fpsr;
    (void)volatile_fpsr;
    #elif defined(__powerpc__) || defined(__ppc__)
    /* PowerPC specific built-ins */
    double d = 3.14;
    __builtin_ppc_mtfsf(0xFF, d);
    #endif
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
