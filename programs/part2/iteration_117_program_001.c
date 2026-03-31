#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Declare function from helper file */
extern void weak_constructor_function(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

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
    volatile v4si dummy = result;
    (void)dummy;
}

/* Function using AVX512 check */
void check_and_use_avx512(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Check for AVX512 support - may generate helper */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F is supported\n");
        
        /* Use vector extensions with complex operations */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
        
        /* Trigonometric approximation using Taylor series
           This complex operation might need helper functions */
        v4sf angle = vec1 * 0.017453292519943295f; /* degrees to radians */
        v4sf angle_sq = angle * angle;
        v4sf sin_approx = angle - (angle_sq * angle) / 6.0f 
                         + (angle_sq * angle_sq * angle) / 120.0f;
        
        volatile v4sf dummy = sin_approx;
        (void)dummy;
    } else {
        printf("AVX512F not supported\n");
    }
}

/* Function with large stack array to trigger stack protection */
void function_with_large_stack(void) {
    char large_buffer[4096]; /* Large stack allocation */
    volatile int i;
    
    /* Fill buffer to prevent optimization */
    for (i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i & 0xFF;
    }
    
    /* Use buffer in a way that might need protection */
    memcpy(&large_buffer[2048], &large_buffer[0], 2048);
    
    volatile int sum = 0;
    for (i = 0; i < sizeof(large_buffer); i++) {
        sum += large_buffer[i];
    }
    (void)sum;
}

int main(void) {
    printf("Starting target hook test program\n");
    
    /* 1. Check and use CPU features with builtins */
    check_and_use_avx512();
    
    /* 2. Use AVX2 vector operations */
    avx2_vector_operations();
    
    /* 3. OpenMP target region - may generate fallback helpers */
    #pragma omp target device(0) map(tofrom: global_counter)
    {
        /* Simple parallel operation in target region */
        #pragma omp parallel for
        for (int i = 0; i < 10; i++) {
            /* Use atomic to ensure operation isn't optimized away */
            #pragma omp atomic
            global_counter += i;
        }
    }
    printf("OpenMP target result: %d\n", global_counter);
    
    /* 4. Transactional memory block */
    __transaction_atomic {
        global_counter++;
        /* Nested complex operation inside transaction */
        function_with_large_stack();
    }
    printf("After transaction: %d\n", global_counter);
    
    /* 5. Call weak constructor function from another file */
    weak_constructor_function();
    
    /* 6. Additional architecture-specific builtins */
    #if defined(__arm__)
    /* ARM-specific builtin */
    unsigned int fpscr = __builtin_arm_get_fpscr();
    printf("ARM FPSCR: %u\n", fpscr);
    #elif defined(__powerpc__) || defined(__PPC__)
    /* PowerPC-specific builtin */
    __builtin_ppc_mtfsf(0xFF, 0);
    #elif defined(__aarch64__)
    /* AArch64-specific builtin */
    unsigned long fpcr = __builtin_aarch64_get_fpcr();
    printf("AArch64 FPCR: %lu\n", fpcr);
    #endif
    
    /* 7. More complex vector operations that might need helpers */
    {
        v4si v1 = {100, 200, 300, 400};
        v4si v2 = {2, 3, 4, 5};
        
        /* Division on integer vectors often needs runtime helpers */
        v4si div_result = v1 / v2;
        
        /* Modulo operation also often needs helpers */
        v4si mod_result = v1 % v2;
        
        volatile v4si dummy1 = div_result;
        volatile v4si dummy2 = mod_result;
        (void)dummy1;
        (void)dummy2;
    }
    
    printf("Program completed\n");
    return 0;
}
