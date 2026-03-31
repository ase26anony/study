/* main.c - Primary file with multiple coverage techniques */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
int global_counter = 0;

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_function(void);

/* Function using AVX2 intrinsics through target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression mixing operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si volatile_result = result;
    (void)volatile_result;
}

/* Function using CPU feature detection */
void cpu_feature_dependent_code(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for various features - may generate helper functions */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Use the results in conditional compilation */
    if (has_avx2) {
        avx2_vector_operations();
    }
    
    /* Force generation of helpers by using in complex expression */
    volatile int features = has_avx512 + has_avx2 * 2 + has_sse4 * 3;
    (void)features;
}

/* OpenMP target region - will generate fallback helpers if offloading not supported */
void openmp_offload_attempt(void) {
    int data[100];
    
    #pragma omp parallel for
    for (int i = 0; i < 100; i++) {
        data[i] = i * 2;
    }
    
    /* Attempt offloading - may generate hidden helper functions */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:100]) device(0)
    for (int i = 0; i < 100; i++) {
        data[i] += i;
    }
    
    /* Use data to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* Transactional Memory section */
void transactional_operations(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        for (int i = 0; i < 10; i++) {
            global_counter += i;
        }
    }
    
    /* Another transaction with more complexity */
    __transaction_atomic {
        int local_array[50];
        for (int i = 0; i < 50; i++) {
            local_array[i] = global_counter + i;
        }
        
        /* Use vector operations inside transaction */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {0.5f, 1.5f, 2.5f, 3.5f};
        v4sf vec_result = vec1 * vec2 + vec1 / (vec2 + 1.0f);
        
        volatile v4sf volatile_vec = vec_result;
        (void)volatile_vec;
    }
}

/* Function with stack protector trigger */
void stack_protector_trigger(void) {
    /* Large array to trigger stack protection */
    char buffer[256];
    
    /* Fill with pattern */
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i % 256;
    }
    
    /* Call external function that might be optimized */
    if (target_helper_function) {
        target_helper_function();
    }
    
    /* Use buffer to prevent optimization */
    volatile int checksum = 0;
    for (int i = 0; i < sizeof(buffer); i++) {
        checksum += buffer[i];
    }
    (void)checksum;
}

int main(void) {
    printf("Starting target hook coverage program...\n");
    
    /* Execute all coverage techniques */
    cpu_feature_dependent_code();
    openmp_offload_attempt();
    transactional_operations();
    stack_protector_trigger();
    
    /* ARM-specific built-in if compiled for ARM */
    #ifdef __arm__
    {
        unsigned int p15_c0_value;
        /* Access coprocessor register - may need helper */
        __asm__ volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(p15_c0_value));
        printf("ARM CP15 register: %u\n", p15_c0_value);
    }
    #endif
    
    /* PowerPC specific if compiled for PPC */
    #ifdef __powerpc__
    {
        /* Manipulate FPSCR */
        unsigned long fpscr = 0;
        __builtin_ppc_mtfsf(0xFF, fpscr);
    }
    #endif
    
    /* AArch64 specific */
    #ifdef __aarch64__
    {
        unsigned long fpcr = __builtin_aarch64_get_fpcr();
        printf("AArch64 FPCR: %lu\n", fpcr);
    }
    #endif
    
    printf("Program completed. Global counter: %d\n", global_counter);
    
    return 0;
}
