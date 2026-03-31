/* main.c - Primary file with complex operations to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) weak_helper_function(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Simulate complex computation that might need helper */
    v4si result = (a * b) + (c / (b + 1));
    
    /* Use result to prevent optimization */
    volatile v4si dummy = result;
    (void)dummy;
}

/* Function with AVX512 check and operations */
void check_and_use_avx512(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for AVX512 - this may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F is supported\n");
        
        #pragma GCC target("avx512f")
        {
            /* Vector operations with AVX512 */
            typedef float v16sf __attribute__((vector_size(64)));
            v16sf v1 = {0}, v2 = {0};
            v16sf v3 = v1 + v2;
            volatile v16sf v_dummy = v3;
            (void)v_dummy;
        }
    } else {
        printf("AVX512F not supported\n");
    }
}

/* Function with ARM-style built-in (will be compiled on x86 too) */
void simulate_arm_builtins(void) {
    /* These may generate stub helpers on non-ARM targets */
    volatile unsigned long pfr;
    
    /* Attempt to use ARM-specific builtins */
    #ifdef __arm__
    pfr = __builtin_arm_mrc(15, 0, 0, 0, 0);
    #elif __aarch64__
    pfr = __builtin_aarch64_get_fpcr();
    #endif
    
    (void)pfr;
}

/* Function with PowerPC-style built-in */
void simulate_ppc_builtins(void) {
    volatile double d = 1.0;
    
    /* Attempt to use PPC-specific builtin */
    #ifdef __powerpc__
    __builtin_ppc_mtfsf(0xFF, d);
    #endif
    
    (void)d;
}

/* Function using transactional memory */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local_array[256];
        for (int i = 0; i < 256; i++) {
            local_array[i] = i * global_counter;
        }
        
        /* Use array to prevent optimization */
        volatile int sum = 0;
        for (int i = 0; i < 256; i++) {
            sum += local_array[i];
        }
        (void)sum;
    }
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 1000;
    int *data = (int*)malloc(n * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - may generate fallback helpers */
    #pragma omp target device(0) map(tofrom: data[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] *= 2;
        }
    }
    
    /* Verify and use results */
    volatile int check = 0;
    for (int i = 0; i < n; i++) {
        check += data[i];
    }
    (void)check;
    
    free(data);
}

int main(void) {
    printf("Starting target hook trigger program\n");
    
    /* 1. CPU feature detection and vector operations */
    check_and_use_avx512();
    avx2_vector_operations();
    
    /* 2. Architecture-specific builtins */
    simulate_arm_builtins();
    simulate_ppc_builtins();
    
    /* 3. Transactional memory operations */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    printf("Global counter: %d\n", global_counter);
    
    /* 4. OpenMP offload attempt */
    openmp_offload_attempt();
    
    /* 5. Call weak function from helper.c */
    if (&weak_helper_function) {
        weak_helper_function();
    }
    
    /* Complex expression with stack protection trigger */
    {
        char large_buffer[1024];
        volatile int secret = 42;
        
        /* Fill buffer in a way that might trigger stack protection */
        for (size_t i = 0; i < sizeof(large_buffer); i++) {
            large_buffer[i] = (char)(i ^ secret);
        }
        
        /* Use buffer to prevent optimization */
        volatile char checksum = 0;
        for (size_t i = 0; i < sizeof(large_buffer); i++) {
            checksum ^= large_buffer[i];
        }
        (void)checksum;
    }
    
    /* Additional vector math that might need helpers */
    {
        v4sf angles = {0.0f, 1.57f, 3.14f, 4.71f};
        v4sf results = {0};
        
        /* Approximate sin using Taylor series - may trigger math helpers */
        for (int i = 0; i < 4; i++) {
            float x = angles[i];
            float x2 = x * x;
            float x3 = x2 * x;
            float x5 = x3 * x2;
            results[i] = x - x3/6.0f + x5/120.0f;
        }
        
        volatile v4sf dummy = results;
        (void)dummy;
    }
    
    printf("Program completed\n");
    return 0;
}
