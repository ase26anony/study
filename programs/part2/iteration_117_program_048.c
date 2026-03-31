/* main.c - Primary file with complex operations to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void weak_helper_function(void) __attribute__((weak));

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that may need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression with multiple operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si* volatile_ptr = &result;
    (void)volatile_ptr;
    
    /* Check CPU features - may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported\n");
    }
    
    /* Initialize CPU features */
    __builtin_cpu_init();
}

/* Function with ARM-specific built-in (will be compiled on x86 too) */
void check_arm_features(void) {
#ifdef __arm__
    /* ARM-specific built-in that may need helpers */
    unsigned int fpcr = __builtin_arm_mrc(15, 7, 0, 0, 0);
    printf("FPCR: %u\n", fpcr);
#endif
}

/* Function with PowerPC built-in */
void check_ppc_features(void) {
#ifdef __powerpc__
    /* PowerPC built-in for FPSCR */
    __builtin_ppc_mtfsf(0xFF, 0);
#endif
}

/* Transactional memory function */
void transactional_operation(void) {
    /* Use GCC transactional memory - requires runtime support */
    __transaction_atomic {
        global_counter++;
        /* Complex operation inside transaction */
        for (int i = 0; i < 100; i++) {
            global_counter += i % 10;
        }
    }
}

/* OpenMP target region */
void openmp_offload(void) {
    int n = 1000;
    int* data = (int*)malloc(n * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - may generate fallback helpers */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2 + 1;
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    (void)sum;
    
    free(data);
}

/* Function with large stack for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char buffer[4096];
    int large_array[2048];
    
    /* Use arrays to prevent optimization */
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i % 256;
    }
    
    for (int i = 0; i < 2048; i++) {
        large_array[i] = i * i;
    }
    
    /* Complex expression */
    volatile int result = 0;
    for (int i = 0; i < 2048; i++) {
        result += large_array[i] + buffer[i % sizeof(buffer)];
    }
    (void)result;
}

int main(void) {
    printf("Starting target hook test program\n");
    
    /* 1. CPU feature detection and vector operations */
    printf("1. Testing CPU features and vector operations...\n");
    avx2_vector_operations();
    
    /* 2. Architecture-specific built-ins */
    printf("2. Testing architecture-specific built-ins...\n");
    check_arm_features();
    check_ppc_features();
    
    /* 3. Transactional memory */
    printf("3. Testing transactional memory...\n");
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    printf("Global counter: %d\n", global_counter);
    
    /* 4. OpenMP offloading */
    printf("4. Testing OpenMP offloading...\n");
    openmp_offload();
    
    /* 5. Large stack usage for stack protector */
    printf("5. Testing large stack functions...\n");
    large_stack_function();
    
    /* 6. Call weak function from helper.c */
    printf("6. Calling weak helper function...\n");
    if (weak_helper_function) {
        weak_helper_function();
    } else {
        printf("Weak helper function not available\n");
    }
    
    /* 7. Complex floating point vector operations */
    printf("7. Complex floating point operations...\n");
    {
        v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf f2 = {5.0f, 6.0f, 7.0f, 8.0f};
        
        /* Trigonometric approximation using series expansion
           May generate helper functions for unsupported operations */
        v4sf result = f1 / f2 + (f1 * f1) / (f2 * f2 * f2) / 6.0f;
        
        volatile v4sf* vptr = &result;
        (void)vptr;
    }
    
    /* 8. Mixed operations with pragma */
    printf("8. Mixed operations with target pragma...\n");
    #pragma GCC push_options
    #pragma GCC target("avx2")
    {
        v4si x = {10, 20, 30, 40};
        v4si y = {2, 3, 4, 5};
        v4si z = x / y + x % (y + 1);  /* Complex integer vector ops */
        volatile v4si* vz = &z;
        (void)vz;
    }
    #pragma GCC pop_options
    
    printf("Program completed successfully\n");
    return 0;
}
