/* main.c - Primary file with multiple coverage techniques */
#include <stdio.h>
#include <stdlib.h>
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
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression with multiple operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si *volatile ptr = &result;
    (void)ptr;
}

/* Function with AVX512 target attribute */
__attribute__((target("avx512f")))
void avx512_check_and_compute(void) {
    /* Check CPU features at runtime */
    if (__builtin_cpu_supports("avx512f")) {
        __builtin_cpu_init();
        
        /* Use vector extensions with complex operations */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
        
        /* Trigonometric approximation using Taylor series
           This might require helper functions */
        v4sf angle = vec1 * 0.1f;
        v4sf sin_approx = angle - (angle*angle*angle)/6.0f 
                         + (angle*angle*angle*angle*angle)/120.0f;
        
        volatile v4sf *volatile vptr = &sin_approx;
        (void)vptr;
    }
}

/* OpenMP target region */
void attempt_offload(void) {
    int n = 100;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt to offload - may generate fallback helpers */
    #pragma omp target device(0) map(tofrom: array[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Use result */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    
    free(array);
}

/* Transactional memory function */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local = global_counter;
        for (int i = 0; i < 10; i++) {
            local += i;
        }
        global_counter = local;
    }
}

/* Function with large stack for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char buffer[1024];
    int another_buffer[256];
    
    /* Use buffers to prevent optimization */
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i % 256;
    }
    
    /* Complex operations */
    for (int i = 0; i < 256; i++) {
        another_buffer[i] = buffer[i] * 2 + global_counter;
    }
    
    /* Volatile use */
    volatile int *vptr = another_buffer;
    (void)vptr;
}

int main(void) {
    printf("Starting target hook coverage test...\n");
    
    /* 1. Call weak constructor function from helper.c */
    if (&target_helper_init) {
        target_helper_init();
    }
    
    /* 2. Execute AVX2 operations */
    avx2_vector_operations();
    
    /* 3. Check and use AVX512 if available */
    avx512_check_and_compute();
    
    /* 4. Attempt OpenMP offload */
    attempt_offload();
    
    /* 5. Use transactional memory */
    for (int i = 0; i < 5; i++) {
        transactional_operation();
    }
    
    /* 6. Function with large stack */
    large_stack_function();
    
    /* 7. Additional target-specific built-ins for various architectures */
    
    /* ARM-specific (will be compiled out on x86 but still parsed) */
    #ifdef __arm__
    {
        unsigned int fpscr = __builtin_arm_mrc(15, 7, 5, 0, 0);
        volatile unsigned int *vfpscr = &fpscr;
        (void)vfpscr;
    }
    #endif
    
    /* PowerPC specific */
    #ifdef __powerpc__
    {
        __builtin_ppc_mtfsf(0xFF, 0.0);
    }
    #endif
    
    /* AArch64 specific */
    #ifdef __aarch64__
    {
        unsigned long fpcr = __builtin_aarch64_get_fpcr();
        volatile unsigned long *vfpcr = &fpcr;
        (void)vfpcr;
    }
    #endif
    
    printf("Test completed. Global counter: %d\n", global_counter);
    
    return 0;
}
