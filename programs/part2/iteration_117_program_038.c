/* file1.c - Main file with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* For Transactional Memory */
volatile int global_counter = 0;

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Weak function declaration from file2 */
extern void weak_constructor_func(void) __attribute__((weak));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Mix of operations - some might need runtime helpers */
    v4si result = a * b + (a >> 1) - (b << 1);
    
    /* Use result to prevent optimization */
    volatile v4si volatile_result = result;
    (void)volatile_result;
}

/* Function using CPU feature detection */
void cpu_feature_dependent_code(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check multiple features to increase chance of helper generation */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    if (has_avx2) {
        avx2_vector_operations();
    }
    
    /* Complex conditional with builtins */
    volatile int features = has_avx512 + has_avx2 * 2 + has_sse4 * 4;
    (void)features;
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 100;
    int data[n];
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    /* Try to offload - may generate fallback helpers */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        for (int i = 0; i < n; i++) {
            data[i] += i;
        }
    }
    
    /* Use data to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* Transactional memory function */
void transactional_operation(void) {
    /* Transactional memory block */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        for (int i = 0; i < 10; i++) {
            global_counter += i;
        }
    }
    
    /* Another transaction with more complexity */
    __transaction_atomic {
        int local = global_counter;
        for (int i = 0; i < 5; i++) {
            local *= 2;
        }
        global_counter = local;
    }
}

/* Function with large stack usage for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char buffer[1024 * 1024];  /* 1MB buffer */
    
    /* Fill and use buffer */
    memset(buffer, 0xAA, sizeof(buffer));
    
    /* Complex operations on buffer */
    for (size_t i = 0; i < sizeof(buffer); i += 256) {
        buffer[i] = (char)(i % 256);
    }
    
    /* Use buffer to prevent optimization */
    volatile char first = buffer[0];
    volatile char last = buffer[sizeof(buffer) - 1];
    (void)first;
    (void)last;
}

int main(void) {
    printf("Starting target hook trigger program...\n");
    
    /* Execute all techniques */
    cpu_feature_dependent_code();
    
    openmp_offload_attempt();
    
    transactional_operation();
    
    large_stack_function();
    
    /* Call weak constructor function if available */
    if (&weak_constructor_func) {
        weak_constructor_func();
    }
    
    printf("Global counter: %d\n", global_counter);
    printf("Program completed.\n");
    
    return 0;
}
