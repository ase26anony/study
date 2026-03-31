/* main.c - Primary file with complex operations to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) weak_helper_function(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Mix of operations including division which often needs helpers */
    v4si c = a + b;
    v4si d = a * b;
    
    /* Simulate complex operation using conditional select pattern */
    v4si mask = a > b;
    v4si result = mask ? c : d;
    
    /* Use result to prevent optimization */
    volatile v4si* volatile_ptr = &result;
    (void)volatile_ptr;
    
    printf("AVX2 vector operation completed\n");
}

/* Function checking CPU features */
void check_and_use_cpu_features(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for various features - each might trigger helper generation */
    int has_avx = __builtin_cpu_supports("avx");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_avx512f = __builtin_cpu_supports("avx512f");
    
    printf("CPU features: AVX=%d, AVX2=%d, AVX512F=%d\n", 
           has_avx, has_avx2, has_avx512f);
    
    if (has_avx2) {
        avx2_vector_operations();
    }
    
    /* Force generation of helpers for unsupported features too */
    if (has_avx512f) {
        /* This might trigger AVX512 helper generation even if not executed */
        printf("AVX512F supported\n");
    }
}

/* OpenMP target region - will generate fallback helpers */
void openmp_offload_attempt(void) {
    int data[100];
    
    #pragma omp parallel for
    for (int i = 0; i < 100; i++) {
        data[i] = i * 2;
    }
    
    /* Attempt offloading to potentially unsupported device */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:100]) device(0)
    for (int i = 0; i < 100; i++) {
        data[i] += i;
    }
    
    /* Use data to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    printf("OpenMP offload result (partial): %d\n", data[50]);
}

/* Transactional memory section */
void transactional_operation(void) {
    /* Transactional memory block */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local_array[256];
        for (int i = 0; i < 256; i++) {
            local_array[i] = i * global_counter;
        }
        
        /* Force stack usage with large array */
        volatile int large_array[1024];
        for (int i = 0; i < 1024; i++) {
            large_array[i] = i;
        }
    }
    
    printf("Transactional operation completed, counter=%d\n", global_counter);
}

/* ARM-specific builtins (if compiled for ARM) */
#ifdef __arm__
void arm_specific_operations(void) {
    /* Use ARM-specific builtins */
    unsigned int result;
    __asm__ volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(result));
    printf("ARM CPU ID: 0x%08x\n", result);
}
#endif

/* PowerPC-specific builtins (if compiled for PowerPC) */
#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* Use PowerPC-specific builtins */
    double d = 3.14159;
    __builtin_ppc_mtfsf(0xFF, d);
    printf("PowerPC MTFSF operation completed\n");
}
#endif

/* Main function with conditional execution paths */
int main(int argc, char** argv) {
    printf("Starting target hook trigger program\n");
    
    /* Path 1: CPU feature detection and vector operations */
    check_and_use_cpu_features();
    
    /* Path 2: OpenMP offloading attempt */
    openmp_offload_attempt();
    
    /* Path 3: Transactional memory operations */
    for (int i = 0; i < 3; i++) {
        transactional_operation();
    }
    
    /* Path 4: Call weak external function that uses target builtins */
    if (weak_helper_function) {
        weak_helper_function();
    } else {
        printf("Weak helper function not available\n");
    }
    
    /* Path 5: Architecture-specific operations */
    #ifdef __arm__
    arm_specific_operations();
    #endif
    
    #ifdef __powerpc__
    powerpc_specific_operations();
    #endif
    
    /* Complex expression with multiple builtins to force helper generation */
    volatile int use_builtins = 0;
    if (argc > 1) {
        __builtin_cpu_init();
        use_builtins = __builtin_cpu_supports("sse4.2") || 
                      __builtin_cpu_supports("avx") ||
                      __builtin_cpu_supports("fma");
    }
    
    /* Final print to ensure all code paths are considered */
    printf("Program completed successfully. Global counter: %d\n", global_counter);
    
    return 0;
}
