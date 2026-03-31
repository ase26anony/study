/* file1.c - Main program with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For transactional memory */
int global_counter = 0;

/* Weak function that will be defined in file2.c */
extern void weak_constructor_func(void) __attribute__((weak));

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Operations that may require helper functions */
    v4si result = a * b + a / (b + 1);
    
    /* Use result to prevent optimization */
    volatile v4si volatile_result = result;
    (void)volatile_result;
}

/* Function using CPU feature detection */
void cpu_feature_detection(void) {
    /* Force CPU initialization and feature checking */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported\n");
    } else if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
        avx2_vector_operations();
    }
    
    /* ARM-specific built-in (will be ignored on x86 but parsed) */
#ifdef __arm__
    unsigned int val = __builtin_arm_mrc(15, 0, 13, 0, 3);
    printf("ARM MRC: %u\n", val);
#endif
    
    /* PowerPC built-in */
#ifdef __powerpc__
    __builtin_ppc_mtfsf(0xFF, 1.0);
#endif
}

/* Transactional memory function */
void transactional_operation(void) {
    /* This requires libitm runtime helpers */
    __transaction_atomic {
        global_counter++;
        printf("Transactional counter: %d\n", global_counter);
    }
}

/* OpenMP offloading attempt */
void openmp_offload(void) {
    int data[100];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - may generate fallback helpers */
    #pragma omp target map(tofrom: data[0:100]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] *= 2;
        }
    }
    
    /* Use data to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    printf("OpenMP result sum: %d\n", sum);
}

/* Function with large stack to trigger stack protection */
void large_stack_function(void) {
    /* Large array to potentially trigger stack protector */
    char buffer[4096];
    int another_buffer[512];
    
    /* Use buffers to prevent optimization */
    memset(buffer, 0, sizeof(buffer));
    for (int i = 0; i < 512; i++) {
        another_buffer[i] = i;
    }
    
    volatile int check = buffer[100] + another_buffer[200];
    (void)check;
}

int main(void) {
    printf("Starting target hook trigger program\n");
    
    /* 1. CPU feature detection with built-ins */
    cpu_feature_detection();
    
    /* 2. Vector operations with target attributes */
    avx2_vector_operations();
    
    /* 3. OpenMP offloading attempt */
    openmp_offload();
    
    /* 4. Transactional memory operation */
    transactional_operation();
    
    /* 5. Large stack usage for stack protection */
    large_stack_function();
    
    /* 6. Call weak constructor function from other file */
    if (&weak_constructor_func) {
        weak_constructor_func();
    }
    
    /* Additional complex expression with built-ins */
    volatile int use_builtin = 0;
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse4.2")) {
        use_builtin = 1;
    }
    
    printf("Program completed. Counter: %d\n", global_counter);
    return use_builtin;
}
