/* Main file with multiple patterns to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
volatile int global_counter = 0;

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Weak function declaration that will be defined in file2.c */
extern void __attribute__((weak)) weak_constructor_func(void);

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector division approximation - may need helper */
    v4si c = a / (b + 1);
    
    /* Use result to prevent optimization */
    volatile v4si result = c;
    (void)result;
    
    /* Mix with CPU feature check */
    if (__builtin_cpu_supports("avx512f")) {
        /* More complex operations for newer CPUs */
        v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf f2 = {0.5f, 0.25f, 0.125f, 0.0625f};
        v4sf f3 = f1 * f2;
        volatile v4sf fres = f3;
        (void)fres;
    }
}

/* Function with ARM-specific builtins (will compile on x86 too) */
void arm_builtin_test(void) {
    /* These will be no-ops or generate calls on appropriate targets */
#ifdef __arm__
    unsigned int val = __builtin_arm_mrc(15, 0, 0, 0, 0);
    volatile unsigned int v = val;
    (void)v;
#endif
    
#ifdef __aarch64__
    unsigned long fpcr = __builtin_aarch64_get_fpcr();
    volatile unsigned long v = fpcr;
    (void)v;
#endif
}

/* Function with PowerPC builtins */
void ppc_builtin_test(void) {
#ifdef __powerpc__
    double d = 3.14159;
    __builtin_ppc_mtfsf(0xFF, d);
#endif
}

/* OpenMP target region */
void openmp_offload_test(void) {
    int n = 100;
    int a[n], b[n], c[n];
    
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Attempt offload - may generate fallback helpers */
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) device(0)
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
void transactional_test(void) {
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operation in transaction */
        v4si v1 = {1, 2, 3, 4};
        v4si v2 = {5, 6, 7, 8};
        v4si v3 = v1 * v2 - v1 / (v2 + 1);
        volatile v4si vres = v3;
        (void)vres;
    }
}

/* Function with large stack for stack protector */
void stack_protector_test(void) {
    char large_buffer[1024 * 16];  /* Large stack frame */
    
    /* Initialize to prevent optimization */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i % 256;
    }
    
    /* Use buffer in a way that might need protection */
    volatile int checksum = 0;
    for (int i = 0; i < sizeof(large_buffer); i++) {
        checksum += large_buffer[i];
    }
    (void)checksum;
    
    /* Call another function to create stack complexity */
    avx2_vector_operations();
}

int main(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    printf("Testing target hook triggers...\n");
    
    /* Pattern 1: CPU feature checks with vector operations */
    if (__builtin_cpu_supports("avx2")) {
        avx2_vector_operations();
        printf("AVX2 operations completed\n");
    }
    
    /* Pattern 2: Architecture-specific builtins */
    arm_builtin_test();
    ppc_builtin_test();
    
    /* Pattern 3: OpenMP offloading */
    #ifdef _OPENMP
    openmp_offload_test();
    printf("OpenMP offload attempted\n");
    #endif
    
    /* Pattern 4: Transactional memory */
    transactional_test();
    printf("Transactional block executed, counter: %d\n", global_counter);
    
    /* Pattern 5: Stack protection triggers */
    stack_protector_test();
    printf("Stack protector test completed\n");
    
    /* Pattern 6: Weak symbol/constructor from another file */
    weak_constructor_func();
    
    /* Final check with all patterns combined */
    volatile int final_check = 0;
    
    #pragma omp parallel reduction(+:final_check)
    {
        v4si va = {1, 2, 3, 4};
        v4si vb = {2, 3, 4, 5};
        v4si vc = va * vb;
        
        for (int i = 0; i < 4; i++) {
            final_check += vc[i];
        }
    }
    
    printf("Final result: %d\n", final_check);
    
    return 0;
}
