/* file1.c - Main program with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For transactional memory */
int global_counter = 0;

/* Weak function that will be aliased in file2.c */
void __attribute__((weak, constructor)) target_helper_init(void);

/* Vector type using GCC extensions */
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
    volatile v4si volatile_result = result;
    (void)volatile_result;
}

/* Function using AVX512 check */
void check_and_use_avx512(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Check for AVX512 - may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F is supported\n");
        
        /* Use pragma to switch target */
        #pragma GCC push_options
        #pragma GCC target("avx512f")
        {
            /* Complex vector operation in AVX512 context */
            typedef float v16sf __attribute__((vector_size(64)));
            v16sf v1 = {0}, v2 = {0};
            for (int i = 0; i < 16; i++) {
                v1[i] = i * 1.5f;
                v2[i] = i * 0.5f;
            }
            
            /* Operation that might need runtime support */
            v16sf v3 = v1 / (v2 + 1.0f);
            volatile v16sf volatile_v3 = v3;
            (void)volatile_v3;
        }
        #pragma GCC pop_options
    } else {
        printf("AVX512F not supported\n");
    }
}

/* Function with transactional memory */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested memory access in transaction */
        int local_copy = global_counter;
        global_counter = local_copy * 2;
    }
}

/* OpenMP target region */
void attempt_offload(void) {
    int n = 100;
    int *data = (int*)malloc(n * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt to offload - will likely generate fallback functions */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2 + 1;
        }
    }
    
    /* Use result to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    (void)sum;
    
    free(data);
}

/* Function with large stack usage for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char buffer[1024 * 16]; /* 16KB buffer */
    
    /* Fill buffer to prevent optimization */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (char)(i % 256);
    }
    
    /* Complex operation on buffer */
    volatile size_t check = 0;
    for (size_t i = 0; i < sizeof(buffer) - 1; i++) {
        buffer[i] = buffer[i] ^ buffer[i + 1];
        check += buffer[i];
    }
    (void)check;
}

/* ARM-specific builtins if compiled for ARM */
#ifdef __arm__
void arm_specific_operations(void) {
    /* Use ARM-specific builtins */
    unsigned int cpuid;
    __asm__ volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(cpuid));
    
    /* Alternative using GCC builtin if available */
    #ifdef __builtin_arm_mrc
    unsigned int result = __builtin_arm_mrc(15, 0, 0, 0, 0);
    volatile unsigned int vol_result = result;
    (void)vol_result;
    #endif
}
#endif

/* PowerPC-specific builtins if compiled for PowerPC */
#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* Use PowerPC builtin */
    #ifdef __builtin_ppc_mtfsf
    __builtin_ppc_mtfsf(0xFF, 0);
    #endif
}
#endif

/* AArch64-specific builtins if compiled for AArch64 */
#ifdef __aarch64__
void aarch64_specific_operations(void) {
    /* Use AArch64 builtin */
    #ifdef __builtin_aarch64_get_fpcr
    unsigned long fpcr = __builtin_aarch64_get_fpcr();
    volatile unsigned long vol_fpcr = fpcr;
    (void)vol_fpcr;
    #endif
}
#endif

int main(void) {
    printf("Starting target hook triggering program\n");
    
    /* 1. Check and use CPU features */
    check_and_use_avx512();
    
    /* 2. Use AVX2 vector operations */
    avx2_vector_operations();
    
    /* 3. Attempt OpenMP offload */
    attempt_offload();
    
    /* 4. Use transactional memory */
    transactional_operation();
    printf("Global counter after transaction: %d\n", global_counter);
    
    /* 5. Large stack usage for stack protector */
    large_stack_function();
    
    /* 6. Call weak constructor function (defined in file2.c) */
    target_helper_init();
    
    /* 7. Architecture-specific operations */
    #ifdef __arm__
    arm_specific_operations();
    #elif defined(__powerpc__)
    powerpc_specific_operations();
    #elif defined(__aarch64__)
    aarch64_specific_operations();
    #endif
    
    printf("Program completed\n");
    return 0;
}
