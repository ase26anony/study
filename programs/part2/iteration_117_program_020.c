/* main.c - Main program with multiple coverage techniques */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_init(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
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
}

/* Function with AVX512 target attribute */
__attribute__((target("avx512f")))
void avx512_check_and_compute(void) {
    /* Check CPU features at runtime */
    if (__builtin_cpu_supports("avx512f")) {
        __builtin_cpu_init();
        
        /* More complex vector operations */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
        
        /* Trigonometric approximation using vector operations */
        for (int i = 0; i < 4; i++) {
            /* Taylor series approximation for sin(x) */
            float x = vec1[i];
            float term = x;
            float sum = term;
            
            for (int n = 1; n < 5; n++) {
                term = -term * x * x / ((2*n) * (2*n+1));
                sum += term;
            }
            vec2[i] = sum;
        }
        
        volatile v4sf* vptr = &vec2;
        (void)vptr;
    }
}

/* Transactional memory function */
void transactional_operation(void) {
    /* Use GCC transactional memory */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation within transaction */
        int temp = global_counter;
        for (int i = 0; i < 100; i++) {
            temp = temp * 1103515245 + 12345;
        }
        global_counter = temp & 0x7fffffff;
    }
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
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
    
    /* Use result */
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
    int another_buffer[512];
    
    /* Use buffers to prevent optimization */
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i & 0xFF;
    }
    
    /* Complex operation on buffer */
    for (int i = 0; i < 512; i++) {
        another_buffer[i] = buffer[i * 2] + buffer[i * 2 + 1];
    }
    
    volatile int* vptr = another_buffer;
    (void)vptr;
}

/* Main function with conditional execution paths */
int main(void) {
    printf("Starting coverage test...\n");
    
    /* Path 1: CPU feature detection and vector operations */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported, running vector operations\n");
        avx2_vector_operations();
    }
    
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported, running advanced operations\n");
        avx512_check_and_compute();
    }
    
    /* Path 2: Transactional memory */
    printf("Testing transactional memory\n");
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    printf("Global counter: %d\n", global_counter);
    
    /* Path 3: OpenMP offload attempt */
    printf("Attempting OpenMP offload\n");
    openmp_offload_attempt();
    
    /* Path 4: Large stack operations */
    printf("Testing stack protection\n");
    large_stack_function();
    
    /* Path 5: Call weak external function */
    printf("Calling target helper init\n");
    if (&target_helper_init) {
        target_helper_init();
    }
    
    /* Additional architecture-specific built-ins */
    #ifdef __arm__
    /* ARM-specific built-in */
    unsigned int fpscr = __builtin_arm_mrc(15, 0, 1, 0, 0);
    printf("ARM FPSCR: %u\n", fpscr);
    #elif defined(__aarch64__)
    /* AArch64-specific built-in */
    unsigned long fpcr = __builtin_aarch64_get_fpcr();
    printf("AArch64 FPCR: %lu\n", fpcr);
    #elif defined(__powerpc__) || defined(__PPC__)
    /* PowerPC-specific built-in */
    __builtin_ppc_mtfsf(0xFF, 0);
    printf("PowerPC MTFSF executed\n");
    #endif
    
    printf("Test completed\n");
    return 0;
}
