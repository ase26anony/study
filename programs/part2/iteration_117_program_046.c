/* main.c - Primary file with multiple code paths to trigger target hooks */
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

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
static v4si vector_division(v4si a, v4si b) {
    /* Complex vector operation that may need helper */
    v4si result;
    
    /* Use CPU feature check - may generate helper */
    if (__builtin_cpu_supports("avx512f")) {
        /* This branch unlikely but forces consideration of AVX512 helpers */
        result = a / (b + 1);  /* Integer vector division may need runtime helper */
    } else {
        /* More complex operation mixing types */
        v4sf fa = __builtin_convertvector(a, v4sf);
        v4sf fb = __builtin_convertvector(b + 1, v4sf);
        v4sf fresult = fa / fb;  /* Float division */
        result = __builtin_convertvector(fresult, v4si);
    }
    
    /* Trigonometric approximation on vector - may need math helper */
    for (int i = 0; i < 4; i++) {
        /* Approximate sin using Taylor series - forces math helper consideration */
        float x = result[i] * 0.01f;
        float x2 = x * x;
        float x3 = x2 * x;
        float sin_approx = x - x3/6.0f + x3*x2/120.0f;
        result[i] = (int)(sin_approx * 100.0f);
    }
    
    return result;
}

/* Function with transactional memory */
__attribute__((noinline))
static void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local_array[256];  /* Large array for stack protector */
        for (int i = 0; i < 256; i++) {
            local_array[i] = i * global_counter;
        }
        
        /* Use result to prevent optimization */
        volatile int sum = 0;
        for (int i = 0; i < 256; i++) {
            sum += local_array[i];
        }
    }
}

/* OpenMP target region */
static void attempt_offload(void) {
    int n = 1000;
    int a[n], b[n], c[n];
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Attempt offload - will likely generate fallback helpers */
    #pragma omp target device(0) map(to: a[0:n], b[0:n]) map(from: c[0:n])
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Verify result */
    volatile int check = 0;
    for (int i = 0; i < n; i++) {
        check += c[i];
    }
}

/* PowerPC specific built-in (if compiled for PowerPC) */
#ifdef __powerpc__
static void powerpc_special(void) {
    /* Use PowerPC special register built-in */
    unsigned long fpscr;
    __asm__ volatile ("mffs %0" : "=f"(fpscr));
    
    /* Built-in that may need helper */
    __builtin_ppc_mtfsf(0xFF, fpscr);
}
#endif

/* ARM specific built-in (if compiled for ARM) */
#ifdef __arm__
static void arm_special(void) {
    /* ARM system register access */
    unsigned int val;
    val = __builtin_arm_mrc(15, 0, 13, 0, 0);
    
    /* Use the value */
    volatile unsigned int *ptr = &val;
    *ptr = *ptr + 1;
}
#endif

int main(void) {
    printf("Starting target hook trigger program...\n");
    
    /* Initialize CPU features - may generate helper */
    __builtin_cpu_init();
    
    /* Call weak constructor function from helper.c */
    if (&target_helper_init) {
        target_helper_init();
    }
    
    /* Path 1: Vector operations with CPU feature checks */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Check multiple CPU features */
    int has_avx = __builtin_cpu_supports("avx");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_avx512 = __builtin_cpu_supports("avx512f");
    
    printf("CPU Features: AVX=%d, AVX2=%d, AVX512=%d\n", 
           has_avx, has_avx2, has_avx512);
    
    if (has_avx2) {
        v4si result = vector_division(vec1, vec2);
        
        /* Use result to prevent dead code elimination */
        volatile int sum = 0;
        for (int i = 0; i < 4; i++) {
            sum += result[i];
        }
        printf("Vector result sum: %d\n", sum);
    }
    
    /* Path 2: Transactional memory operations */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    printf("Global counter after transactions: %d\n", global_counter);
    
    /* Path 3: OpenMP offload attempt */
    #ifdef _OPENMP
    attempt_offload();
    printf("OpenMP offload attempted\n");
    #endif
    
    /* Path 4: Architecture-specific built-ins */
    #ifdef __powerpc__
    powerpc_special();
    printf("PowerPC special operations executed\n");
    #endif
    
    #ifdef __arm__
    arm_special();
    printf("ARM special operations executed\n");
    #endif
    
    /* Path 5: More complex CPU feature dependent code */
    #pragma GCC push_options
    #pragma GCC target("avx2")
    if (__builtin_cpu_supports("avx2")) {
        /* Inline assembly that might need AVX helper */
        __asm__ volatile (
            "vmovdqa %%ymm0, %%ymm1\n\t"
            "vpxor %%ymm1, %%ymm1, %%ymm1"
            : : : "ymm0", "ymm1"
        );
    }
    #pragma GCC pop_options
    
    printf("Program completed successfully.\n");
    return 0;
}
