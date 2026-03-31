/* main.c - Primary file with complex patterns to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Declare external weak function from helper.c */
extern void __attribute__((weak)) target_helper_init(void);

/* Global variable for transactional memory */
volatile int global_counter = 0;

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that may need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Use CPU feature detection */
    if (__builtin_cpu_supports("avx2")) {
        __builtin_cpu_init();
        
        /* Complex vector expression - division might need helper */
        v4si c = a + b;
        v4si d = b - a;
        
        /* Simulate complex operation that might need helper */
        for (int i = 0; i < 4; i++) {
            /* Force potential helper generation */
            if (c[i] > 0) {
                d[i] = d[i] / (c[i] + 1);
            }
        }
        
        /* Use result to prevent optimization */
        volatile v4si result = c + d;
        (void)result;
    }
}

/* Function with AVX512 target attribute */
__attribute__((target("avx512f")))
void avx512_complex_math(void) {
    /* Trigger AVX512 helper generation */
    if (__builtin_cpu_supports("avx512f")) {
        /* Large array for stack protection */
        char buffer[1024];
        for (int i = 0; i < sizeof(buffer); i++) {
            buffer[i] = i % 256;
        }
        
        /* Use buffer to prevent optimization */
        volatile int sum = 0;
        for (int i = 0; i < sizeof(buffer); i++) {
            sum += buffer[i];
        }
        (void)sum;
    }
}

/* Transactional memory function */
void transactional_operation(void) {
    /* GCC Transactional Memory - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        if (global_counter % 2 == 0) {
            global_counter *= 2;
        }
    }
    
    /* Another transaction with more complexity */
    __transaction_atomic {
        int local = global_counter;
        for (int i = 0; i < 10; i++) {
            local += i;
        }
        global_counter = local;
    }
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int n = 100;
    int a[n], b[n], c[n];
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Attempt offloading - may generate fallback helpers */
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) device(0)
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Verify results */
    volatile int check = 0;
    for (int i = 0; i < n; i++) {
        check += c[i];
    }
    (void)check;
}

/* ARM-specific built-ins (if compiled for ARM) */
#ifdef __arm__
void arm_specific_operations(void) {
    /* ARM system register access - may need helper */
    unsigned int fpscr = __builtin_arm_mrc(15, 0, 1, 0, 0);
    volatile unsigned int result = fpscr;
    (void)result;
}
#endif

/* PowerPC-specific built-ins (if compiled for PowerPC) */
#ifdef __powerpc__
void powerpc_specific_operations(void) {
    /* PowerPC MTFSF - may need helper */
    double d = 3.14159;
    __builtin_ppc_mtfsf(0xFF, d);
}
#endif

/* Main function with conditional execution paths */
int main(void) {
    printf("Starting target hook trigger program...\n");
    
    /* Path 1: Vector operations with CPU feature detection */
    avx2_vector_operations();
    
    /* Path 2: AVX512 complex operations */
    avx512_complex_math();
    
    /* Path 3: Transactional memory */
    for (int i = 0; i < 5; i++) {
        transactional_operation();
    }
    printf("Global counter after transactions: %d\n", global_counter);
    
    /* Path 4: OpenMP offloading attempt */
    openmp_offload_attempt();
    
    /* Path 5: Call weak external function that uses target built-ins */
    if (&target_helper_init) {
        target_helper_init();
    }
    
    /* Architecture-specific paths */
    #ifdef __arm__
    arm_specific_operations();
    #elif defined(__powerpc__)
    powerpc_specific_operations();
    #elif defined(__x86_64__)
    /* x86 specific: Use more intrinsics */
    if (__builtin_cpu_supports("sse4.2")) {
        /* Force potential helper generation */
        volatile long long a = 123456789;
        volatile long long b = 987654321;
        volatile long long result = __builtin_ia32_crc32di(a, b);
        (void)result;
    }
    #endif
    
    /* Final complex expression mixing everything */
    {
        /* Large volatile array for stack protection */
        volatile int big_array[256];
        for (int i = 0; i < 256; i++) {
            big_array[i] = i * global_counter;
        }
        
        /* Use in transaction */
        __transaction_atomic {
            for (int i = 0; i < 256; i += 16) {
                global_counter += big_array[i];
            }
        }
    }
    
    printf("Program completed. Final counter: %d\n", global_counter);
    return 0;
}
