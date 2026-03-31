/* main.c - Primary file with complex patterns to trigger target hooks */
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

/* Function with AVX2 target attribute - may require runtime checking */
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

/* Function using CPU feature detection */
void cpu_feature_dependent_code(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Check for various features - may generate helper calls */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Use the results in a complex way */
    if (has_avx512) {
        #pragma GCC target("avx512f")
        {
            /* Vector operations with AVX512 */
            typedef float v16sf __attribute__((vector_size(64)));
            v16sf v1 = {}, v2 = {};
            v16sf v3 = v1 * v2 + v1 / (v2 + 1.0f);
            volatile v16sf* vp = &v3;
            (void)vp;
        }
    } else if (has_avx2) {
        avx2_vector_operations();
    }
    
    /* Print features (prevents dead code elimination) */
    printf("CPU Features - AVX512: %d, AVX2: %d, SSE4.2: %d\n", 
           has_avx512, has_avx2, has_sse4);
}

/* Function with transactional memory */
void transactional_operations(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested memory access pattern */
        int local_array[256];
        for (int i = 0; i < 256; i++) {
            local_array[i] = i * global_counter;
        }
        
        /* Force stack protector due to large array */
        volatile int sum = 0;
        for (int i = 0; i < 256; i++) {
            sum += local_array[i];
        }
    }
    
    printf("Global counter after transaction: %d\n", global_counter);
}

/* OpenMP target region - may generate fallback helpers */
void openmp_offload_attempt(void) {
    int n = 1000;
    int* data = (int*)malloc(n * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - will likely use host fallback */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2 + 1;
        }
    }
    
    /* Verify some results */
    volatile int check = 0;
    for (int i = 0; i < 10; i++) {
        check += data[i];
    }
    
    printf("OpenMP offload check sum (first 10): %d\n", check);
    free(data);
}

/* ARM-specific if compiled for ARM */
#ifdef __arm__
void arm_specific_operations(void) {
    /* ARM system register access - may need helper */
    unsigned int fpcr;
    #ifdef __aarch64__
    fpcr = __builtin_aarch64_get_fpcr();
    #else
    fpcr = __builtin_arm_mrc(15, 7, 0, 0, 0);
    #endif
    printf("ARM FPCR/register: 0x%08x\n", fpcr);
}
#endif

/* PowerPC-specific if compiled for PowerPC */
#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* PowerPC special register access */
    unsigned long fpscr = 0;
    __builtin_ppc_mtfsf(0xFF, fpscr);
    printf("PowerPC FPSCR updated\n");
}
#endif

int main(void) {
    printf("Starting target hook triggering program...\n");
    
    /* 1. CPU feature detection with builtins */
    cpu_feature_dependent_code();
    
    /* 2. Call weak function from helper.c */
    if (&target_helper_init) {
        target_helper_init();
    }
    
    /* 3. Transactional memory operations */
    transactional_operations();
    
    /* 4. OpenMP offload attempt */
    openmp_offload_attempt();
    
    /* 5. Architecture-specific operations */
    #ifdef __arm__
    arm_specific_operations();
    #endif
    
    #ifdef __powerpc__
    powerpc_specific_operations();
    #endif
    
    /* 6. Additional vector operations with pragmas */
    #pragma GCC push_options
    #pragma GCC target("avx2")
    {
        v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf f2 = {5.0f, 6.0f, 7.0f, 8.0f};
        v4sf f3 = f1 / f2 + f1 * f2 - f2 / f1;
        
        volatile v4sf* vf = &f3;
        (void)vf;
    }
    #pragma GCC pop_options
    
    printf("Program completed successfully.\n");
    return 0;
}
