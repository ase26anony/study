/* main.c - Primary file with complex target-specific operations */
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
    volatile v4si* volatile_ptr = &result;
    (void)volatile_ptr;
}

/* Function checking CPU features */
void check_and_use_cpu_features(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check for various features - each might trigger helper generation */
    if (__builtin_cpu_supports("avx512f")) {
        printf("AVX512F supported\n");
        /* Complex operation that might need runtime helper */
        unsigned long long r = 0;
        /* Use inline assembly to ensure builtin is used */
        __asm__ volatile("" : "=r"(r) : "0"(r));
    }
    
    if (__builtin_cpu_supports("avx2")) {
        printf("AVX2 supported\n");
        avx2_vector_operations();
    }
    
    /* ARM-specific builtins (will be ignored on x86 but parsed) */
    #ifdef __arm__
    unsigned int p15_c0 = __builtin_arm_mrc(15, 0, 0, 0, 0);
    volatile unsigned int* vp = &p15_c0;
    (void)vp;
    #endif
    
    /* PowerPC-specific builtins */
    #ifdef __powerpc__
    __builtin_ppc_mtfsf(0xFF, 0.0);
    #endif
}

/* OpenMP target region - will generate fallback helpers */
void openmp_offload_attempt(void) {
    int n = 100;
    int a[n], b[n], c[n];
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Attempt offload - compiler may generate hidden helpers */
    #pragma omp target device(0) map(to: a, b) map(from: c)
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Use result */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += c[i];
    }
    (void)sum;
}

/* Transactional memory section */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation */
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {0.5f, 1.5f, 2.5f, 3.5f};
        v4sf vec3 = vec1 * vec2;
        
        volatile v4sf* v = &vec3;
        (void)v;
    }
}

/* Function with large stack for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char buffer[1024 * 10];
    
    /* Use buffer to prevent optimization */
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i % 256;
    }
    
    volatile char* vbuf = buffer;
    (void)vbuf;
}

int main(void) {
    printf("Starting target hook test program\n");
    
    /* 1. Check and use CPU features */
    check_and_use_cpu_features();
    
    /* 2. Call weak function from helper.c */
    if (&target_helper_init) {
        target_helper_init();
    }
    
    /* 3. Attempt OpenMP offload */
    #ifdef _OPENMP
    openmp_offload_attempt();
    #endif
    
    /* 4. Use transactional memory */
    #ifdef __TM_FENCE__
    transactional_operation();
    printf("Global counter: %d\n", global_counter);
    #endif
    
    /* 5. Function with large stack */
    large_stack_function();
    
    /* 6. Additional complex vector math that might need helpers */
    {
        v4si x = {100, 200, 300, 400};
        v4si y = {3, 3, 3, 3};
        
        /* Complex operation: (x * y) + (x / y) - might need helper */
        v4si z = (x * y) + (x / y);
        
        volatile v4si* vz = &z;
        (void)vz;
    }
    
    printf("Program completed\n");
    return 0;
}
