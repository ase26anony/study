/* main.c - Primary file with complex operations to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_init(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with AVX2 target attribute - may require runtime checks */
__attribute__((target("avx2")))
static v4si avx2_vector_divide(v4si a, v4si b) {
    /* Integer vector division - may require helper function */
    v4si result;
    for (int i = 0; i < 4; i++) {
        /* Complex division with safety check */
        result[i] = b[i] != 0 ? a[i] / (b[i] + 1) : 0;
    }
    return result;
}

/* Function using CPU feature detection */
static void cpu_feature_demo(void) {
    /* Force __builtin_cpu_init generation */
    __builtin_cpu_init();
    
    /* Check multiple CPU features - may generate helper calls */
    int has_avx = __builtin_cpu_supports("avx");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_avx512f = __builtin_cpu_supports("avx512f");
    
    printf("CPU Features: AVX=%d, AVX2=%d, AVX512F=%d\n", 
           has_avx, has_avx2, has_avx512f);
    
    if (has_avx2) {
        /* Use vector extensions with complex operations */
        v4si vec1 = {100, 200, 300, 400};
        v4si vec2 = {3, 5, 7, 9};
        v4si result = avx2_vector_divide(vec1, vec2);
        
        /* Prevent optimization */
        volatile v4si volatile_result = result;
        printf("Vector result: %d, %d, %d, %d\n", 
               result[0], result[1], result[2], result[3]);
    }
}

/* Transactional memory function */
static void transactional_demo(void) {
    /* GCC Transactional Memory - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        for (int i = 0; i < 10; i++) {
            global_counter += i % 3;
        }
    }
    
    printf("Transactional counter: %d\n", global_counter);
}

/* OpenMP offloading attempt */
static void openmp_offload_demo(void) {
    int n = 1000;
    float *array = (float*)malloc(n * sizeof(float));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = (float)i / 10.0f;
    }
    
    /* Attempt offload - may generate fallback helpers */
    #pragma omp target device(0) map(tofrom: array[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            /* Complex math that might need runtime support */
            array[i] = array[i] * array[i] / (array[i] + 1.0f);
        }
    }
    
    /* Use result to prevent elimination */
    volatile float sum = 0;
    for (int i = 0; i < n; i += 100) {
        sum += array[i];
    }
    
    printf("OpenMP result sample: %f\n", sum);
    free(array);
}

/* Function with large stack for stack protector */
static void stack_protector_demo(void) {
    /* Large array to trigger stack protection */
    char buffer[1024 * 16];  /* 16KB buffer */
    int secret = 42;
    
    /* Fill buffer in a way that might trigger protections */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (char)((i * secret) & 0xFF);
    }
    
    /* Complex operation on buffer */
    volatile long checksum = 0;
    for (size_t i = 0; i < sizeof(buffer); i += 256) {
        checksum += buffer[i];
    }
    
    printf("Stack protector checksum: %ld\n", checksum);
}

int main(void) {
    printf("=== Starting target hook trigger program ===\n");
    
    /* 1. Call weak constructor function from helper.c */
    if (target_helper_init) {
        target_helper_init();
    }
    
    /* 2. CPU feature detection and vector operations */
    cpu_feature_demo();
    
    /* 3. Transactional memory */
    transactional_demo();
    
    /* 4. OpenMP offloading attempt */
    openmp_offload_demo();
    
    /* 5. Stack protector demo */
    stack_protector_demo();
    
    /* 6. Additional ARM/PowerPC built-ins if compiled for those arches */
    #if defined(__arm__) || defined(__aarch64__)
    /* ARM specific built-in */
    unsigned int fpcr = 0;
    #ifdef __aarch64__
    fpcr = __builtin_aarch64_get_fpcr();
    #else
    fpcr = __builtin_arm_mrc(15, 7, 0, 0, 0);
    #endif
    printf("ARM FPCR/register: %u\n", fpcr);
    #elif defined(__powerpc__) || defined(__ppc__)
    /* PowerPC specific built-in */
    double d = 3.14159;
    __builtin_ppc_mtfsf(0xFF, d);
    printf("PowerPC MTFSF executed\n");
    #endif
    
    printf("=== Program completed ===\n");
    return 0;
}
