/* file1.c - Main program with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
volatile int global_counter = 0;

/* Weak function that will be aliased in another file */
void __attribute__((weak, constructor)) weak_constructor_func();

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations() {
    /* Complex vector operations that might need helper functions */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector division - often requires helper functions */
    v4si result;
    for (int i = 0; i < 4; i++) {
        result[i] = a[i] / (b[i] + 1); /* Avoid division by zero */
    }
    
    /* Use result to prevent optimization */
    volatile v4si dummy = result;
    (void)dummy;
}

/* Function using CPU feature detection */
void cpu_feature_detection() {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for various features - each might generate helpers */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Use the results */
    volatile int features = has_avx512 + has_avx2 + has_sse4;
    (void)features;
    
    if (has_avx2) {
        avx2_vector_operations();
    }
}

/* Transactional memory function */
void transactional_operation() {
    /* This requires libitm runtime helpers */
    __transaction_atomic {
        global_counter++;
        
        /* Nested memory access to increase complexity */
        int temp = global_counter;
        global_counter = temp * 2;
    }
}

/* OpenMP target region */
void openmp_offload_attempt() {
    int n = 100;
    int data[n];
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - will likely generate fallback helpers */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2 + 1;
        }
    }
    
    /* Use results */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* Function with large stack for stack protector */
void large_stack_function() {
    /* Large array to trigger stack protection */
    char buffer[1024 * 1024]; /* 1MB buffer */
    
    /* Use buffer to prevent optimization */
    memset(buffer, 0xAA, sizeof(buffer));
    
    volatile char check = buffer[sizeof(buffer) - 1];
    (void)check;
}

/* ARM/PowerPC specific built-ins (if compiled for those archs) */
void arch_specific_builtins() {
    /* These will be compiled conditionally based on architecture */
    #if defined(__arm__) || defined(__aarch64__)
    /* ARM specific */
    unsigned int fpscr = __builtin_arm_get_fpscr();
    volatile unsigned int dummy_arm = fpscr;
    (void)dummy_arm;
    #elif defined(__powerpc__) || defined(__ppc__)
    /* PowerPC specific */
    __builtin_ppc_mtfsf(0xFF, 0);
    #endif
}

int main() {
    printf("Starting program to trigger target hooks...\n");
    
    /* 1. CPU feature detection and vector operations */
    cpu_feature_detection();
    
    /* 2. Transactional memory operation */
    transactional_operation();
    
    /* 3. OpenMP offload attempt */
    openmp_offload_attempt();
    
    /* 4. Large stack function for stack protector */
    large_stack_function();
    
    /* 5. Architecture-specific built-ins */
    arch_specific_builtins();
    
    /* 6. Call weak constructor function */
    if (&weak_constructor_func) {
        weak_constructor_func();
    }
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
