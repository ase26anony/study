/* main.c - Main program with multiple patterns to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
static void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression with multiple operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si dummy = result;
    (void)dummy;
}

/* Function using CPU feature detection */
static void cpu_feature_dependent_code(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for various CPU features */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported\n");
        /* This might trigger helper generation for AVX512 */
        volatile int has_avx512 = 1;
        (void)has_avx512;
    }
    
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
        avx2_vector_operations();
    }
    
    /* ARM-specific built-in (will be ignored on x86 but parsed) */
#ifdef __arm__
    unsigned int pfr0 = __builtin_arm_mrc(15, 0, 0, 0, 0);
    volatile unsigned int dummy = pfr0;
    (void)dummy;
#endif
    
    /* PowerPC specific (will be ignored on other arches) */
#ifdef __powerpc__
    __builtin_ppc_mtfsf(0xFF, 0.0);
#endif
}

/* Function with OpenMP target region */
static void openmp_offload_attempt(void) {
    int n = 100;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt offloading - compiler may generate fallback helpers */
    #pragma omp target device(0) map(tofrom: array[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    (void)sum;
    
    free(array);
}

/* Function using transactional memory */
static void transactional_memory_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation */
        volatile int temp = global_counter * 2;
        global_counter = temp / 2;
    }
    
    /* Another transaction with more complexity */
    __transaction_atomic {
        for (int i = 0; i < 10; i++) {
            global_counter += i;
        }
    }
}

/* Function with large stack usage for stack protector */
static void stack_protector_test(void) {
    /* Large array to trigger stack protection */
    char large_buffer[4096];
    volatile int* volatile_ptr = (volatile int*)large_buffer;
    
    /* Fill with pattern */
    for (size_t i = 0; i < sizeof(large_buffer) / sizeof(int); i++) {
        volatile_ptr[i] = i * 0x12345678;
    }
    
    /* Call external function that might be weak/aliased */
    if (target_helper_init) {
        target_helper_init();
    }
    
    /* Complex operation to prevent optimization */
    volatile int checksum = 0;
    for (size_t i = 0; i < sizeof(large_buffer) / sizeof(int); i++) {
        checksum ^= volatile_ptr[i];
    }
    (void)checksum;
}

int main(void) {
    printf("Starting target hook test program\n");
    
    /* Execute all patterns in sequence */
    cpu_feature_dependent_code();
    
    openmp_offload_attempt();
    
    transactional_memory_operation();
    
    stack_protector_test();
    
    /* Print results to prevent optimization */
    printf("Global counter: %d\n", global_counter);
    
    /* Force use of vector extensions in main */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_result = vec1 * vec2 + vec1 / (vec2 + 1.0f);
    
    volatile float result_element = vec_result[0];
    printf("Vector result element: %f\n", result_element);
    
    return 0;
}
