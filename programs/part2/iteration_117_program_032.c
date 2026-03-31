/* file1.c - Main program with multiple techniques to trigger target hooks */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For transactional memory */
int global_counter = 0;

/* Weak function that will be aliased in another file */
void __attribute__((weak, constructor)) init_target_helper(void);

/* Vector operations using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Target-specific function using AVX2 */
__attribute__((target("avx2")))
v4si vector_division(v4si a, v4si b) {
    /* Complex vector operation that might need helper */
    v4si result;
    /* Simulate division using approximation - may trigger helper generation */
    for (int i = 0; i < 4; i++) {
        /* This complex operation might need runtime helper */
        result[i] = a[i] / (b[i] | 1); /* Avoid division by zero */
    }
    return result;
}

/* Function using PowerPC built-in (if compiled for PPC) */
#ifdef __powerpc__
void ppc_special_operation(void) {
    /* Use PowerPC special register built-in */
    unsigned long fpscr = __builtin_ppc_mfmsr();
    __builtin_ppc_mtfsf(0xFF, fpscr);
}
#endif

/* Function using ARM built-ins (if compiled for ARM) */
#ifdef __arm__
unsigned int arm_special_operation(void) {
    /* Access coprocessor register */
    return __builtin_arm_mrc(15, 0, 0, 0, 0);
}
#endif

#ifdef __aarch64__
unsigned long aarch64_special_operation(void) {
    /* Access FPCR register */
    return __builtin_aarch64_get_fpcr();
}
#endif

/* OpenMP target region */
void openmp_offload_test(void) {
    int n = 100;
    int arr[n];
    
    #pragma omp target map(tofrom: arr[0:n]) device(0)
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;
    }
    
    /* Use volatile to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
}

/* Transactional memory block */
void transactional_test(void) {
    __transaction_atomic {
        global_counter++;
        /* Complex operation inside transaction */
        for (int i = 0; i < 10; i++) {
            global_counter += i;
        }
    }
}

/* Large stack usage for stack protector */
void stack_protector_test(void) {
    char large_buffer[1024 * 10]; /* Large stack allocation */
    volatile int* ptr = (volatile int*)large_buffer;
    
    /* Fill with pattern */
    for (size_t i = 0; i < sizeof(large_buffer) / sizeof(int); i++) {
        ptr[i] = i * 0x12345678;
    }
    
    /* Call external function to force stack protection */
    if (init_target_helper) {
        /* Reference to prevent dead code elimination */
        printf("Helper available\n");
    }
}

int main(void) {
    volatile int result = 0;
    
    /* 1. CPU feature detection and vector operations */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx2")) {
        v4si a = {1, 2, 3, 4};
        v4si b = {5, 6, 7, 8};
        v4si c = vector_division(a, b);
        
        /* Use result to prevent optimization */
        for (int i = 0; i < 4; i++) {
            result += c[i];
        }
        printf("Vector result: %d\n", result);
    }
    
    /* 2. Architecture-specific built-ins */
#if defined(__powerpc__)
    ppc_special_operation();
    printf("PowerPC special operation executed\n");
#elif defined(__arm__)
    result += arm_special_operation();
    printf("ARM special operation executed\n");
#elif defined(__aarch64__)
    result += aarch64_special_operation();
    printf("AArch64 special operation executed\n");
#endif
    
    /* 3. OpenMP offloading */
    openmp_offload_test();
    printf("OpenMP offload test completed\n");
    
    /* 4. Transactional memory */
    transactional_test();
    printf("Transactional test completed. Counter: %d\n", global_counter);
    
    /* 5. Stack protector test */
    stack_protector_test();
    printf("Stack protector test completed\n");
    
    /* 6. Call weak/constructor function */
    if (init_target_helper) {
        printf("Target helper initialized\n");
    }
    
    /* Additional complex expression with built-ins */
    {
        /* Mix of operations that might need helpers */
        volatile long complex_result = 0;
        for (int i = 0; i < 100; i++) {
            /* Use CPU supports in loop - might generate helper */
            if (__builtin_cpu_supports("sse4.2")) {
                complex_result += i * 2;
            } else {
                complex_result += i;
            }
        }
        printf("Complex result: %ld\n", complex_result);
    }
    
    return 0;
}
