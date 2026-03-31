/* Main file with multiple patterns to trigger target hook generation */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Pattern 1: Target-specific built-ins and vector extensions */
#ifdef __x86_64__
#include <x86intrin.h>
#endif

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Pattern 2: Vector extensions with complex operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Pattern 3: Function using AVX intrinsics if available */
__attribute__((target("avx2")))
void avx_vector_operations(void) {
    /* Use target-specific built-in to check CPU features */
    if (__builtin_cpu_supports("avx2")) {
        v4si a = {1, 2, 3, 4};
        v4si b = {5, 6, 7, 8};
        v4si c = {0};
        
        /* Complex vector operations that might need helper functions */
        for (int i = 0; i < 4; i++) {
            /* Simulate complex operation - division might need runtime helper */
            c[i] = a[i] * b[i] / (a[i] + 1);
        }
        
        /* Use result to prevent optimization */
        volatile v4si dummy = c;
        (void)dummy;
    }
}

/* Pattern 4: Transactional Memory block */
void transactional_operation(void) {
    /* This requires -fgnu-tm flag */
    __transaction_atomic {
        global_counter++;
        /* Nested complexity */
        if (global_counter % 2 == 0) {
            global_counter *= 2;
        }
    }
}

/* Pattern 5: Large stack usage with protection */
void stack_protected_function(void) {
    /* Large array to trigger stack protection */
    char buffer[1024 * 1024];  /* 1MB buffer */
    
    /* Initialize to prevent optimization */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (char)(i % 256);
    }
    
    /* Use buffer to prevent dead code elimination */
    volatile int sum = 0;
    for (size_t i = 0; i < sizeof(buffer); i += 1024) {
        sum += buffer[i];
    }
    (void)sum;
}

/* Pattern 6: OpenMP offloading attempt */
void openmp_offload_attempt(void) {
    int n = 1000;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Attempt offload - will likely generate fallback helpers */
    #pragma omp target map(tofrom: array[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Use result */
    volatile int check = array[n-1];
    (void)check;
    
    free(array);
}

/* Pattern 7: ARM/PowerPC specific built-ins (compile-time guarded) */
void arch_specific_builtins(void) {
#if defined(__arm__) || defined(__aarch64__)
    /* ARM specific */
    unsigned int fpscr = 0;
    fpscr = __builtin_arm_get_fpscr();
    volatile unsigned int dummy_arm = fpscr;
    (void)dummy_arm;
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* PowerPC specific */
    unsigned long fpscr = 0;
    __builtin_ppc_mtfsf(0xFF, fpscr);
#elif defined(__x86_64__) || defined(__i386__)
    /* x86 CPU initialization */
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx512f")) {
        /* Use AVX512 if available */
        __m512i vec = _mm512_set1_epi32(42);
        volatile __m512i dummy_vec = vec;
        (void)dummy_vec;
    }
#endif
}

/* External function from second compilation unit */
extern void weak_constructor_function(void);

int main(void) {
    printf("Starting target hook coverage test...\n");
    
    /* Pattern 1: CPU feature detection and vector operations */
    arch_specific_builtins();
    
    /* Pattern 2: AVX vector operations */
    avx_vector_operations();
    
    /* Pattern 3: Transactional memory */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    printf("Global counter after TM: %d\n", global_counter);
    
    /* Pattern 4: Stack protection trigger */
    stack_protected_function();
    
    /* Pattern 5: OpenMP offload attempt */
    openmp_offload_attempt();
    
    /* Pattern 6: Call weak constructor from other file */
    weak_constructor_function();
    
    /* Pattern 7: More complex vector math that might need helpers */
    {
        v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
        v4sf result;
        
        /* Complex floating-point vector operation */
        for (int i = 0; i < 4; i++) {
            /* Approximation that might need helper */
            result[i] = vec1[i] * vec2[i] / (vec1[i] + vec2[i]);
        }
        
        volatile v4sf dummy = result;
        (void)dummy;
    }
    
    printf("Test completed.\n");
    return 0;
}
