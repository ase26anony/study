/* Primary file with main() and complex patterns */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* For Transactional Memory */
volatile int global_counter = 0;

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Weak function declaration that will be defined in file2.c */
extern void __attribute__((weak)) weak_helper_function(void);

/* Function using target-specific attributes and built-ins */
int __attribute__((target("avx2"))) avx2_vector_operation(void) {
    /* Use CPU feature detection built-ins */
    __builtin_cpu_init();
    
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {0};
    
    /* Vector division - often requires helper functions */
    for (int i = 0; i < 4; i++) {
        c[i] = a[i] / (b[i] + 1);  /* Avoid division by zero */
    }
    
    /* Use AVX intrinsics if supported */
    if (__builtin_cpu_supports("avx2")) {
        /* Complex expression with multiple operations */
        v4si result = a * b + c;
        return result[0] + result[1] + result[2] + result[3];
    }
    
    return c[0] + c[1] + c[2] + c[3];
}

/* Function with floating-point vector operations */
float __attribute__((target("default"))) complex_float_operations(void) {
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    
    /* Trigonometric approximation - may need helper functions */
    v4sf result = f1 * f2;
    
    /* Complex operation that might trigger helper generation */
    for (int i = 0; i < 4; i++) {
        /* Approximate sin using Taylor series (truncated) */
        float x = result[i];
        float sin_approx = x - (x*x*x)/6.0f + (x*x*x*x*x)/120.0f;
        result[i] = sin_approx;
    }
    
    return result[0] + result[1] + result[2] + result[3];
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 100;
    int data[n];
    
    #pragma omp target map(tofrom: data[0:n]) device(0)
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        data[i] = i * i;
    }
    
    /* Use volatile to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
}

/* Transactional memory block */
void transactional_operation(void) {
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local_array[256];
        for (int i = 0; i < 256; i++) {
            local_array[i] = i * global_counter;
        }
        
        /* Force stack usage */
        volatile int temp = 0;
        for (int i = 0; i < 256; i++) {
            temp += local_array[i];
        }
    }
}

int main(void) {
    int result = 0;
    
    /* Path 1: AVX2 vector operations */
    result += avx2_vector_operation();
    
    /* Path 2: Complex float operations */
    result += (int)complex_float_operations();
    
    /* Path 3: OpenMP offload attempt */
    openmp_offload_attempt();
    
    /* Path 4: Transactional memory */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    
    /* Path 5: Weak function call */
    if (weak_helper_function) {
        weak_helper_function();
    }
    
    /* Use ARM-specific built-in if compiling for ARM */
    #ifdef __arm__
    {
        unsigned int fpcr;
        fpcr = __builtin_arm_mrc(15, 7, 0, 0, 0);
        result += (int)fpcr;
    }
    #elif __aarch64__
    {
        unsigned long fpcr = __builtin_aarch64_get_fpcr();
        result += (int)fpcr;
    }
    #elif __powerpc__
    {
        /* PowerPC built-in */
        __builtin_ppc_mtfsf(0xFF, 0);
    }
    #endif
    
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    /* Force stack protector usage with large array */
    char large_buffer[1024 * 1024];
    for (size_t i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = (char)(result + i);
    }
    
    volatile char checksum = 0;
    for (size_t i = 0; i < sizeof(large_buffer); i += 4096) {
        checksum += large_buffer[i];
    }
    
    return checksum;
}
