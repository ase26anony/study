/* file1.c - Main program with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For transactional memory */
int global_counter = 0;

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Declare function from file2.c */
extern void weak_constructor_function(void);

/* Function using AVX2 target pragma */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector division - often requires helper functions */
    v4si c;
    for (int i = 0; i < 4; i++) {
        c[i] = a[i] / (b[i] + 1); /* Avoid division by zero */
    }
    
    /* Use result to prevent optimization */
    volatile v4si result = c;
    (void)result;
}

/* Function using CPU feature detection */
void cpu_feature_detection(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check multiple features to increase chance of helper generation */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported\n");
        
        /* Force potential helper generation with complex condition */
        volatile int use_avx512 = __builtin_cpu_supports("avx512f") && 
                                  __builtin_cpu_supports("avx512cd");
        (void)use_avx512;
    }
    
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
        avx2_vector_operations();
    }
}

/* Function with OpenMP offloading attempt */
void openmp_offloading(void) {
    int n = 100;
    int data[n];
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offloading - compiler may generate fallback helpers */
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
    (void)sum;
}

/* Function using transactional memory */
void transactional_memory_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        if (global_counter % 2 == 0) {
            global_counter *= 2;
        }
    }
    
    printf("Global counter: %d\n", global_counter);
}

/* Function with large stack usage for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char buffer[1024 * 16]; /* 16KB buffer */
    
    /* Initialize and use buffer */
    memset(buffer, 'A', sizeof(buffer));
    
    /* Complex operations on buffer */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (buffer[i] + i) % 256;
    }
    
    /* Use result */
    volatile char check = buffer[sizeof(buffer) - 1];
    (void)check;
}

/* Main function orchestrating all techniques */
int main(void) {
    printf("Starting target hook triggering program\n");
    
    /* 1. CPU feature detection with builtins */
    cpu_feature_detection();
    
    /* 2. OpenMP offloading attempt */
    openmp_offloading();
    
    /* 3. Transactional memory operations */
    for (int i = 0; i < 5; i++) {
        transactional_memory_operation();
    }
    
    /* 4. Large stack usage for stack protector */
    large_stack_function();
    
    /* 5. Call weak constructor function from other file */
    weak_constructor_function();
    
    /* 6. Additional ARM/PowerPC builtins if compiled for those arches */
    #if defined(__arm__) || defined(__aarch64__)
    /* ARM specific builtin */
    unsigned int fpcr = 0;
    #ifdef __aarch64__
    fpcr = __builtin_aarch64_get_fpcr();
    #else
    fpcr = __builtin_arm_mrc(15, 7, 0, 0, 0);
    #endif
    volatile unsigned int arm_result = fpcr;
    (void)arm_result;
    #elif defined(__powerpc__) || defined(__ppc__)
    /* PowerPC specific builtin */
    __builtin_ppc_mtfsf(0xFF, 0);
    #endif
    
    /* 7. More vector operations with different sizes */
    {
        typedef float v8sf __attribute__((vector_size(32)));
        v8sf v1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        v8sf v2 = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
        
        /* Complex operation that might need helper */
        v8sf v3 = v1 / (v2 + 0.1);
        volatile v8sf vec_result = v3;
        (void)vec_result;
    }
    
    printf("Program completed\n");
    return 0;
}
