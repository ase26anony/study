/* main.c - Primary file with complex operations to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_init(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression with multiple operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si* volatile_ptr = &result;
    (void)volatile_ptr;
    
    /* Trigonometric approximation on integer vectors (may need helper) */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 0.25f, 0.125f, 0.0625f};
    
    /* Approximation: sin(x) ≈ x - x³/6 + x⁵/120 */
    v4sf x = fa * fb;
    v4sf x3 = x * x * x;
    v4sf x5 = x3 * x * x;
    v4sf sin_approx = x - x3 / 6.0f + x5 / 120.0f;
    
    volatile v4sf* volatile_ptr2 = &sin_approx;
    (void)volatile_ptr2;
}

/* Function with AVX512 target attribute */
__attribute__((target("avx512f")))
void avx512_check_and_compute(void) {
    /* Check CPU features - may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        __builtin_cpu_init();
        
        /* Large array to trigger stack protection */
        char buffer[1024];
        for (int i = 0; i < sizeof(buffer); i++) {
            buffer[i] = i % 256;
        }
        
        /* Use buffer to prevent optimization */
        volatile char* volatile_buf = buffer;
        (void)volatile_buf;
    }
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 100;
    int a[n], b[n], c[n];
    
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Attempt offload - may generate fallback helper functions */
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) device(0)
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Use result */
    volatile int* volatile_c = c;
    (void)volatile_c;
}

/* Transactional memory function */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation */
        v4si vec1 = {global_counter, 2, 3, 4};
        v4si vec2 = {5, 6, 7, 8};
        v4si vec_result = vec1 * vec2 + vec1 / (vec2 + 1);
        
        volatile v4si* volatile_vec = &vec_result;
        (void)volatile_vec;
    }
}

/* ARM-specific built-ins (if compiled for ARM) */
#ifdef __arm__
void arm_specific_operations(void) {
    /* ARM system register access - may need helper */
    unsigned int val;
    __asm__ volatile ("mrc p15, 0, %0, c0, c0, 0" : "=r"(val));
    
    /* Use the value */
    volatile unsigned int* volatile_val = &val;
    (void)volatile_val;
}
#endif

/* PowerPC-specific built-ins (if compiled for PowerPC) */
#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* PowerPC special register access */
    double d = 3.14159;
    long long ll = __builtin_ppc_mftb();
    
    volatile long long* volatile_ll = &ll;
    (void)volatile_ll;
    
    /* Use floating point control */
    __builtin_ppc_mtfsf(0xFF, d);
}
#endif

int main(void) {
    printf("Starting target hook triggering program...\n");
    
    /* 1. Call weak function from helper.c */
    if (target_helper_init) {
        target_helper_init();
    }
    
    /* 2. Execute AVX2 operations */
    avx2_vector_operations();
    
    /* 3. Check and use AVX512 if available */
    avx512_check_and_compute();
    
    /* 4. Attempt OpenMP offload */
    openmp_offload_attempt();
    
    /* 5. Perform transactional memory operations */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    
    /* 6. Architecture-specific operations */
    #ifdef __arm__
    arm_specific_operations();
    #endif
    
    #ifdef __powerpc__
    powerpc_specific_operations();
    #endif
    
    /* 7. More CPU feature checks */
    const char* features[] = {"avx", "avx2", "sse4.2", "fma", NULL};
    for (int i = 0; features[i] != NULL; i++) {
        if (__builtin_cpu_supports(features[i])) {
            printf("CPU supports %s\n", features[i]);
        }
    }
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
