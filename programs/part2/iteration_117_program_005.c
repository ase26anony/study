/* main.c - Main program with multiple patterns to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) weak_helper_function(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
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
    
    /* Check CPU features - may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        /* This built-in may require runtime initialization */
        __builtin_cpu_init();
    }
}

/* Function with transactional memory */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        for (int i = 0; i < 10; i++) {
            global_counter += i;
        }
    }
}

/* Function with large stack for stack protection */
void stack_protected_function(void) {
    /* Large array to trigger stack protection */
    char large_buffer[4096];
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i % 256;
    }
    
    /* Use buffer in volatile way */
    volatile char* vptr = large_buffer;
    (void)vptr;
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int data[100];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - may generate fallback functions */
    #pragma omp target map(tofrom: data[0:100]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] *= 2;
        }
    }
    
    /* Use result */
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* ARM-specific built-ins (if compiled for ARM) */
#ifdef __arm__
void arm_specific_operations(void) {
    /* ARM system register access - may need helper */
    unsigned int val = __builtin_arm_mrc(15, 0, 0, 0, 0);
    volatile unsigned int* v = &val;
    (void)v;
}
#endif

/* PowerPC specific (if compiled for PPC) */
#ifdef __powerpc__
void ppc_specific_operations(void) {
    /* PowerPC special register access */
    double d = 1.0;
    __builtin_ppc_mtfsf(0xFF, d);
}
#endif

/* Main function with conditional execution paths */
int main(int argc, char** argv) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Path 1: AVX2 vector operations if supported */
    if (__builtin_cpu_supports("avx2")) {
        avx2_vector_operations();
        printf("AVX2 operations executed\n");
    }
    
    /* Path 2: Transactional memory */
    transactional_operation();
    printf("Transactional operation completed. Counter: %d\n", global_counter);
    
    /* Path 3: Stack protection trigger */
    stack_protected_function();
    printf("Stack protected function executed\n");
    
    /* Path 4: OpenMP offload attempt */
    #ifdef _OPENMP
    openmp_offload_attempt();
    printf("OpenMP offload attempted\n");
    #endif
    
    /* Path 5: Call weak helper function */
    if (&weak_helper_function) {
        weak_helper_function();
        printf("Weak helper function called\n");
    }
    
    /* Path 6: Architecture-specific operations */
    #ifdef __arm__
    arm_specific_operations();
    printf("ARM-specific operations executed\n");
    #endif
    
    #ifdef __powerpc__
    ppc_specific_operations();
    printf("PowerPC-specific operations executed\n");
    #endif
    
    /* Complex conditional to prevent dead code elimination */
    volatile int runtime_check = argc > 1 ? atoi(argv[1]) : 0;
    
    if (runtime_check > 0) {
        /* More vector operations with different target attributes */
        #pragma GCC push_options
        #pragma GCC target("sse4.2")
        {
            v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
            v4sf fresult = fvec * fvec + fvec / (fvec + 1.0f);
            volatile v4sf* vf = &fresult;
            (void)vf;
        }
        #pragma GCC pop_options
    }
    
    return 0;
}
