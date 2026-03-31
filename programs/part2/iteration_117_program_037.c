/* Main file with multiple patterns to trigger target hooks */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For Transactional Memory */
volatile int global_counter = 0;

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* External function from second compilation unit */
extern void weak_constructor_function(void);

/* Function using AVX2 target pragma */
__attribute__((target("avx2")))
void avx2_vector_operations(void) {
    /* Complex vector operations that might need runtime helpers */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex expression with multiple operations */
    v4si result = (a * b) + (c / (a + 1));
    
    /* Use result to prevent optimization */
    volatile v4si volatile_result = result;
    (void)volatile_result;
}

/* Function using target-specific builtins */
void use_target_builtins(void) {
    /* x86_64 CPU feature detection - may generate helper functions */
    __builtin_cpu_init();
    
    /* Check multiple features to increase chance of helper generation */
    int has_avx512 = __builtin_cpu_supports("avx512f");
    int has_avx2 = __builtin_cpu_supports("avx2");
    int has_sse4 = __builtin_cpu_supports("sse4.2");
    
    /* ARM builtins (if compiled for ARM) */
    /* unsigned int fpsr = __builtin_arm_get_fpscr(); */
    
    /* PowerPC builtins (if compiled for PowerPC) */
    /* __builtin_ppc_mtfsf(0xFF, 0.0); */
    
    /* Use results to prevent dead code elimination */
    volatile int vol_has_avx512 = has_avx512;
    volatile int vol_has_avx2 = has_avx2;
    volatile int vol_has_sse4 = has_sse4;
    (void)vol_has_avx512;
    (void)vol_has_avx2;
    (void)vol_has_sse4;
}

/* Function with OpenMP target region */
void openmp_offloading(void) {
    int n = 100;
    int data[n];
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offloading - may generate fallback functions */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2 + 1;
        }
    }
    
    /* Use data to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    (void)sum;
}

/* Function using Transactional Memory */
void transactional_memory_operation(void) {
    /* Transactional memory block - requires runtime support */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complex operations */
        int local_array[100];
        for (int i = 0; i < 100; i++) {
            local_array[i] = i * global_counter;
        }
        
        /* Use array to prevent optimization */
        volatile int vol_sum = 0;
        for (int i = 0; i < 100; i++) {
            vol_sum += local_array[i];
        }
        (void)vol_sum;
    }
}

/* Function with large stack usage for stack protector */
void large_stack_function(void) {
    /* Large array to trigger stack protection */
    char large_buffer[4096];
    
    /* Initialize and use buffer */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i % 256;
    }
    
    /* Complex operation on buffer */
    volatile int checksum = 0;
    for (int i = 0; i < sizeof(large_buffer); i++) {
        checksum += large_buffer[i];
    }
    (void)checksum;
}

int main(void) {
    printf("Starting target hook coverage test...\n");
    
    /* Pattern 1: CPU feature detection and vector operations */
    use_target_builtins();
    
    /* Pattern 2: AVX2 vector operations */
    avx2_vector_operations();
    
    /* Pattern 3: OpenMP offloading */
    openmp_offloading();
    
    /* Pattern 4: Transactional Memory */
    for (int i = 0; i < 5; i++) {
        transactional_memory_operation();
    }
    
    /* Pattern 5: Large stack usage */
    large_stack_function();
    
    /* Pattern 6: Call weak constructor function from other compilation unit */
    weak_constructor_function();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return 0;
}
