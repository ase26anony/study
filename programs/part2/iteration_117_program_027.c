/* main.c - Primary file with multiple coverage techniques */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper(void);

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
        printf("AVX512F supported at runtime\n");
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
    printf("Global counter after transaction: %d\n", global_counter);
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 100;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma omp target map(tofrom: data[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = i * 2;
        }
    }
    
    /* Use data to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    printf("OpenMP offload result (partial sum): %d\n", sum % 1000);
    
    free(data);
}

/* Function with stack protector trigger */
void stack_protector_test(void) {
    /* Large array to trigger stack protection */
    char buffer[256];
    volatile int i;
    
    for (i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i & 0xFF;
    }
    
    /* Call external function that may not exist (weak) */
    if (target_helper) {
        target_helper();
    }
}

/* ARM-specific built-in if compiled for ARM */
#ifdef __arm__
void arm_specific_operations(void) {
    /* ARM system register access - may need helper */
    unsigned int val;
    __asm__ volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(val));
    printf("ARM MIDR: 0x%08x\n", val);
}
#endif

/* PowerPC specific if compiled for PPC */
#ifdef __powerpc__
void ppc_specific_operations(void) {
    /* PPC special register access */
    unsigned long fpscr;
    __asm__ volatile("mffs %0" : "=f"(fpscr));
    printf("PPC FPSCR: 0x%08lx\n", fpscr);
}
#endif

int main(void) {
    printf("Starting target hook coverage test...\n");
    
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Execute different code paths based on runtime checks */
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 detected, running vector operations\n");
        avx2_vector_operations();
    } else if (__builtin_cpu_supports("sse2")) {
        printf("SSE2 detected\n");
    }
    
    /* Try OpenMP offload */
    openmp_offload_attempt();
    
    /* Transactional memory operation */
    transactional_operation();
    
    /* Stack protector test */
    stack_protector_test();
    
    /* Architecture-specific operations */
    #ifdef __arm__
    arm_specific_operations();
    #elif defined(__powerpc__)
    ppc_specific_operations();
    #endif
    
    /* Complex expression with builtins */
    volatile int use_builtin = __builtin_popcount(0x12345678);
    printf("Population count: %d\n", use_builtin);
    
    /* Force generation of math helpers with vector division */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_result = vec1 / vec2;
    
    volatile v4sf* vol_vec = &vec_result;
    (void)vol_vec;
    
    printf("Test completed.\n");
    return 0;
}
