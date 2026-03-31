/* file1.c - Main program with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
#ifdef __GNUC__
#define TM_SUPPORTED 1
#else
#define TM_SUPPORTED 0
#endif

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Weak function declaration that will be defined in file2.c */
extern void __attribute__((weak)) weak_constructor_func(void);

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression that might trigger helper generation */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use volatile to prevent optimization */
    volatile v4si* volatile_ptr = &result;
    (void)volatile_ptr;
    
    printf("AVX2 vector operation completed\n");
}

/* Function using AVX512 check */
void check_avx512_features(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for AVX512 - this may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F is supported\n");
        
        /* Additional checks that might generate more helpers */
        if (__builtin_cpu_supports("avx512cd") && 
            __builtin_cpu_supports("avx512bw") &&
            __builtin_cpu_supports("avx512dq")) {
            printf("Extended AVX512 features available\n");
        }
    } else {
        printf("AVX512F not supported\n");
    }
}

/* Function with large stack usage for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char large_buffer[1024 * 16];
    
    /* Fill and use buffer to prevent optimization */
    memset(large_buffer, 0xAA, sizeof(large_buffer));
    
    /* Complex operations on buffer */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = (large_buffer[i] ^ 0x55) + i;
    }
    
    /* Use volatile to ensure operations aren't optimized away */
    volatile char* vbuf = large_buffer;
    (void)vbuf;
    
    printf("Large stack function executed\n");
}

/* Function using transactional memory */
void transactional_operation(void) {
#if TM_SUPPORTED
    printf("Starting transactional memory operation\n");
    
    /* Transactional memory block */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        for (int i = 0; i < 10; i++) {
            global_counter += i;
        }
        
        /* Call another function within transaction */
        large_stack_function();
    }
    
    printf("Transactional operation completed. Counter: %d\n", global_counter);
#else
    printf("Transactional memory not supported\n");
#endif
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int data[100];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    printf("Attempting OpenMP offload\n");
    
    /* Attempt to offload to a device - may generate fallback helpers */
    #pragma omp target map(tofrom: data[0:100]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] = data[i] * 2 + 1;
        }
    }
    
    /* Verify some results */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    printf("OpenMP offload completed. Data sum: %d\n", sum);
}

/* ARM-specific built-ins (if compiled for ARM) */
#ifdef __arm__
void arm_specific_operations(void) {
    /* ARM system register access - may need helpers */
    unsigned int value;
    value = __builtin_arm_mrc(15, 0, 0, 0, 0);
    printf("ARM MRC result: %u\n", value);
}
#endif

#ifdef __aarch64__
void aarch64_specific_operations(void) {
    /* AArch64 system register access */
    unsigned long fpcr = __builtin_aarch64_get_fpcr();
    printf("AArch64 FPCR: %lu\n", fpcr);
}
#endif

#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* PowerPC special register access */
    unsigned long value = 0xFF;
    __builtin_ppc_mtfsf(0xFF, value);
    printf("PowerPC MTFSF executed\n");
}
#endif

/* Main function orchestrating all techniques */
int main(void) {
    printf("=== Starting target hook triggering program ===\n\n");
    
    /* 1. Check CPU features (triggers built-in helpers) */
    printf("1. Checking CPU features:\n");
    check_avx512_features();
    printf("\n");
    
    /* 2. Use vector extensions with complex operations */
    printf("2. Using vector extensions:\n");
    avx2_vector_operations();
    printf("\n");
    
    /* 3. Large stack usage for stack protection */
    printf("3. Large stack function:\n");
    large_stack_function();
    printf("\n");
    
    /* 4. Transactional memory */
    printf("4. Transactional memory:\n");
    transactional_operation();
    printf("\n");
    
    /* 5. OpenMP offloading attempt */
    printf("5. OpenMP offload:\n");
    openmp_offload_attempt();
    printf("\n");
    
    /* 6. Architecture-specific operations */
    printf("6. Architecture-specific operations:\n");
    #ifdef __arm__
    arm_specific_operations();
    #endif
    #ifdef __aarch64__
    aarch64_specific_operations();
    #endif
    #ifdef __powerpc__
    powerpc_specific_operations();
    #endif
    printf("\n");
    
    /* 7. Call weak constructor function from other compilation unit */
    printf("7. Calling weak constructor function:\n");
    if (&weak_constructor_func) {
        weak_constructor_func();
    } else {
        printf("Weak function not resolved\n");
    }
    printf("\n");
    
    /* Additional complex expression mixing techniques */
    printf("8. Mixed complex expression:\n");
    {
        /* Mix CPU feature check with vector operations */
        __builtin_cpu_init();
        int use_avx = __builtin_cpu_supports("avx");
        
        v4si v1 = {1, 2, 3, 4};
        v4si v2 = {5, 6, 7, 8};
        v4si result;
        
        if (use_avx) {
            /* Complex vector expression */
            result = (v1 * v2) + (v1 / (v2 + 1));
        } else {
            result = v1 + v2;
        }
        
        volatile v4si* vr = &result;
        (void)vr;
        
        printf("Mixed expression completed\n");
    }
    
    printf("\n=== Program completed ===\n");
    
    return 0;
}
