/* main.c - Primary file with complex operations to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_init(void);

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
}

/* Function using AVX-512 builtins */
void check_avx512_features(void) {
    /* Force CPU initialization and feature checking */
    __builtin_cpu_init();
    
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX-512F is supported\n");
        
        /* This might trigger helper generation for unsupported operations */
        #pragma GCC target("avx512f")
        {
            /* Vector operations that might need runtime support */
            typedef float v16sf __attribute__((vector_size(64)));
            v16sf v1 = {}, v2 = {};
            volatile v16sf* vptr = &v1;
            (void)vptr;
        }
    } else {
        printf("AVX-512F not supported\n");
    }
}

/* Transactional memory function */
void transactional_operation(void) {
    /* GCC Transactional Memory - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {0.5f, 0.25f, 0.125f, 0.0625f};
        v4sf result = vec1 * vec2;
        
        volatile v4sf* vptr = &result;
        (void)vptr;
    }
}

/* OpenMP target region */
void attempt_offload(void) {
    int n = 100;
    int* data = (int*)malloc(n * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - will likely generate fallback helpers */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] *= 2;
        }
    }
    
    /* Use data to prevent optimization */
    volatile int* vptr = &data[0];
    (void)vptr;
    
    free(data);
}

/* Function with large stack usage for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char buffer[4096];
    int i;
    
    /* Use buffer to prevent optimization */
    for (i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i % 256;
    }
    
    /* Complex operation with buffer */
    v4si vec = {0};
    for (i = 0; i < 4; i++) {
        vec[i] = buffer[i * 1024];
    }
    
    volatile v4si* vptr = &vec;
    (void)vptr;
}

int main(void) {
    printf("Starting target hook triggering program...\n");
    
    /* 1. Call weak constructor function from helper.c */
    if (target_helper_init) {
        target_helper_init();
    }
    
    /* 2. Check CPU features and use vector operations */
    check_avx512_features();
    
    /* 3. Use AVX2 vector operations */
    avx2_vector_operations();
    
    /* 4. Perform transactional memory operations */
    for (int i = 0; i < 5; i++) {
        transactional_operation();
    }
    printf("Global counter after transactions: %d\n", global_counter);
    
    /* 5. Attempt OpenMP offload */
    #ifdef _OPENMP
    attempt_offload();
    #endif
    
    /* 6. Use large stack to trigger stack protection helpers */
    large_stack_function();
    
    /* 7. Use ARM-specific builtins if compiled for ARM */
    #if defined(__arm__) || defined(__aarch64__)
    {
        /* ARM system register access - may need helper */
        unsigned long fpcr;
        #ifdef __aarch64__
        fpcr = __builtin_aarch64_get_fpcr();
        #else
        fpcr = __builtin_arm_mrc(15, 0, 0, 1, 0);
        #endif
        printf("ARM FPCR/control register: 0x%lx\n", fpcr);
    }
    #endif
    
    /* 8. Use PowerPC builtins if compiled for PPC */
    #ifdef __powerpc__
    {
        /* PowerPC special register access */
        unsigned long fpscr = 0;
        __builtin_ppc_mtfsf(0xFF, fpscr);
        printf("PowerPC FPSCR modified\n");
    }
    #endif
    
    printf("Program completed successfully\n");
    return 0;
}
