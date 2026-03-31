/* file1.c - Main program with multiple patterns to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
int global_counter = 0;

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Declare external weak function */
extern void weak_constructor_func(void) __attribute__((weak));

/* Function using AVX2 intrinsics through target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Division on integer vectors often requires helper functions */
    v4si c;
    for (int i = 0; i < 4; i++) {
        c[i] = a[i] / (b[i] + 1); /* Non-uniform division */
    }
    
    /* Mix with floating point */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 0.25f, 0.125f, 0.0625f};
    v4sf fc = fa / fb; /* Division might need runtime support */
    
    volatile v4si dummy = c; /* Prevent optimization */
    (void)dummy;
}

/* Function with OpenMP offloading */
void openmp_offload_test(void) {
    int n = 100;
    int arr[n];
    
    #pragma omp target map(tofrom: arr[0:n]) device(0)
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        arr[i] = i * i;
    }
    
    /* Use result to prevent elimination */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
}

/* Function using transactional memory */
void transactional_memory_test(void) {
    __transaction_atomic {
        global_counter++;
        /* Complex operation inside transaction */
        int x = global_counter * 2;
        global_counter = x % 100;
    }
}

/* Function using CPU feature detection */
void cpu_feature_test(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check multiple features - each might need helpers */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Force generation of helper functions by using in complex expression */
    volatile int features = (has_avx512 << 2) | (has_avx2 << 1) | has_sse4;
    
    if (has_avx2) {
        avx2_vector_operations();
    }
}

/* Large stack usage to trigger stack protection helpers */
void stack_protection_test(void) {
    char large_buffer[4096]; /* Large stack allocation */
    int another_buffer[512];
    
    /* Fill buffers to ensure they're used */
    for (int i = 0; i < 4096; i++) {
        large_buffer[i] = (char)(i % 256);
    }
    
    for (int i = 0; i < 512; i++) {
        another_buffer[i] = i * 3;
    }
    
    /* Complex operation mixing buffers */
    volatile int checksum = 0;
    for (int i = 0; i < 512; i++) {
        checksum += another_buffer[i] + large_buffer[i * 2];
    }
}

/* ARM-specific built-ins (if compiled for ARM) */
#ifdef __arm__
void arm_builtin_test(void) {
    /* Use ARM system register access */
    unsigned int val;
    __asm__ volatile ("mrc p15, 0, %0, c0, c0, 0" : "=r"(val));
    volatile unsigned int dummy = val;
}
#endif

/* PowerPC-specific built-ins (if compiled for PowerPC) */
#ifdef __powerpc__
void powerpc_builtin_test(void) {
    /* Use PowerPC special register access */
    double d = 1.0;
    __builtin_ppc_mtfsf(0xFF, d);
}
#endif

int main(void) {
    printf("Starting target hook trigger program...\n");
    
    /* Execute all test patterns */
    
    /* 1. CPU feature detection and vector operations */
    cpu_feature_test();
    
    /* 2. OpenMP offloading (will likely use fallback helpers) */
    openmp_offload_test();
    
    /* 3. Transactional memory */
    for (int i = 0; i < 10; i++) {
        transactional_memory_test();
    }
    
    /* 4. Stack protection trigger */
    stack_protection_test();
    
    /* 5. Call weak constructor function if available */
    if (weak_constructor_func) {
        weak_constructor_func();
    }
    
    /* 6. Architecture-specific tests */
    #ifdef __arm__
    arm_builtin_test();
    #endif
    
    #ifdef __powerpc__
    powerpc_builtin_test();
    #endif
    
    /* Use global to prevent optimization */
    volatile int result = global_counter;
    printf("Final counter: %d\n", result);
    
    return 0;
}
