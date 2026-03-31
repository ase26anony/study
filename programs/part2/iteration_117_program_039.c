/* file1.c - Main file with multiple target-specific patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables for transactional memory */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Weak function that will be aliased in another file */
void __attribute__((weak, constructor)) weak_constructor_func();

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations() {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression with multiple operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si* volatile_ptr = &result;
    (void)volatile_ptr;
    
    /* Check CPU features at runtime */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F is supported\n");
    }
}

/* Function using ARM-style builtins (will be compiled for appropriate arch) */
void check_system_registers() {
    /* These builtins may require runtime helpers */
    unsigned int fpsr;
    
    /* Use architecture-specific builtins */
    #ifdef __arm__
    fpsr = __builtin_arm_get_fpscr();
    #elif __aarch64__
    fpsr = __builtin_aarch64_get_fpcr();
    #elif __powerpc__
    __builtin_ppc_mtfsf(0xFF, 0);
    #endif
    
    volatile unsigned int* vol_fpsr = &fpsr;
    (void)vol_fpsr;
}

/* Transactional memory function */
void transactional_operation() {
    /* Transactional memory block - may generate TM runtime helpers */
    __transaction_atomic {
        global_counter++;
        global_result = global_counter * 2;
    }
}

/* OpenMP target region */
void openmp_offload_attempt() {
    int n = 100;
    int* data = (int*)malloc(n * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - may generate fallback helpers */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2 + 1;
        }
    }
    
    /* Use result */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    
    free(data);
}

/* Function with large stack usage for stack protector */
void large_stack_function() {
    /* Large array to trigger stack protection */
    char large_buffer[4096];
    int another_buffer[512];
    
    /* Use buffers to prevent optimization */
    memset(large_buffer, 0, sizeof(large_buffer));
    for (int i = 0; i < 512; i++) {
        another_buffer[i] = i * 2;
    }
    
    /* Complex operations on buffers */
    volatile int check = 0;
    for (int i = 0; i < sizeof(large_buffer); i++) {
        check += large_buffer[i];
    }
    
    /* Call target-specific function */
    avx2_vector_operations();
}

/* Main function orchestrating all patterns */
int main() {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    printf("Starting target hook coverage test...\n");
    
    /* 1. Use target-specific builtins and vector extensions */
    avx2_vector_operations();
    check_system_registers();
    
    /* 2. Complex vector operations */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Trigonometric approximation - may need helper */
    v4sf vec_result = vec1 * vec2 + vec1 / (vec2 + 1.0f);
    volatile v4sf* vol_vec = &vec_result;
    (void)vol_vec;
    
    /* 3. OpenMP offloading */
    openmp_offload_attempt();
    
    /* 4. Transactional memory */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    
    printf("Transactional counter: %d, result: %d\n", global_counter, global_result);
    
    /* 5. Large stack usage with stack protection */
    large_stack_function();
    
    /* 6. Call weak constructor function (defined in another file) */
    weak_constructor_func();
    
    printf("Test completed.\n");
    
    return 0;
}
