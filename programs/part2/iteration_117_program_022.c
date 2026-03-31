/* Main file with complex patterns to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For transactional memory */
int global_counter = 0;

/* Weak function declaration that will be defined in file2.c */
extern void __attribute__((weak)) weak_constructor_func(void);

/* Vector extension type */
typedef int v4si __attribute__((vector_size(16)));

/* Function using AVX2 target attribute */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Mix of operations including division which often needs runtime support */
    v4si result = a + b * 2;
    
    /* Use result to prevent optimization */
    volatile v4si v = result;
    (void)v;
}

/* Function with transactional memory */
void transactional_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        /* Nested complexity */
        if (global_counter % 2 == 0) {
            global_counter *= 2;
        }
    }
}

/* OpenMP target region */
void openmp_offload_attempt(void) {
    int data[100];
    
    #pragma omp target map(tofrom: data[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] = i * i;
        }
    }
    
    /* Use data to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* Function using CPU feature detection */
void cpu_feature_detection(void) {
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Check multiple features to increase chance of helper generation */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* Force generation of helper functions by using conditional execution */
    if (has_avx512) {
        /* Use AVX512 builtins if available */
        avx2_vector_operations();
    } else if (has_avx2) {
        avx2_vector_operations();
    }
    
    /* Print results to prevent optimization */
    printf("CPU Features - AVX512: %d, AVX2: %d, SSE4.2: %d\n", 
           has_avx512, has_avx2, has_sse4);
}

/* Large stack usage to trigger stack protection helpers */
void large_stack_function(void) {
    char large_buffer[4096];  /* Large stack allocation */
    
    /* Fill and use buffer */
    memset(large_buffer, 0xAA, sizeof(large_buffer));
    
    /* Complex operations on buffer */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = (large_buffer[i] + i) & 0xFF;
    }
    
    /* Use result */
    volatile int checksum = 0;
    for (int i = 0; i < sizeof(large_buffer); i++) {
        checksum += large_buffer[i];
    }
    (void)checksum;
}

int main(void) {
    printf("Starting target hook trigger program...\n");
    
    /* 1. CPU feature detection and vector operations */
    cpu_feature_detection();
    
    /* 2. OpenMP offload attempt */
    openmp_offload_attempt();
    
    /* 3. Transactional memory operations */
    for (int i = 0; i < 10; i++) {
        transactional_operation();
    }
    printf("Global counter after transactions: %d\n", global_counter);
    
    /* 4. Large stack usage (triggers stack protection helpers) */
    large_stack_function();
    
    /* 5. Call weak constructor function from other compilation unit */
    weak_constructor_func();
    
    /* 6. Additional architecture-specific builtins */
    #ifdef __aarch64__
    /* ARM-specific builtin */
    unsigned long fpcr = __builtin_aarch64_get_fpcr();
    printf("FPCR: %lu\n", fpcr);
    #elif defined(__powerpc__) || defined(__ppc__)
    /* PowerPC-specific builtin */
    __builtin_ppc_mtfsf(0xFF, 0);
    #endif
    
    /* 7. More complex vector operations */
    {
        /* Trigonometric approximation using vector operations */
        v4si angles = {0, 45, 90, 135};
        v4si results;
        
        /* This complex operation might trigger helper generation */
        for (int i = 0; i < 4; i++) {
            /* Approximate sine using Taylor series (simplified) */
            int angle = angles[i];
            /* Convert to radians approximation */
            int rad = angle * 114 / 365;  /* Rough pi/180 approximation */
            results[i] = rad - (rad*rad*rad)/6 + (rad*rad*rad*rad*rad)/120;
        }
        
        volatile v4si v = results;
        (void)v;
    }
    
    printf("Program completed.\n");
    return 0;
}
