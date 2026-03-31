/* file1.c - Main program with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
volatile int global_counter = 0;

/* Weak function declaration that will be defined in file2.c */
extern void __attribute__((weak)) weak_constructor_func(void);

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Division on integer vectors often requires helper functions */
    v4si result;
    for (int i = 0; i < 4; i++) {
        result[i] = a[i] / (b[i] + 1); /* Complex enough to avoid optimization */
    }
    
    /* Use result to prevent dead code elimination */
    volatile v4si volatile_result = result;
    (void)volatile_result;
}

/* Function with AVX512 target attribute */
__attribute__((target("avx512f")))
void avx512_check_and_compute(void) {
    /* Check CPU features at runtime */
    if (__builtin_cpu_supports("avx512f")) {
        __builtin_cpu_init();
        
        /* More complex vector operations */
        typedef float v16sf __attribute__((vector_size(64)));
        v16sf v1 = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f,
                    0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f};
        v16sf v2 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                    9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
        
        /* Complex operation that might need helper */
        v16sf v3 = v1 / (v2 + 1.0f);
        
        volatile v16sf volatile_v3 = v3;
        (void)volatile_v3;
    }
}

/* Function with ARM-specific builtins (will be compiled on ARM only) */
#ifdef __arm__
void arm_specific_operations(void) {
    /* Use ARM-specific builtins */
    unsigned int fpscr = __builtin_arm_mrc(15, 0, 1, 0, 0);
    volatile unsigned int volatile_fpscr = fpscr;
    (void)volatile_fpscr;
}
#endif

#ifdef __aarch64__
void aarch64_specific_operations(void) {
    /* Use AArch64-specific builtins */
    unsigned long fpcr = __builtin_aarch64_get_fpcr();
    volatile unsigned long volatile_fpcr = fpcr;
    (void)volatile_fpcr;
}
#endif

#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* Use PowerPC-specific builtins */
    double d = 3.14;
    __builtin_ppc_mtfsf(0xFF, d);
}
#endif

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 100;
    int a[n], b[n], c[n];
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Attempt to offload - will likely generate fallback helpers */
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) device(0)
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Use result to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += c[i];
    }
    (void)sum;
}

/* Transactional Memory function */
void transactional_operation(void) {
    /* Transactional memory block */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local_array[100];
        for (int i = 0; i < 100; i++) {
            local_array[i] = i * global_counter;
        }
        
        /* Use array to prevent optimization */
        volatile int volatile_sum = 0;
        for (int i = 0; i < 100; i++) {
            volatile_sum += local_array[i];
        }
        (void)volatile_sum;
    }
}

/* Function with large stack frame to trigger stack protection */
void large_stack_frame_function(void) {
    /* Large array to trigger stack protector */
    char large_buffer[1024 * 10]; /* 10KB buffer */
    
    /* Fill with pattern */
    for (size_t i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = (char)(i % 256);
    }
    
    /* Complex operation on buffer */
    volatile size_t checksum = 0;
    for (size_t i = 0; i < sizeof(large_buffer); i++) {
        checksum += large_buffer[i];
    }
    (void)checksum;
    
    /* Call target-specific functions */
    #pragma GCC push_options
    #pragma GCC target("avx2")
    avx2_vector_operations();
    #pragma GCC pop_options
}

int main(void) {
    printf("Starting target hook triggering program...\n");
    
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Execute different code paths based on CPU features */
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported, running vector operations...\n");
        avx2_vector_operations();
    }
    
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported, running AVX512 operations...\n");
        avx512_check_and_compute();
    }
    
    /* Platform-specific operations */
    #ifdef __arm__
    printf("ARM architecture, running ARM-specific operations...\n");
    arm_specific_operations();
    #endif
    
    #ifdef __aarch64__
    printf("AArch64 architecture, running AArch64-specific operations...\n");
    aarch64_specific_operations();
    #endif
    
    #ifdef __powerpc__
    printf("PowerPC architecture, running PowerPC-specific operations...\n");
    powerpc_specific_operations();
    #endif
    
    /* Attempt OpenMP offload */
    printf("Attempting OpenMP offload...\n");
    openmp_offload_attempt();
    
    /* Transactional memory operation */
    printf("Running transactional memory operation...\n");
    transactional_operation();
    
    /* Function with large stack frame */
    printf("Running function with large stack frame...\n");
    large_stack_frame_function();
    
    /* Call weak constructor function from other compilation unit */
    printf("Calling weak constructor function...\n");
    if (&weak_constructor_func) {
        weak_constructor_func();
    }
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
