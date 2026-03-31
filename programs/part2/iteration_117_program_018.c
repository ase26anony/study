/* file1.c - Main program with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For transactional memory */
int global_counter = 0;

/* Weak function that will be aliased in another file */
void __attribute__((weak)) target_helper_function(void);

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector division - often requires helper functions */
    v4si result;
    for (int i = 0; i < 4; i++) {
        result[i] = a[i] / (b[i] + 1); /* Avoid division by zero */
    }
    
    /* Use result to prevent optimization */
    volatile v4si v = result;
    (void)v;
}

/* Function with AVX512 check */
void check_and_use_avx512(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for AVX512 - may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F is supported\n");
        
        /* Use pragma to switch target */
        #pragma GCC push_options
        #pragma GCC target("avx512f")
        {
            /* Complex vector operations with larger vectors */
            typedef float v16f __attribute__((vector_size(64)));
            v16f v1 = {0}, v2 = {0};
            
            /* Initialize vectors */
            for (int i = 0; i < 16; i++) {
                v1[i] = i * 1.5f;
                v2[i] = i * 2.5f;
            }
            
            /* Operation that might need runtime checking */
            v16f v3 = v1 + v2;
            
            /* Use volatile to prevent optimization */
            volatile v16f v = v3;
            (void)v;
        }
        #pragma GCC pop_options
    } else {
        printf("AVX512F not supported\n");
    }
}

/* Transactional memory function */
void transactional_operation(void) {
    /* This requires -fgnu-tm flag */
    __transaction_atomic {
        global_counter++;
        
        /* Nested memory access to force TM runtime */
        int temp = global_counter;
        global_counter = temp * 2;
    }
}

/* OpenMP offloading attempt */
void attempt_offload(void) {
    int n = 100;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt to offload - will likely generate fallback helpers */
    #pragma omp target map(tofrom: array[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Use result */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    (void)sum;
    
    free(array);
}

/* Function with large stack to trigger stack protection helpers */
void large_stack_function(void) {
    /* Large array to trigger stack protector */
    char buffer[4096];
    
    /* Use buffer to prevent optimization */
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i % 256;
    }
    
    /* Call external function that might use builtins */
    if (target_helper_function) {
        target_helper_function();
    }
    
    /* Use buffer content */
    volatile char c = buffer[100];
    (void)c;
}

int main(void) {
    printf("Starting target hook trigger program\n");
    
    /* 1. Check and use CPU features */
    check_and_use_avx512();
    
    /* 2. Use AVX2 vector operations */
    avx2_vector_operations();
    
    /* 3. Attempt OpenMP offloading */
    attempt_offload();
    
    /* 4. Use transactional memory */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    printf("Global counter: %d\n", global_counter);
    
    /* 5. Function with large stack */
    large_stack_function();
    
    /* 6. Use ARM/PowerPC builtins if compiled for those architectures */
    #if defined(__arm__) || defined(__aarch64__)
    {
        /* ARM specific builtin */
        unsigned int fpcr = __builtin_aarch64_get_fpcr();
        printf("FPCR: %u\n", fpcr);
    }
    #elif defined(__powerpc__) || defined(__ppc__)
    {
        /* PowerPC specific builtin */
        double d = 3.14;
        __builtin_ppc_mtfsf(0xFF, d);
    }
    #endif
    
    printf("Program completed\n");
    return 0;
}
