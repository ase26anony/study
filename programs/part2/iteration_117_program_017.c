/* main.c - Primary file with complex patterns to trigger target hooks */
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

/* Function with AVX2 target attribute - may require runtime checks */
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
    
    /* Mix with CPU feature check */
    if (__builtin_cpu_supports("avx2")) {
        /* More complex operations when AVX2 is available */
        v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf f2 = {5.0f, 6.0f, 7.0f, 8.0f};
        v4sf f3 = f1 * f2 + f1 / f2;
        
        volatile v4sf* volatile_fptr = &f3;
        (void)volatile_fptr;
    }
}

/* Function with different target for potential helper generation */
__attribute__((target("default")))
void default_target_operations(void) {
    /* Use __builtin_cpu_init to potentially trigger helper generation */
    __builtin_cpu_init();
    
    /* Check multiple CPU features */
    int has_sse = __builtin_cpu_supports("sse");
    int has_sse2 = __builtin_cpu_supports("sse2");
    int has_avx = __builtin_cpu_supports("avx");
    
    /* Complex conditional based on CPU features */
    if (has_sse && has_sse2) {
        /* Vector operations that might need runtime support */
        v4si v1 = {100, 200, 300, 400};
        v4si v2 = {1, 2, 3, 4};
        
        /* Non-trivial operation that might need helper */
        for (int i = 0; i < 4; i++) {
            v1[i] = v1[i] / (v2[i] + 1);
        }
        
        volatile v4si* vptr = &v1;
        (void)vptr;
    }
    
    if (has_avx) {
        /* Force potential AVX helper generation */
        avx2_vector_operations();
    }
}

/* Transactional memory function */
void transactional_operation(void) {
    /* Use GNU transactional memory extension */
    __transaction_atomic {
        global_counter++;
        
        /* Nested complexity */
        int local_temp = global_counter * 2;
        for (int i = 0; i < 10; i++) {
            local_temp += i;
        }
        
        /* Use result */
        volatile int* vptr = &local_temp;
        (void)vptr;
    }
    
    /* Another transaction with more complexity */
    __transaction_atomic {
        int array[100];
        for (int i = 0; i < 100; i++) {
            array[i] = global_counter + i;
        }
        
        /* Force stack protector */
        volatile int sum = 0;
        for (int i = 0; i < 100; i++) {
            sum += array[i];
        }
        
        global_counter = sum % 1000;
    }
}

/* OpenMP target region - may generate fallback helpers */
void openmp_offload_attempt(void) {
    int n = 1000;
    int* data = (int*)malloc(n * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    
    /* Attempt offload - will likely use host fallback */
    #pragma omp target map(tofrom: data[0:n]) device(0)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2 + 1;
        }
    }
    
    /* Use result to prevent elimination */
    volatile int check = 0;
    for (int i = 0; i < n; i++) {
        check += data[i];
    }
    
    volatile int* vptr = &check;
    (void)vptr;
    
    free(data);
}

/* Large stack usage to trigger stack protection helpers */
void large_stack_function(void) {
    /* Large array to trigger stack protector */
    char large_buffer[4096];
    int another_array[512];
    
    /* Use buffers to prevent optimization */
    for (int i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = i % 256;
    }
    
    for (int i = 0; i < 512; i++) {
        another_array[i] = large_buffer[i % 256] + i;
    }
    
    /* Complex computation */
    volatile int sum = 0;
    for (int i = 0; i < 512; i++) {
        sum += another_array[i];
    }
    
    /* Call external weak function */
    if (target_helper_init) {
        target_helper_init();
    }
}

/* Main function orchestrating all patterns */
int main(void) {
    printf("Starting target hook trigger program...\n");
    
    /* Initialize CPU features */
    __builtin_cpu_init();
    
    /* Execute different code paths based on CPU features */
    if (__builtin_cpu_supports("sse")) {
        default_target_operations();
    }
    
    if (__builtin_cpu_supports("avx")) {
        avx2_vector_operations();
    }
    
    /* Force transactional memory operations */
    for (int i = 0; i < 5; i++) {
        transactional_operation();
    }
    
    /* Attempt OpenMP offload */
    openmp_offload_attempt();
    
    /* Use large stack to trigger protection */
    large_stack_function();
    
    /* Print results to prevent optimization */
    printf("Global counter: %d\n", global_counter);
    
    /* Check CPU features at runtime */
    printf("CPU supports SSE: %d\n", __builtin_cpu_supports("sse"));
    printf("CPU supports AVX: %d\n", __builtin_cpu_supports("avx"));
    printf("CPU supports AVX2: %d\n", __builtin_cpu_supports("avx2"));
    
    return 0;
}
