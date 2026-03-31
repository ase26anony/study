/* file1.c - Main program with multiple patterns to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
volatile int global_counter = 0;

/* Weak function declaration that will be defined in file2.c */
extern void __attribute__((weak)) weak_target_helper(void);

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Function using AVX2 target attribute and vector extensions */
__attribute__((target("avx2")))
static v4si vector_division(v4si a, v4si b) {
    /* Complex vector operation that might need helper functions */
    v4si result;
    /* Simulate division using approximation - may trigger helper generation */
    for (int i = 0; i < 4; i++) {
        /* This complex operation might need runtime support */
        result[i] = a[i] / (b[i] != 0 ? b[i] : 1);
    }
    return result;
}

/* Function with AVX512 check and complex operations */
static void check_and_use_avx512(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Check for AVX512 support - may trigger helper generation */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F is supported\n");
        
        /* Use vector extensions with complex operations */
        v4si vec1 = {100, 200, 300, 400};
        v4si vec2 = {3, 4, 5, 6};
        v4si result = vector_division(vec1, vec2);
        
        /* Prevent dead code elimination */
        volatile v4si volatile_result = result;
        (void)volatile_result;
    } else {
        printf("AVX512F not supported\n");
    }
}

/* OpenMP target region - will likely generate fallback helpers */
static void attempt_offload(void) {
    int data[100];
    
    #pragma omp target map(tofrom: data[0:100]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] = i * 2;
        }
    }
    
    /* Use data to prevent elimination */
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* Transactional memory block */
static void transactional_operation(void) {
    __transaction_atomic {
        global_counter++;
        /* Complex operation inside transaction */
        for (int i = 0; i < 10; i++) {
            global_counter += i;
        }
    }
}

/* Function with large stack usage to trigger stack protection helpers */
static void stack_intensive_function(void) {
    char large_buffer[4096];  /* Large stack allocation */
    volatile int* ptr = (volatile int*)large_buffer;
    
    /* Initialize and use buffer */
    for (int i = 0; i < 1024; i++) {
        ptr[i] = i;
    }
    
    /* Call external function that might use target builtins */
    if (weak_target_helper) {
        weak_target_helper();
    }
}

int main(void) {
    printf("Starting target hook trigger program\n");
    
    /* Pattern 1: CPU feature detection and vector operations */
    check_and_use_avx512();
    
    /* Pattern 2: OpenMP offloading attempt */
    #ifdef _OPENMP
    attempt_offload();
    #endif
    
    /* Pattern 3: Transactional memory */
    #ifdef __GNUC__
    transactional_operation();
    printf("Global counter after transaction: %d\n", global_counter);
    #endif
    
    /* Pattern 4: Stack intensive operations */
    stack_intensive_function();
    
    /* Pattern 5: Call weak function from other compilation unit */
    if (weak_target_helper) {
        printf("Weak helper function called\n");
    }
    
    /* Additional ARM-specific builtin if compiled for ARM */
    #ifdef __arm__
    {
        unsigned int reg_value;
        __builtin_arm_mrc(15, 0, reg_value, 0, 0, 0);
        printf("ARM MRC instruction executed\n");
    }
    #endif
    
    /* Additional PowerPC-specific builtin if compiled for PowerPC */
    #ifdef __powerpc__
    {
        __builtin_ppc_mtfsf(0xFF, 0.0);
        printf("PowerPC MTFSF instruction executed\n");
    }
    #endif
    
    printf("Program completed\n");
    return 0;
}
